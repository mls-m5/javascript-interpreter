/*
 * virtualmachine.h
 *
 *  Created on: 30 sep. 2016
 *      Author: mattias
 */

#pragma once

#include <iostream>
#include <memory>
#include <map>
#include <vector>

#include "value.h"

class Callable {
public:
	virtual ~Callable() {}

	virtual Value call(ObjectValue &context, class Value &arguments) {
		throw "cannot call statement";
	}
};


class Assignment: public Statement {
public:
	~Assignment() {}
	Assignment() = default;
	Assignment(Identifier identifier, Value expression):
	identifier(identifier),
	expression(expression) {}

	Identifier identifier;
	Value expression;

	Value run(ObjectValue &context) override {
		//Todo: in the more performant version this should be calculated in forehand
		auto value = expression.run(context);

		context.setVariable(identifier, value);
		return value;
	}
};

class PropertyAccessor: public Statement {
public:
	PropertyAccessor(Value object, Value member):
		object(object), member(member) {}

	Value run(ObjectValue &context) override {
		auto variable = context.getVariable(object.toString());

		if (variable.type == Value::Undefined) {
			throw "variable " + object.run(context).toString() + " is not defined";
		}

		auto object = variable.getObject();
		if (object) {
			return object->getVariable(member.run(context).toString());
		}
		else {
			throw "variable is not a object";
		}

	}

	Value object;
	Value member;
};

class DeleteStatement: public Statement {
public:
	DeleteStatement() = default;
	DeleteStatement(Identifier identifier): identifier(identifier) {}

	Value run(ObjectValue &context) override {
		context.deleteVariable(identifier);

		return UndefinedValue;
	}


	Identifier identifier;
};

class CodeBlock: public Statement, public Callable {
public:
	~CodeBlock() {}
	vector<Value> statements;

	Value run(ObjectValue &context) override {
		map<string, Value> localVariables;
		for (auto &statement: statements) {
			statement.run(context);
		}
		//unload scoped "let" variables
	}

};

class FunctionDeclaration: public Statement {
public:
	CodeBlock block;

	Identifier identifier;
	~FunctionDeclaration() {}

	//Do special difference except the arguments
	Value run(ObjectValue &context) override {
		return Value(block);
	}
};

class FunctionCall: public Statement {
public:
	~FunctionCall() {}
	FunctionCall() = default;
	FunctionCall(const FunctionCall &) = default;
	FunctionCall(Identifier identifier):
		identifier(identifier) {}
	FunctionCall(Identifier identifier, Value arguments):
	identifier(identifier), arguments(arguments){}

	Identifier identifier;
	Value arguments;

	//Todo make it possible to send arguments
	Value run(ObjectValue &context) override {
		auto functionValue = context.getVariable(identifier);

		if (functionValue.type != Value::Undefined) {
			return functionValue.call(context, arguments);
		}
		else {
			return Value();
		}
	}
};

class VariableGetter: public Statement {
public:
	~VariableGetter() {};
	VariableGetter() = default;
	VariableGetter(const VariableGetter &) = default;
	VariableGetter(VariableGetter &&) = default;
	VariableGetter(Identifier identifier): variableName(identifier) {}


	Value run(ObjectValue &context) override {
		return context.getVariable(variableName);
	}

	Identifier variableName;
};

void runGarbageCollection();

int getGlobalObjectCount();




