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
