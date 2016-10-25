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

class UnaryStatement: public Statement {
public:
	typedef Value (Value::*MemberPointerType) ();

	StatementPointer statement;
	MemberPointerType functionPointer;

	UnaryStatement(StatementPointer statement, MemberPointerType function):
	statement(statement),
	functionPointer(function){}

	Value run(ObjectValue &context) override {
		auto statementValue = statement->run(context);
		return (statementValue.*functionPointer)();
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
	VariableDeclaration(Value name, StatementPointer value = StatementPointer()): name(name), value(value) {}

	Value name;
	StatementPointer value;
	Value run(ObjectValue &context) override {
		if (value) {
			return context.defineVariable(name.run(context).toString(), value->run(context));
		}
		else {
			return context.defineVariable(name.run(context).toString());
		}
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
	vector<StatementPointer> statements;

	Value run(ObjectValue &context) override {
//		map<string, Value> localVariables;
		Value ret;
		for (auto &statement: statements) {
			ret = statement->run(context);
		}
		return ret;
	}
};

//A class that manages function argument names
class FunctionBlock: public Statement {
public:
	vector<Token> argumentNames;

	StatementPointer block;


	Value run(ObjectValue &context) override {
		Value arguments = context.getVariable("arguments");
		if (auto o = arguments.getObject()) {
			for (int i = 0; i < argumentNames.size(); ++i) {
				Value index(i);
				auto argument = o->getVariable(index.toString());
				context.setVariable(argumentNames[i], argument, true);
			}
		}
		block->run(context);
	}
};

class FunctionDeclaration: public Statement {
public:
	StatementPointer block;

	Token name;
	~FunctionDeclaration() {}

	//Do special difference except the arguments
	Value run(ObjectValue &context) override {
		if (!name.empty()) {
			context.setVariable(name, *block);
		}
		return *block;
	}
};

class IfStatement: public Statement {
public:
	IfStatement(StatementPointer condition, StatementPointer block):
		conditions({condition}),
		blocks({block}) {}
	IfStatement() {};

	std::vector<StatementPointer> conditions, blocks;

	Value run(ObjectValue &context) override {
		for (ptrdiff_t i = 0; i < conditions.size(); ++i) {
			auto &condition = conditions[i];
			auto &block = blocks[i];
			if (condition->run(context)) {
				return block->run(context);
			}
		}
		//Check if there is another else block (without condition)
		if (blocks.size() > conditions.size()) {
			return blocks.back()->run(context);
		}
		else {
			return UndefinedValue;
		}
	}
};

class WhileLoop: public Statement {
public:
	WhileLoop(StatementPointer condition, StatementPointer block):
	condition(condition), block(block){}
	StatementPointer condition, block;


	Value run(ObjectValue &context) override {
		Value returnValue;

		while (auto c = condition->run(context)) {
			returnValue = block->run(context);
		}
		return returnValue;
	}
};

class ForLoop: public Statement {
public:
	ForLoop(StatementPointer initialization, StatementPointer condition, StatementPointer increment, StatementPointer block):
	initialization(initialization), condition(condition), increment(increment), block(block){}
	StatementPointer initialization, condition, increment, block;


	Value run(ObjectValue &context) override {
		Value returnValue;
		initialization->run(context);
		while (auto c = condition->run(context)) {
			returnValue = block->run(context);
			increment->run(context);
		}
		return returnValue;
	}
};

class FunctionCall: public Statement {
public:
	~FunctionCall() {}
	FunctionCall() = default;
	FunctionCall(const FunctionCall &) = default;
	FunctionCall(StatementPointer identifier):
		identifier(identifier) {}

	StatementPointer identifier;
	ArgumentStatement arguments;

	//Todo make it possible to send arguments
	Value run(ObjectValue &context) override {
		auto functionValue = identifier->run(context).getValue();//context.getVariable(identifier);

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




