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

	static StatementPointer compile(std::istream &stream) {
		AstUnit unit(stream);
		return compile(unit);
	}

	static void run(std::string text, ObjectValue &context) {
		auto statement = compile(text);
		statement->run(context);
	}

	static StatementPointer compile(AstUnit& unit);

	static StatementPointer createBinaryStatement(AstUnit& unit);
	static StatementPointer createPrefixStatement(AstUnit& unit);

private:
	//Combine binary statements of type sequence to one object
	static void cumulate(AstUnit &unit, vector<shared_ptr<Statement>> &statements, bool literals = false) {
		if (unit.type == AstUnit::Sequence) {
			auto first = unit[0];
			auto second = unit[2];

			cumulate(first, statements, literals);

			if (literals) {
				statements.push_back(StatementPointer(new StringLiteralStatement(second.token)));
			}
			else {
				statements.push_back(compile(second));
			}
		}
		else {
			if (literals) {
				statements.push_back(StatementPointer(new StringLiteralStatement(unit.token)));
			}
			else {
				statements.push_back(compile(unit));
			}

		}
	};

};
