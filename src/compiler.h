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
		Statement *statement;

		switch (unit.type) {
		case unit.Function:
		{
			auto f = new FunctionDeclaration();
			if (auto declarationName = unit.getByType(unit.DeclarationName)) {
				f->name = declarationName->token;
			}

			statement = f;
		}

		break;
		}
		return StatementPointer(statement);
	}

};

