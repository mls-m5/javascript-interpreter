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

class BinaryStatement: public Statement {
public:
	typedef Value (Value::*MemberPointerType) (Value &);

	StatementPointer left;
	StatementPointer right;
	MemberPointerType functionPointer;

	~BinaryStatement() {}
	BinaryStatement(StatementPointer left, StatementPointer right, MemberPointerType function = nullptr):
	left(left),
	right(right),
	functionPointer(function){}


	Value run(ObjectValue &context) override {
		auto leftValue = left->run(context);
		auto rightValue = right->run(context);
		return (leftValue.*functionPointer)(rightValue);
	}
};

class Assignment: public BinaryStatement {
public:
	Assignment() = default;
	Assignment(StatementPointer left, StatementPointer right):
		BinaryStatement(left, right) {}

	Value run(ObjectValue &context) override {
		//Todo: in the more performant version this should be calculated in forehand
		auto variable = left->run(context);
		auto value = right->run(context);
		if (variable.type == Value::Reference) {
			*variable.referencePtr = value;
			return variable;
		}
		else {
			throw "variable not defined";
		}
	}
};

class VariableDeclaration: public Statement {
public:
	~VariableDeclaration() {}
	VariableDeclaration(Value name, Value value = Value()): name(name), value(value) {}

	Value name;
	Value value;
	Value run(ObjectValue &context) override {
		return context.defineVariable(name.run(context).toString(), value.run(context));
	}
};

class PropertyAccessor: public Identifier {
public:
	PropertyAccessor(Value object, Value member):
		object(object), member(member) {}

	Value run(ObjectValue &context) override {
		auto variable = object.run(context);//context.getVariable(object.toString());

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
	DeleteStatement(string identifier): identifier(identifier) {}

	Value run(ObjectValue &context) override {
		context.deleteVariable(identifier);

		return UndefinedValue;
	}


	string identifier;
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
	}

};

class FunctionDeclaration: public Statement {
public:
	CodeBlock block;

//	Identifier identifier;
	Token name;
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
	FunctionCall(StatementPointer identifier):
		identifier(identifier) {}
//	FunctionCall(StatementPointer identifier, Value arguments):
//	identifier(identifier), arguments(arguments){}

	StatementPointer identifier;
	ArgumentStatement arguments;

	//Todo make it possible to send arguments
	Value run(ObjectValue &context) override {
		auto functionValue = identifier->run(context).getValue();//context.getVariable(identifier);
//		auto functionValue =  context.getVariable(id.toString());

		if (functionValue.type != Value::Undefined) {
			if (arguments.statements.empty()) {
				return functionValue.call(context, UndefinedValue);
			}
			else {
				auto args = arguments.run(context);
				return functionValue.call(context, args);
			}
		}
		else {
			throw "not a function";
		}
	}
};

void runGarbageCollection();

int getGlobalObjectCount();




