/*
 * ast_test.cpp
 *
 *  Created on: 13 okt. 2016
 *      Author: mattias
 */

#include "unittest.h"
#include "../ast.h"


TEST_SUIT_BEGIN

TEST_CASE("grouping") {
	{
		AstUnit unit("{ x = 2 }");

		//	unit.print(std::cout);

		unit[0].groupUnit();

		//	unit.print(std::cout);

		ASSERT_EQ(unit.size(), 1);
		ASSERT_EQ(unit.type, unit.GenericGroup);
		ASSERT_EQ(unit[0].type, unit.Braces);
	}

	{
		AstUnit unit("{()}");

		ASSERT_EQ(unit.size(), 1);
		ASSERT_EQ(unit.type, unit.GenericGroup);
		ASSERT_EQ(unit[0].size(), 1);
		ASSERT_EQ(unit[0].type, unit.Braces);
		ASSERT_EQ(unit[0][0].type, unit.Parenthesis);
	}


	{
		SimpleLexer lexer;
		AstUnit unit; //This seems to be difficult to the ast-parser
		auto tokens = lexer.tokenize("( () {}  )");
		for (auto &it: tokens) {
			unit.children.push_back(std::shared_ptr<AstUnit>(new AstUnit(it)));
		}
		unit.type = unit.GenericGroup;
		unit.groupByParenthesis();
		unit.print(std::cout);

		ASSERT_EQ(unit.size(), 1);
		ASSERT_EQ(unit.type, unit.GenericGroup);
		ASSERT_EQ(unit[0].size(), 2);
		ASSERT_EQ(unit[0].type, unit.Parenthesis);
		ASSERT_EQ(unit[0][0].type, unit.Parenthesis);
		ASSERT_EQ(unit[0][1].type, unit.Braces);
	}
}

TEST_CASE("function test") {
	{
		AstUnit unit("function apa () { console.log('hej');} ");
		ASSERT_EQ(unit.type, unit.Function);
		ASSERT_EQ(unit[1].type, unit.Name);
	}
	{
		AstUnit unit("x = function apa() {}");

		auto f = unit.getByType(unit.Function);
		ASSERT(f, "no function found");
		auto a = f->getByType(unit.Arguments);
		ASSERT(a, "no arguments found");
	}

	{
		AstUnit unit("apa (x, y, z)");

		ASSERT_EQ(unit.size(), 2);
		ASSERT_EQ(unit[1].type, unit.Arguments);
	}
}

TEST_CASE("if-statement") {
	{
		AstUnit unit("if (x) {}");
		ASSERT_EQ(unit.type, unit.IfStatement);
		ASSERT(unit.getByType(unit.Condition), "could not find condition");
	}
	{
		//Else statement
		AstUnit unit("if (x) {} else {}");
		ASSERT_EQ(unit.type, unit.IfStatement);
		ASSERT_EQ(unit[0].type, unit.IfStatement);
		ASSERT(unit.getByType(unit.Braces), "no else statement found");
	}
	{
		//Else if statement
		AstUnit unit("if (x) {} else if(y) {}");
		ASSERT_EQ(unit.type, unit.IfStatement);
		ASSERT_EQ(unit[0].type, unit.IfStatement);
		ASSERT(unit[0].getByType(unit.Condition), "no condition found in if part");
		ASSERT(unit[0].getByType(unit.Braces), "no if statement found");
		ASSERT_EQ(unit[1].type, unit.ElseKeyword);
		ASSERT_EQ(unit[2].type, unit.IfStatement);
		ASSERT(unit[2].getByType(unit.Condition), "no condition found in else part");
		ASSERT(unit[2].getByType(unit.Braces), "no else statement found");
	}
	{
		AstUnit unit("if (x) { if (y) {}}");
		ASSERT_EQ(unit.type, unit.IfStatement);
		auto braces = unit.getByType(unit.Braces);
		braces->groupUnit();
		ASSERT(braces, "could not find statement of if statement");
		ASSERT_EQ((*braces)[0].type, unit.IfStatement);
	}
}

TEST_CASE("for loop") {
	AstUnit unit("for (let x = 0; x < 3; ++x) {console.log(x)}");
	ASSERT_EQ(unit.type, unit.ForLoop);
	ASSERT_NE(unit.front().type, unit.ForLoop);
}

