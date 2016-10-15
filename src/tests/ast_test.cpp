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

TEST_CASE("function test") {
	{
		AstUnit unit("function apa () { console.log('hej');} ");
		ASSERT_EQ(unit.type, unit.Function);
		ASSERT_EQ(unit[1].type, unit.DeclarationName);
	}
	{
		AstUnit unit("x = function apa() {}");

		auto f = unit.getByType(unit.Function);
		ASSERT(f, "no function found");
		auto a = f->getByType(unit.Arguments);
		ASSERT(a, "no arguments found");
	}
}

TEST_CASE("keyword test") {
	AstUnit unit("function");

	ASSERT_EQ(unit[0].type, AstUnit::FunctionKeyword);
}

TEST_CASE("operator 19") {
	{
		AstUnit unit("new Apa (x)"); //New with arguments
		ASSERT_EQ(unit.type, unit.NewStatement);
	}
}

TEST_CASE("operator 18") {
	{
		AstUnit unit("apa ()");
		ASSERT_EQ(unit.type, unit.FunctionCall);
	}
	{
		AstUnit unit("new apa"); //New without arguments
		ASSERT_EQ(unit.type, unit.NewStatement);
	}
}

TEST_CASE("operator 17") {
	{
		AstUnit unit("x++");
		ASSERT_EQ(unit.type, unit.PostfixStatement);
	}
}


TEST_CASE("operator 16") {
	{
		AstUnit unit("typeof x");
		ASSERT_EQ(unit.type, unit.PrefixStatement);
	}

	{
		AstUnit unit("delete x");
		ASSERT_EQ(unit.type, unit.PrefixStatement);
	}
}


TEST_CASE("operator 3-15 (binary + conditional)") {
	std::vector<std::string> operators = {"**", "*", "+", "<<", "<", "==", "&", "|", "^", "&&", "||", "="};
	for (auto op: operators){
		AstUnit unit("apa " + op + " bepa");
		ASSERT_EQ(unit.type, unit.BinaryStatement);
	}
	{
		AstUnit unit("23? x: y");
		ASSERT_EQ(unit.type, unit.Conditional);
	}
}

TEST_CASE("operator 0 - Coma sequence") {
	{
		AstUnit unit("23? x: y");
		ASSERT_EQ(unit.type, unit.Conditional);
	}
}



TEST_CASE("simple group test") {
	{
		AstUnit unit("simple (group test)");


		ASSERT_EQ(unit.size(), 2);
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

		ASSERT(unit.size(), 2);
		ASSERT_EQ(unit[0].size(), 0);
		ASSERT_EQ(unit[1].size(), 2);
	}
}


TEST_SUIT_END
