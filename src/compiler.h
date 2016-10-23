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
	CompilationException(Token token, const std::string what):
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

	static StatementPointer compile(AstUnit& unit);

	static StatementPointer createBinaryStatement(AstUnit& unit);

private:
	//Combine binary statements of type sequence to one object
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