TEST_CASE("variable declaration") {
	{
		AstUnit unit("var x = 10");
		ASSERT_EQ(unit.type, unit.VariableDeclaration);
		ASSERT_EQ(unit[1].type, unit.Assignment);
	}
	{
		AstUnit unit("let x = 10");
		ASSERT_EQ(unit.type, unit.VariableDeclaration);
		ASSERT_EQ(unit[1].type, unit.Assignment);
	}
}

TEST_CASE("literals") {
	AstUnit stringUnit("\"hej\"");
	ASSERT_EQ(stringUnit[0].type, stringUnit.String);

	AstUnit booleanUnit("true");
	ASSERT_EQ(booleanUnit[0].type, booleanUnit.Boolean);

	AstUnit numberUnit("1.0");
	ASSERT_EQ(numberUnit[0].type, numberUnit.Number);
}

TEST_CASE("multiple binary test") {
	AstUnit unit("x * y * z");

	ASSERT_EQ(unit.type, unit.BinaryStatement);
	ASSERT_EQ(unit.size(), 3);
	ASSERT_EQ(unit[0].type, unit.BinaryStatement);
}

TEST_CASE("keyword test") {
	AstUnit unit("function");

	ASSERT_EQ(unit[0].type, AstUnit::FunctionKeyword);
}

TEST_CASE("semicolon separated statements") {
	AstUnit unit("x = 1; y = 2");
//	unit.print(std::cout);
	ASSERT_EQ(unit[0].type, unit.Assignment);
	ASSERT_EQ(unit[1].type, unit.Semicolon);
	ASSERT_EQ(unit[2].type, unit.Assignment);
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

	{
		AstUnit unit("++ x");
		unit.print(std::cout);
		ASSERT_EQ(unit.type, unit.PrefixStatement);
	}
}

TEST_CASE("binary operator in condition") {
	AstUnit unit("(x < 1)");
	unit[0].type = unit.Condition;
	unit[0].groupUnit();
	ASSERT_EQ(unit[0][0].type, unit.BinaryStatement);
}

TEST_CASE("operator 3-15 (binary + conditional)") {
	std::vector<std::string> operators = {"**", "*", "+", "<<", "<", "==", "&", "|", "^", "&&", "||"};
	for (auto op: operators){
		AstUnit unit("apa " + op + " bepa");
		ASSERT_EQ(unit.type, unit.BinaryStatement);
	}
	{
		AstUnit unit("x = y");
		ASSERT_EQ(unit.type, unit.Assignment);
	}
	{
		AstUnit unit("23? x: y");
		ASSERT_EQ(unit.type, unit.Conditional);
	}
}

TEST_CASE("operator 4 - Conditional") {
	{
		AstUnit unit("23? x: y");
		ASSERT_EQ(unit.type, unit.Conditional);
	}
}

TEST_CASE("operator 0 - Sequences(coma)") {
	AstUnit unit("1, 2 ,3");

	ASSERT_EQ(unit.type, unit.Sequence);
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

TEST_CASE("basic object tests") {
	{
		AstUnit unit("{}");

//		unit.print(std::cout);

		ASSERT_EQ(unit.size(), 1);
		ASSERT_EQ(unit[0].type, unit.Braces);
	}

	{
		AstUnit unit("x: 10");

//		unit.print(std::cout);

		ASSERT_EQ(unit.type, unit.ObjectMemberDefinition);
	}


	{
		AstUnit unit("x: 10, y: function() {}");

//		unit.print(std::cout);

		ASSERT_EQ(unit.type, unit.Sequence);
		ASSERT_EQ(unit[0].type, unit.ObjectMemberDefinition);
	}
}

TEST_CASE("function definition as argument") {
	AstUnit unit("apa ( function() { x = 0 } )");

	unit.print(std::cout);
	ASSERT_EQ(unit.size(), 2);
	ASSERT_EQ(unit.type, unit.FunctionCall);
	unit.print(std::cout);
}

TEST_CASE("property accessor vs array") {
	AstUnit unit("b [3]");
	AstUnit unit2("[1, 2, 3]");

	ASSERT_EQ(unit.type, unit.PropertyAccessor);
	ASSERT_EQ(unit2.type, unit.Array);
}

TEST_CASE("return statement") {
	AstUnit unit("return 1");
	ASSERT_EQ(unit.type, unit.ReturnStatement);
}

TEST_CASE("object method call") {
	AstUnit unit("x[2]()");
	ASSERT_EQ(unit.type, unit.MethodCall);
}


TEST_SUIT_END
