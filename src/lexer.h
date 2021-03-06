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
#include <array>
#include "token.h"

class Lexer {
public:
	virtual ~Lexer() {}
	virtual std::vector<Token> tokenize(std::string) = 0;
	virtual std::vector<Token> tokenize(std::istream &stream) = 0;
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
		Citation,
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

		setCharacterType("+-*/^.=<>,:;!", Operator);
		setCharacterType("[](){}", Paranthesis);
		setCharacterType(" \n	", Space);
		setCharacterType("0123456789", Digit);
		setCharacterType("\"'", Citation);
		setCharacterType(".", Period);
	}

	virtual std::vector<Token> tokenize(std::string text) override {
		std::istringstream ss(text);
		return tokenize(ss);
	}

	virtual std::vector<Token> tokenize(std::istream &ss) override {
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
			case SimpleLexer::Citation:
				word.type = Token::StringLiteral;
				break;
			case SimpleLexer::Paranthesis:
				word.type = Token::Paranthesis;
				if (word.size() > 1) {
					std::string tmpWord = word;
					for (size_t i = 0; i < tmpWord.size(); ++i) {
						word.assign(tmpWord.substr(i, 1));
						word.before = textBefore;
						textBefore.clear();
						returnValue.push_back(word);
						word.clear();
					}
					return;
				}
				break;
			case SimpleLexer::Space:
				if (returnValue.empty()) {
					textBefore += word;
					word.clear();
				}
				else {
					returnValue.back().after += word;
					word.clear();
				}
				return; // Do not add last space as a token
			case SimpleLexer::Digit:
				if (!returnValue.empty() && returnValue.back().after.empty() && textBefore.empty()) {
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

		auto getCitation = [&word, &c, &ss] (char startSign) {
			ss.get(c);
			while(!ss.eof() && c != startSign) {
				//Todo: fix and test more
				word += c;
				ss.get(c);
			}
		};

		auto checkComment = [&word, &c, &ss, &charType, pushWord] () {
			ss.get(c);
			if (c == '/') {
				if (!word.empty()) {
					if (charType != Space) {
						pushWord();
					}
				}

				word.push_back('/');
				while (!ss.eof()) {
					word.push_back(c);
					ss.get(c);
					if (c == '\n') {
						word.push_back('\n');
						break;
					}
				}
				charType = SimpleLexer::Space;
				pushWord();

				return true;
			}
			else if (c == '*') {
				//Block comment
				word.push_back('/');

				while (!ss.eof()) {
					word.push_back(c);
					ss.get(c);
					if (c == '*') {
						word.push_back(c);
						if (ss.eof()) {
							throw "error: got eof when expecting end of comment";
						}
						ss.get(c);
						if (c == '/') {
							word.push_back(c);
							break;
						}
						else {
							ss.unget();
						}
					}

				}
				charType = SimpleLexer::Space;
				pushWord();

				return true;
			}
			else {
				ss.unget(); //False alarm, continue as usual
				return false;
			}
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
			if (ct == Citation) {
				getCitation(c);
				charType = SimpleLexer::Citation;
				pushWord();
				ss.get(c);
				continue;
			}
			if (c == '/') {
				if (checkComment()) {
					ss.get(c);
					continue;
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



