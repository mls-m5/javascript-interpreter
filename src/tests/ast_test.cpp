/*
 * ast_test.cpp
 *
 *  Created on: 13 okt. 2016
 *      Author: mattias
 */

#include "unittest.h"
#include "../ast.h"


TEST_SUIT_BEGIN

TEST_CASE("simple ast test") {
	AstUnit unit("simple ast test");

	ASSERT_EQ(unit.size(), 3);
	ASSERT_EQ(unit[0].token, "simple");
	ASSERT_EQ(unit[1].token, "ast");
	ASSERT_EQ(unit[2].token, "test");
}

TEST_CASE("simple group test") {
	{
		AstUnit unit("simple (group test)");

//		unit.print(std::cout);

		ASSERT(unit.size(), 2);
		ASSERT_EQ(unit[0].size(), 0);
		ASSERT_EQ(unit[1].size(), 2);
	}

	{
		AstUnit unit("simple {group test}");

//		unit.print(std::cout);

		ASSERT(unit.size(), 2);
		ASSERT_EQ(unit[0].size(), 0);
		ASSERT_EQ(unit[1].size(), 2);
	}


	{
		AstUnit unit("simple [group test]");

//		unit.print(std::cout);

		ASSERT(unit.size(), 2);
		ASSERT_EQ(unit[0].size(), 0);
		ASSERT_EQ(unit[1].size(), 2);
	}
}


TEST_SUIT_END
