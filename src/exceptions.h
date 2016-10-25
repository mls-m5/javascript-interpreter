/*
 * exceptions.h
 *
 *  Created on: 25 okt. 2016
 *      Author: mattias
 */

#pragma once

class GenericException {
public:
	GenericException(std::string what): what(what){}
	std::string what;
};


class CompilationException: public GenericException {
public:
	CompilationException(Token token, const std::string what):
		GenericException(what),
		token(token) {}

	Token token;
};

class RuntimeException: public GenericException {
public:
	RuntimeException(std::string what): GenericException(what) {};
};


