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

	virtual Value call(ObjectValue &context, class Expression &arguments) {
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

	Value call(ObjectValue &context, Expression &expression) override {
		//Todo: Fix parent value of ObjectValue
//		localObjectValue.parentObjectValue = &context;
//		localObjectValue.setVariable("arguments", expression.run(context));
//		return this->run(localObjectValue);
	}

	ObjectValue localObjectValue;
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
//
//	Statement *copy() const override {
//		return new FunctionDeclaration(*this);
//	}
};

class FunctionCall: public Statement {
public:
	~FunctionCall() {}
	FunctionCall() = default;
	FunctionCall(const FunctionCall &) = default;
	FunctionCall(Identifier identifier):
		identifier(identifier) {}

//	shared_ptr<FunctionDeclaration> function;
	Identifier identifier;

	//Todo make it possible to send arguments
	Value run(ObjectValue &context) override {
		auto functionValue = context.getVariable(identifier);

		if (functionValue.type != Value::Undefined) {
			return functionValue.run(context);
		}
		else {
			return Value();
		}
	}

//	Statement *copy() const override {
//		return new FunctionCall(*this);
//	}
};

class ConsoleLog: public FunctionDeclaration {
	Value run(ObjectValue &context) override {
		cout << context.getVariable("arguments").toString() << endl;

		return Value();
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


//	Statement *copy() const override {
//		return new VariableGetter(*this);
//	}

	Identifier variableName;
};


