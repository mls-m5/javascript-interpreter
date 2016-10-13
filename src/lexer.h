/*
 * lexer.h
 *
 *  Created on: 12 okt. 2016
 *      Author: mattias
 */

#pragma once


#include <vector>
#include <string>
#include <sstream>

class Token: public std::string {
public:
	enum Type {
		Word,
		Operator,
		Paranthesis,
		Number,
		PropertyAccessor,
	};

	void clear() {
		std::string::clear();
		before.clear();
		after.clear();
	}

	std::string before;
	std::string after;
	int line;
	int row;
	Type type;

};

class Lexer {
public:
	virtual ~Lexer() {}
	virtual std::vector<Token> tokenize(std::string) = 0;
};


class SimpleLexer: public Lexer {
public:
	enum CharacterType {
		Letter = 0,
		Space,
		Operator,
		Paranthesis,
		Digit,
		Period,
	};

	CharacterType getCharType(char c) {
		return charTypes[c];
	}

	void setCharacterType(std::string characters, CharacterType type){
		for (int i = 0; i < characters.size(); ++i) {
			charTypes[characters[i]] = type;
		}
	}

	SimpleLexer() {
		for (auto &it: charTypes) {
			it = Letter;
		}

		setCharacterType("+-*/^.=<>", Operator);
		setCharacterType("[]()", Paranthesis);
		setCharacterType(" \n	", Space);
		setCharacterType("0123456789", Digit);
		setCharacterType(".", Period);
	}

	virtual std::vector<Token> tokenize(std::string text) override {
		std::istringstream ss(text);
		std::vector<Token> returnValue;
		Token word;
		std::string textBefore, textAfter;

		char c;
		CharacterType charType;
		ss.get(c);

		auto pushWord = [&textBefore, &textAfter, &word, &charType, &returnValue] () {
			switch(charType) {
			case SimpleLexer::Letter:
				word.type = Token::Word;
				break;
			case SimpleLexer::Operator:
				word.type = Token::Operator;
				break;
			case SimpleLexer::Paranthesis:
				word.type = Token::Paranthesis;
				break;
			case SimpleLexer::Space:
				returnValue.back().after += textAfter;
				return; // Do not add last space as a token
			case SimpleLexer::Digit:
				if (!returnValue.empty() && returnValue.back().after.empty()) {
					if (returnValue.back().type == Token::Word) {
						returnValue.back() += word;
						word.clear();
						return;
					} else if (returnValue.back().type == Token::PropertyAccessor) {
						returnValue.back() += word;
						returnValue.back().type = Token::Word;
						word.clear();
						return;
					}
				}
				word.type = Token::Number;
				break;
			case SimpleLexer::Period:
				word.type = Token::PropertyAccessor;
				if (!returnValue.empty() && returnValue.back().after.empty()) {
					if (returnValue.back().type == Token::Number) {
						returnValue.back() += word;
						word.clear();
						return;
					}
				}
				break;
			}
			word.before = textBefore;
			textBefore.clear();
			returnValue.push_back(word);
			word.clear();
		};

		while (!ss.eof()) {
			auto ct = getCharType(c);
			if (ct != charType && !word.empty()) {
				if (charType == Space) {
					textBefore = word;
					word.clear();
				}
				else {
					pushWord();
				}
			}
			word += c;
			charType = ct;
			ss.get(c);
		}
		if (!word.empty()) {
			pushWord();
		}
		return returnValue;
	}

	std::array<CharacterType, 255> charTypes;
};



