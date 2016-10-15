/*
 * token.h
 *
 *  Created on: 15 okt. 2016
 *      Author: mattias
 */

#pragma once
#include <string>



class Token: public std::string {
public:
	enum Type {
		Word,
		Operator,
		Paranthesis,
		Number,
		PropertyAccessor,
	};

	Token() = default;
	Token(const char *token): std::string(token) {}

	void clear() {
		std::string::clear();
		before.clear();
		after.clear();
	}

	std::string before;
	std::string after;
	int line = 0;
	int row = 0;
	Type type = Word;

};


