/*
 * lexer_test.cpp
 *
 *  Created on: 13 okt. 2016
 *      Author: mattias
 */

#include "unittest.h"
#include "../lexer.h"

TEST_SUIT_BEGIN

TEST_CASE("simple tokenizer test") {
	SimpleLexer lexer;

	auto tokens = lexer.tokenize("hej då re");

	ASSERT_EQ(tokens.size(), 3);
	ASSERT_EQ(tokens[0], "hej");
	ASSERT_EQ(tokens[1], "då");
	ASSERT_EQ(tokens[2], "re");
}

TEST_CASE("character type test") {
	SimpleLexer lexer;
	ASSERT_EQ(lexer.getCharType('a'), lexer.Letter);
	ASSERT_EQ(lexer.getCharType('.'), lexer.Period);
	ASSERT_EQ(lexer.getCharType('*'), lexer.Operator);
	ASSERT_EQ(lexer.getCharType(' '), lexer.Space);
	ASSERT_EQ(lexer.getCharType(')'), lexer.Paranthesis);
	ASSERT_EQ(lexer.getCharType('='), lexer.Operator);
	ASSERT_EQ(lexer.getCharType(';'), lexer.Operator);
}

TEST_CASE("string literal test") {
	SimpleLexer lexer;
	auto tokens = lexer.tokenize("('hej')\"då\";");
	ASSERT_EQ(tokens.size(), 5);
	ASSERT_EQ(tokens[0].type, Token::Paranthesis);
	ASSERT_EQ(tokens[1].type, Token::StringLiteral);
	ASSERT_EQ(tokens[2].type, Token::Paranthesis);
	ASSERT_EQ(tokens[3].type, Token::StringLiteral);
	ASSERT_EQ(tokens[4].type, Token::Operator);
}

TEST_CASE("paranthesis separation") {
	SimpleLexer lexer;
	auto tokens = lexer.tokenize("{}");
	ASSERT_EQ(tokens.size(), 2);
	ASSERT_EQ(tokens[0].type, Token::Paranthesis);
	ASSERT_EQ(tokens[1].type, Token::Paranthesis);
}

TEST_CASE("more advanced tokenizer test") {
	SimpleLexer lexer;

	auto tokens = lexer.tokenize("Hej43 tjoho(x.y - ++i); !=");

	ASSERT_GT(tokens.size(), 11);
	ASSERT_EQ(tokens[0], "Hej43");
	ASSERT_EQ(tokens[0].type, Token::Word);
	ASSERT_EQ(tokens[1], "tjoho");
	ASSERT_EQ(tokens[1].type, Token::Word);
	ASSERT_EQ(tokens[2], "(");
	ASSERT_EQ(tokens[2].type, Token::Paranthesis);
	ASSERT_EQ(tokens[3], "x");
	ASSERT_EQ(tokens[4], ".");
	ASSERT_EQ(tokens[4].type, Token::PropertyAccessor);
	ASSERT_EQ(tokens[5], "y");
	ASSERT_EQ(tokens[6], "-");
	ASSERT_EQ(tokens[7], "++");
	ASSERT_EQ(tokens[8], "i");
	ASSERT_EQ(tokens[9], ")");
	ASSERT_EQ(tokens[10], ";");
	ASSERT_EQ(tokens[11], "!=");
	ASSERT_EQ(tokens.size(), 12);
}

TEST_CASE("comments") {
	SimpleLexer lexer;
	{
		auto tokens = lexer.tokenize("hej//Comment\n2");

		for (auto it: tokens) {
			std::cout << it << ", " << it.type << std::endl;
		}

		ASSERT_EQ(tokens.size(), 2);
		ASSERT_EQ(tokens[0].type, Token::Word);
		ASSERT_EQ(tokens[1].type, Token::Number);
	}
	{

		auto tokens = lexer.tokenize("hej/*Comment*/2");

		for (auto it: tokens) {
			std::cout << it << ", " << it.type << std::endl;
		}

		ASSERT_EQ(tokens.size(), 2);
		ASSERT_EQ(tokens[0].type, Token::Word);
		ASSERT_EQ(tokens[1].type, Token::Number);
	}

}

TEST_SUIT_END


