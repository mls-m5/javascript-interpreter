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
public:
	StatementPointer compile(std::string text) {
		AstUnit unit(text);
		return compile(unit);
	}

	StatementPointer compile(AstUnit &unit) {
		Statement *statement = nullptr;

		switch (unit.type) {
		case unit.Word:
			statement = new LiteralStatement(unit.token);
		break;
		case unit.String:
			statement = new StringLiteralStatement(unit.token);
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
		}
		if (statement == nullptr) {
//			throw "statement could not be compiled";
		}
		return StatementPointer(statement);
	}

private:
	void cumulate(AstUnit &unit, vector<shared_ptr<Statement>> &statements) {
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

