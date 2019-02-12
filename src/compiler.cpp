/*
 * compiler.cpp
 *
 *  Created on: 17 okt. 2016
 *      Author: mattias
 */

#include "compiler.h"
#include "statement.h"
#include "virtualmachine.h"

//Check so that the statement is not empty, otherwise throw a Compilation Exception
#define ASSERT_COMPILATION(statement, astUnit, text) if (!statement) throw CompilationException(astUnit.createToken(), text);

std::map<string, BinaryStatement::MemberPointerType> binaryOperatorFunctionMap = {
		{"+", &Value::operator+},
		{"-", &Value::operator-},
		{"*", &Value::operator*},
		{"/", &Value::operator/},
		{"<", &Value::operator<},
		{">", &Value::operator>},
		{"<=", &Value::operator<=},
		{">=", &Value::operator>=},
		{"==", &Value::operator==},
		{"!=", &Value::operator!=},
		{"+=", &Value::operator+=},
		{".", &Value::propertyAccessor},
};



std::map<string, UnaryStatement::MemberPointerType> unaryOperatorFunctionMap = {
		{"++", &Value::operator++},
		{"--", &Value::operator++},
		{"+", &Value::unaryPlus},
		{"-", &Value::unaryMinus},
		{"!", &Value::operator!},
};


static StatementPointer createBinaryStatement(AstUnit& unit) {
	if (unit.size() != 3) {
		throw CompilationException(unit.createToken(), "failed to create binary statement: wrong number of arguments");
	}

	auto left = Compiler::compile(unit[0]);
	auto right = Compiler::compile(unit[2]);
	auto middleToken = unit[1].token;

	if (middleToken == "=") {
		auto left = Compiler::compile(unit[0]);
		return StatementPointer(new Assignment(left, right));
	}
	else {
		auto f = binaryOperatorFunctionMap.find(middleToken);
		if (f != binaryOperatorFunctionMap.end()) {
			return StatementPointer(new BinaryStatement(left, right, f->second));
		}
	}

	throw "binary statement not implemented";
}


static StatementPointer createPrefixStatement(AstUnit& unit) {
	if (unit.size() != 2) {
		throw CompilationException(unit.createToken(), "prefix statement is of wrong format");
	}

	auto statement = Compiler::compile(unit[1]);
	auto op = unit[0].token;

	auto f = unaryOperatorFunctionMap.find(op);
	if (f != unaryOperatorFunctionMap.end()) {
		return StatementPointer(new UnaryStatement(statement, f->second));
	}

	throw "unary statement not implemented";
}


static shared_ptr<vector<Token>> getNamesFromArgumentList(AstUnit *arguments) {
	auto argumentNames = make_shared<vector <Token>>();

	if (arguments) {
		arguments->groupUnit();

		if (!arguments->empty()) {
			if (arguments->size() == 1) {
				auto &sequence = arguments->get(0);
				std::function<void (AstUnit&)> sum = [&argumentNames, &sum] (AstUnit &sequence) {
					if (sequence.type == AstUnit::Sequence) {
						sum(sequence.get(0));
						argumentNames->push_back(sequence.get(2).token);
					}
					else {
						argumentNames->push_back(sequence.token);
					}
				};
				sum(sequence);
			}
		}
		else {
			argumentNames->push_back(arguments->token);
		}
	}

	return argumentNames;
}


