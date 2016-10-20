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
		{
			if (unit.children.size() == 1) {
				return compile(unit[0]);
			}
			else {
				throw "statement type cannot be compiled";
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
			else {
				throw "no variable name given in variable declaration";
			}
		}
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

