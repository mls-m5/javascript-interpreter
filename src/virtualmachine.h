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



class Context {
public:
	map<string, Value> variables;

	Context *parentContext = 0;

	Value &getVariable(Identifier identifier) {
		auto f = variables.find(identifier.name);

		if (f == variables.end() && parentContext) {
			return parentContext->getVariable(identifier);
		}
		else {
			return f->second;
		}
	}

	Value setVariable(Identifier identifier, Value value) {
		variables[identifier.name] = value;
		return value;
	}
};

class Statement{
public:
	virtual ~Statement() {}
	virtual Value run(Context &context) {
		throw "abstract class statement called";
	}

	virtual Statement *copy() const  = 0; //{
//		return new Statement(*this);
//	}
};

class Callable {
public:
	virtual ~Callable();

	virtual Value call(Context &context, class Expression &arguments) {
		throw "cannot call statement";
	}
};

class Expression: public ObjectValue {
public:
	Expression(const Expression &e) {
		statement.reset(e.statement->copy());
	}
	Expression(Expression &&e) {
		statement = move(e.statement);
	}
	Expression(const Statement &s) {
		statement.reset(s.copy());
	}

	Value run(Context &context) {
		return statement->run(context);
	}

	unique_ptr<Statement> statement;
};


class Assignment: public Statement {
public:
	~Assignment() {}
	Assignment() = default;
	Assignment(Identifier identifier, Expression expression):
	identifier(identifier),
	expression(expression) {}

	Identifier identifier;
	Expression expression;

	Value run(Context &context) override {
		//Todo: in the more performant version this should be calculated in forehand
		auto value = expression.run(context);

		return context.setVariable(identifier, value);
	}

	Statement *copy() const override {
		return new Assignment(*this);
	}
};

class CodeBlock: public Statement, public Callable {
public:
	~CodeBlock() {}
	vector<Expression> statements;

	Value run(Context &context) override {
		map<string, Value> localVariables;
		for (auto &statement: statements) {
			statement.run(context);
		}
		//unload scoped "let" variables
	}

	Value call(Context &context, Expression &expression) override {
		Context localContext;

		localContext.parentContext = &context;
		localContext.setVariable("arguments", expression.run(context));
		return this->run(localContext);
	}


	Statement *copy() const override {
		return new CodeBlock(*this);
	}
};

class FunctionDeclaration: public CodeBlock {
public:
	~FunctionDeclaration() {}

	//Do special difference except the arguments
	Value run(Context &context) override {
		Context localContext;
		localContext.parentContext = &context;

		auto value = this->CodeBlock::run(localContext);

		//unload context variables

		return value;
	}

	Statement *copy() const override {
		return new FunctionDeclaration(*this);
	}
};

class FunctionCall: public Statement {
	~FunctionCall() {}
	FunctionCall() = default;
	FunctionCall(const FunctionCall &) = default;
	FunctionCall(Identifier identifier):
		identifier(identifier) {}

//	shared_ptr<FunctionDeclaration> function;
	Identifier identifier;

	//Todo make it possible to send arguments
	Value run(Context &context) override {
		auto functionValue = context.getVariable(identifier);

		if (functionValue.type != Value::Undefined) {
			return functionValue.run(context);
		}
		else {
			return Value();
		}
	}

	Statement *copy() const override {
		return new FunctionCall(*this);
	}
};

class ConsoleLog: public FunctionDeclaration {
	Value run(Context &context) override {
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


	Value run(Context &context) override {
		return context.getVariable(variableName);
	}


	Statement *copy() const override {
		return new VariableGetter(*this);
	}

	Identifier variableName;
};