StatementPointer Compiler::compile(AstUnit& unit, bool allowNull) {
	Statement* statement = nullptr;
	switch (unit.type) {
	case unit.Word:
		statement = new LiteralStatement(unit.token);
		break;
	case unit.GenericGroup:
	case unit.Condition:
	case unit.Statement:
	case unit.Braces: {
		if (unit.type != unit.Braces && unit.children.size() == 1) {
			//A condition or a single statement
			return compile(unit[0]);
		} else {
			unit.groupUnit();
			//Check if the unit represents a object
			//i.e if the braces is empty or the first element is on the form x: y or
			if (unit.type == unit.Braces &&
					(unit.empty() || unit[0].getFirstSequenceType() == unit.ObjectMemberDefinition)) {
//				std::cout << "declare object";
				auto objectDeclaration = new ObjectDefinition();

				if (!unit.empty()) {
					auto sequenceList = unit[0].getFlatSequence();
					for (auto it: sequenceList) {
						if (it->type != unit.ObjectMemberDefinition) {
							throw CompilationException(unit.token, "unexpected token in object declaration: " + it->token);
						}
						if (it->size() != 3) {
							throw CompilationException(it->token, "wrong format in object declaration");
						}
						objectDeclaration->declarationPairs.push_back({compile(it->front()), compile(it->back())});
					}
				}

				statement = objectDeclaration;
			}
			else {
				auto block = new CodeBlock();
				for (auto& u : unit) {
					auto s = compile(*u);
					if (s) {
						block->statements.push_back(s);
					}
				}
				statement = block;
			}
		}
	}
		break;
	case unit.Array: {
		auto brackets = unit.getByType(unit.ArrayBrackets);
		brackets->groupUnit();
		vector<StatementPointer> elements;
		for (auto &elementUnit: *brackets) {
			if (elementUnit->type == unit.Coma) {
				continue;
			}
			auto s = compile(*elementUnit);
			if (!s) {
				throw CompilationException(unit.createToken(), "could not interpret statement " + elementUnit->createToken());
			}
			elements.push_back(s);
		}
		statement = new ArrayDefinition(elements);
		break;
	}
	case unit.String:
		statement = new StringLiteralStatement(unit.token);
		break;
	case unit.Number:
		statement = new NumberLiteralStatement(unit.token);
		break;
	case unit.Boolean:
		statement = new BooleanLiteralStatement(unit.token);
		break;
	case unit.This:
		statement = new ThisStatement;
		break;
	case unit.PropertyAssignment:
	{
		if (unit.empty()) {
			throw CompilationException(unit.createToken(), "error with property assignment");
		}
		auto &assignment = (unit.size() == 1)? unit[0] : unit;
		if (assignment.size() != 3 || assignment[1].type != unit.AssignmentOperator) {
			throw CompilationException(unit.createToken(), "error with property assignment");
		}
		auto value = compile(assignment[2]);

		StatementPointer object;
		StatementPointer member;

		auto &accessor = assignment[0];
		if (accessor.size() == 2) {
			//Format x[0] = y
			object = compile(accessor[0]);
			if (accessor[1].empty()) {
				throw CompilationException(unit.createToken(), "no property given in property assignment");
			}
			member = compile(accessor[1][0]);
		}
		else if (accessor.size() == 3) {
			//Fromat x.x = y
			if (accessor[1].type != unit.Period) {
				throw CompilationException(unit.createToken(), "syntax error in property assignment");
			}
			object = compile(accessor[0]);
			member = compile(accessor[2]);
		}

		if (!object) {
			CompilationException(unit.createToken(), "no valid owner object in property assignment");
		}
		if (!member) {
			CompilationException(unit.createToken(), "no valid property in assignment");
		}

		return StatementPointer(new PropertyAssignment(object, member, value));
	}

	case unit.NewTarget:
		return StatementPointer(new NewTargetStatement);
	case unit.PropertyAccessor: {
		if (unit.size() == 3) {
			return createBinaryStatement(unit);
		}
		else if (unit.size() == 2) {
			if (unit[1].size() != 1 && unit[1][0].size() != 1) {
				throw CompilationException(unit.createToken(), "Malformed property accessor");
			}

			return StatementPointer(new BinaryStatement(compile(unit[0]), compile(unit[1][0]), &Value::propertyAccessor));
		}
	}
	break;
	case unit.BinaryStatement:
	case unit.Assignment:
		return createBinaryStatement(unit);
	case unit.PrefixStatement:
		return createPrefixStatement(unit);
	case unit.PostfixStatement: {
		if (unit.size() != 2) {
			throw CompilationException(unit.createToken(), "postfix statement is of wrong format");
		}
		throw CompilationException(unit.createToken(), "postfix not implemented");
	}
	case unit.NewStatement: {
		auto identifier = compile(unit[1]);
		StatementPointer arguments;
		if (auto argumentUnit = unit.getByType(unit.Arguments)) {
			arguments = compile(*argumentUnit, true);
		}
		statement = new NewStatement(identifier, arguments);
		break;
	}
	case unit.Arguments: {
		auto s = new ArgumentStatement();
		if (!unit.empty()) {
			cumulate(unit[0], s->statements);
		}
		statement = s;
	}
		break;
	case unit.FunctionCall: {
		auto fc = new FunctionCall();
		if (auto name = unit.children[0].get()) {
			fc->identifier = StatementPointer(compile(*name));
		}
		if (auto argumentUnit = unit.getByType(unit.Arguments)) {
			if (!argumentUnit->empty()) {
				cumulate(*argumentUnit->children[0],
						fc->arguments.statements);
			}
		}
		statement = fc;
		break;
	}
	case unit.MethodCall: {
		auto object = compile(unit[0][0]);
		StatementPointer memberFunction;
		if (unit[0][1].type == unit.Period) {
			//if the method is written as x.y()
			memberFunction = compile(unit[0][2]);
		}
		else {
			//if the method is written as x["y"]
			memberFunction = compile(unit[0][1][0]);
		}
		ArgumentStatement arguments;
		if (!unit[1].children.empty()) {
			cumulate(*unit[1].children[0], arguments.statements);
		}
		statement = new MethodCall(object, memberFunction, arguments);
		break;
	}
	case unit.Function:
	{
		auto f = new FunctionDeclaration();

		if (auto name = unit.getByType(unit.Name)) {
			f->name = name->token;
		}

		f->argumentNames = make_shared<vector<Token>>();


		vector<Token> &argumentNames(*f->argumentNames.get());
		//Sum all the arguments to a list
		f->argumentNames = getNamesFromArgumentList(unit.getByType(unit.Arguments));

		if (auto block = unit.getByType(unit.Statement)) {
			block->groupUnit();
			f->block = StatementPointer(compile(*block));
		}
		if (!f->block) {
			throw "no body defined for function";
		}
		statement = f;
		break;
	}
	case unit.ArrowFunction: {
		if (unit.size() != 3) {
			CompilationException(unit.createToken(), "Wrong format in arrow function");
		}
		auto &argumentUnit = unit[0];
		auto argumentList = getNamesFromArgumentList(&argumentUnit);
		auto blockUnit = unit[2];
		auto block = compile(blockUnit);
		auto arrowToken = unit[1];

		return StatementPointer(new ArrowFunctionDeclaration(argumentList, block, arrowToken.createToken()));
	}
	case unit.ReturnStatement: {
		statement = new ReturnStatement(compile(unit[1]));
	}
	break;
	case unit.VariableDeclaration: {
		if (auto name = unit.getByType(unit.Name)) {
			auto vd = new VariableDeclaration(name->token);
			statement = vd;
		} else if (auto assignment = unit.getByType(unit.Assignment)) {
			//				unit.print(std::cout);
			if (assignment->size() != 3) {
				throw CompilationException(assignment->createToken(),
						"syntax error in variable declaration");
			}
			if ((*assignment)[1].token == "=") {
				statement = new VariableDeclaration((*assignment)[0].token,
						compile((*assignment)[2]));
			} else {
				throw CompilationException(assignment->createToken(),
						"unexpected token in variable declaration");
			}
		} else {
			throw CompilationException(unit.createToken(),
					"no variable name given in variable declaration");
		}
	}
		break;
	case unit.Semicolon:
		return StatementPointer(nullptr);
		break;
	case unit.IfStatement:
	{
		//Build if-statements
		auto i = new IfStatement();
		auto &conditions = i->conditions;
		auto &blocks = i->blocks;
		bool plainElseBlockAllowed = true; //A else block without condition only allowed one time in the end

		auto firstStatement = &unit;
		while (firstStatement && firstStatement->type == unit.IfStatement) {
			if (auto afterElseStatement = firstStatement->getAfterToken(unit.ElseKeyword)) {
				if (afterElseStatement->type == unit.IfStatement) {
					//The last else is a "else if": extract if-stuff from it
					conditions.insert(conditions.begin(), compile(*afterElseStatement->getByType(unit.Condition)));
					blocks.insert(blocks.begin(), compile((*afterElseStatement->getByType(unit.Braces))));
				}
				else {
					if (!plainElseBlockAllowed) {
						throw CompilationException(unit.createToken(), "only one else statement is allowed");
					}
					//It is only a else-statement add that
					blocks.insert(blocks.begin(), compile(*afterElseStatement));

					plainElseBlockAllowed = false;
				}
			}

			if (auto condition = firstStatement->getByType(unit.Condition)) {
				//The final if-statement
				auto block = firstStatement->getByType(unit.Braces);

				conditions.insert(conditions.begin(), compile(*condition));
				blocks.insert(blocks.begin(), compile(*block));

				firstStatement = nullptr; //Breaks the loop
			}
			else {
				firstStatement = &(*firstStatement)[0]; //Continue one more time
			}

			plainElseBlockAllowed = false; //Only allowed in the end (that is first in the loop)
		}
		statement = i;
	}
	break;
	case unit.WhileLoop:
	{
		auto condition = unit.getByType(unit.Condition);
		condition->groupUnit();
		if (condition->size() != 1) {
			throw CompilationException(unit.createToken(), "syntax error in while loop");
		}
		auto &block = unit[2];
		auto w = new WhileLoop(compile(*condition), compile(block));
		statement = w;
	}
	break;
	case unit.ForLoop:
	{
		auto &argument = unit[1];
		argument.groupUnit();
		auto &block = unit[2];

		auto initialization = argument[0];
		auto condition = argument[2];
		auto increment = argument[4];

		auto f = new ForLoop(compile(initialization), compile(condition), compile(increment), compile(block));

		statement = f;
	}
	break;
	}
	if (statement == nullptr) {
		if (unit.size() > 0 || !allowNull) {
			CompilationException(unit.createToken(), "Could not compile command");
		}
	}
	return StatementPointer(statement);
}


