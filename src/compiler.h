/*
 * compiler.h
 *
 *  Created on: 14 okt. 2016
 *      Author: mattias
 */

#pragma once

#include "ast.h"
#include "virtualmachine.h"
#include <functional>

class CompilationException {
public:
	CompilationException(Token token, std::string what):
		what(what),
		token(token) {}

	std::string what;
	Token token;
};

class Compiler {
private:
	Compiler() {};
public:
	static StatementPointer compile(std::string text) {
		AstUnit unit(text);
		return compile(unit);
	}

	static StatementPointer compile(AstUnit &unit) {
		Statement *statement = nullptr;

		switch (unit.type) {
		case unit.Word:
			statement = new LiteralStatement(unit.token);
		break;
		case unit.GenericGroup:
		case unit.Braces:
		{
			if (unit.type != unit.Braces && unit.children.size() == 1) {
				return compile(unit[0]);
			}
			else {
				unit.groupUnit();
				unit.print(std::cout);
				auto block = new CodeBlock();
				for (auto &u: unit) {
					auto s = compile(*u);
					u->print(std::cout);
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
		case unit.Function:
		{
			auto f = new FunctionDeclaration();
			if (auto declarationName = unit.getByType(unit.DeclarationName)) {
				f->name = declarationName->token;
			}

			statement = f;
		}
		break;
		case unit.BinaryStatement:
		{
			if (unit.size() != 3) {
				throw "failed to create binary statement: wrong number of arguments";
			}
			return createBinaryStatement(unit);
		}
		break;
		case unit.Arguments:
		{
			auto s = new ArgumentStatement();

			if (!unit.empty()) {
				cumulate(unit[0], s->statements);
			}

			statement = s;

		}
		break;
		case unit.FunctionCall:
		{
			auto fc = new FunctionCall();

			if (auto name = unit.children[0].get()) {
				fc->identifier = StatementPointer(compile(*name));
			}
			if (auto argumentUnit = unit.getByType(unit.Arguments)) {
				if (!unit.empty()) {
					cumulate(*argumentUnit->children[0].get(), fc->arguments.statements);
				}
			}

			statement = fc;
		}
		break;
		case unit.VariableDeclaration:
		{
			if (auto name = unit.getByType(unit.Name)) {
				auto vd = new VariableDeclaration(name->token);
				statement = vd;
			}
			else if (auto assignment = unit.getByType(unit.BinaryStatement)) {
//				unit.print(std::cout);
				if (assignment->size() != 3) {
					throw CompilationException(assignment->createToken(), "syntax error in variable declaration");
				}
				if ((*assignment)[1].token == "=") {
					statement = new VariableDeclaration((*assignment)[0].token, compile((*assignment)[2]));
				}
				else {
					throw CompilationException(assignment->createToken(), "unexpected token in variable declaration");
				}
			}
			else {
				throw CompilationException(unit.createToken(), "no variable name given in variable declaration");
			}
		}
		break;
		case unit.SemiColon:
			return StatementPointer(nullptr);
		break;
		}
		if (statement == nullptr) {
//			throw "statement could not be compiled";
			std::cout << "warning: zero statement" << endl;
		}
		return StatementPointer(statement);
	}

	static StatementPointer createBinaryStatement(AstUnit& unit);

private:
	static void cumulate(AstUnit &unit, vector<shared_ptr<Statement>> &statements) {
		if (unit.type == AstUnit::Sequence) {
			auto first = unit[0];
			auto second = unit[2];

			cumulate(first, statements);

			statements.push_back(compile(second));
		}
		else {
			statements.push_back(compile(unit));
		}
	};

};

