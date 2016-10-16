/*
 * compiler.h
 *
 *  Created on: 14 okt. 2016
 *      Author: mattias
 */

#pragma once

#include "ast.h"
#include "virtualmachine.h"

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

		}
		break;
		case unit.FunctionCall:
		{
			auto fc = new FunctionCall();

			if (auto argumentUnit = &*unit.children[1]) {
				auto arguments = new ArgumentStatement();
				if (argumentUnit->type == unit.Arguments) {
					arguments->statements.push_back(StatementPointer(compile(*argumentUnit)));
				}
				else {
					throw "multiple arguments not implemented";
				}
				fc->arguments = StatementPointer(arguments);

//				fc->arguments = StatementPointer(&arguments);
			}
			if (auto name = &*unit.children[0]) {
				fc->identifier = StatementPointer(compile(*name));
			}

			statement = fc;
		}

		break;
		}
		if (statement == nullptr) {
			throw "statement could not be compiled";
		}
		return StatementPointer(statement);
	}

};

