/*
 * compiler.cpp
 *
 *  Created on: 17 okt. 2016
 *      Author: mattias
 */

#include "compiler.h"

std::map<string, BinaryStatement::MemberPointerType> operatorFunctionMap = {
		{"+", &Value::operator+},
		{"-", &Value::operator-},
		{"*", &Value::operator*},
		{"/", &Value::operator/},
};


StatementPointer Compiler::compile(AstUnit& unit) {
	Statement* statement = nullptr;
	switch (unit.type) {
	case unit.Word:
		statement = new LiteralStatement(unit.token);
		break;
	case unit.GenericGroup:
	case unit.Condition:
	case unit.Braces: {
		if (unit.type != unit.Braces && unit.children.size() == 1) {
			return compile(unit[0]);
		} else {
			unit.groupUnit();
//			unit.print(std::cout);
			auto block = new CodeBlock();
			for (auto& u : unit) {
				auto s = compile(*u);
//				u->print(std::cout);
				if (s) {
					block->statements.push_back(s);
				}
			}
			statement = block;
		}
	}
		break;
	case unit.String:
		statement = new StringLiteralStatement(unit.token);
		break;
	case unit.Digit:
		statement = new NumberLiteralStatement(unit.token);
		break;
	case unit.BinaryStatement: {
		if (unit.size() != 3) {
			throw "failed to create binary statement: wrong number of arguments";
		}
		return createBinaryStatement(unit);
	}
		break;
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
				cumulate(*argumentUnit->children[0].get(),
						fc->arguments.statements);
			}
		}
		statement = fc;
	}
		break;
	case unit.VariableDeclaration: {
		if (auto name = unit.getByType(unit.Name)) {
			auto vd = new VariableDeclaration(name->token);
			statement = vd;
		} else if (auto assignment = unit.getByType(unit.BinaryStatement)) {
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
	case unit.SemiColon:
		return StatementPointer(nullptr);
		break;
	case unit.Function:
	{
		auto f = new FunctionDeclaration();

		if (auto name = unit.getByType(unit.Name)) {
			f->name = name->token;
		}

		if (auto block = unit.getByType(unit.Braces)) {
			block->groupUnit();
			f->block = compile(*block);
		}
		statement = f;
	}
	break;
	case unit.IfStatement:
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
	break;
	}
	if (statement == nullptr) {
		//			throw "statement could not be compiled";
		std::cout << "warning: zero statement" << endl;
	}
	return StatementPointer(statement);
}


StatementPointer Compiler::createBinaryStatement(AstUnit& unit) {
	typedef StatementPointer SPtr;

	auto left = compile(unit[0]);
	auto right = compile(unit[2]);
	auto middleToken = unit[1].token;

	if (middleToken == "=") {
		return SPtr(new Assignment(left, right));
	}
	else {
		auto f = operatorFunctionMap.find(middleToken);
		if (f != operatorFunctionMap.end()) {
			return SPtr(new BinaryStatement(left, right, f->second));
		}
	}

	throw "binary statement not implemented";
}
