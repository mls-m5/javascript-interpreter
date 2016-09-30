/*
 * identifier.h
 *
 *  Created on: 30 sep. 2016
 *      Author: mattias
 */

#pragma once
#include <string>
using std::string;


class Identifier {
public:
	Identifier() = default;
	Identifier(const Identifier&) = default;
	Identifier(Identifier &&) = default;
	Identifier(string name) : name(name) {};
	Identifier(const char name[]) : name(name) {};
	string name;

	//Todo: Add member functions and stuff
	//Todo add posibility to use fixed place in memory
};





