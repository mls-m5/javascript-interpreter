/*
 * compiler_test.cpp
 *
 *  Created on: 15 okt. 2016
 *      Author: mattias
 */


#include "unittest.h"
#include "../compiler.h"

TEST_SUIT_BEGIN

TEST_CASE("Simple conversion test") {
	Compiler compiler;
	{
		StatementPointer statement = compiler.compile("function apa() {}");

		auto f = dynamic_cast<FunctionDeclaration*> (statement.get());

		ASSERT(f, "statement is not a function");

		ASSERT_EQ(f->name, "apa");
	}
	{
		auto statement = compiler.compile("apa()");

		auto call = dynamic_cast<FunctionCall*> (statement.get());

		ASSERT(call, "statement is not a function call");

//		ASSERT_EQ(call->identifier.toString(), apa);
	}
}


TEST_SUIT_END


