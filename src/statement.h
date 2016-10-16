/*
 * statement.h
 *
 *  Created on: 16 okt. 2016
 *      Author: mattias
 */

#pragma once

#include <memory>
#include <string>



typedef std::shared_ptr<Statement> StatementPointer;

class Statement{
public:
	virtual ~Statement() {}
	virtual Value run(ObjectValue &context) {
		throw "abstract class statement called";
	}

	virtual string toString() {
		return "statement";
	}

	Value call(ObjectValue &context, Value &expression) {
		ObjectValue localContext;
		localContext.parent = &context;
		localContext.defineVariable("arguments", expression);
		return this->run(localContext);
	}
};

//Check here for more info:
//https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Lexical_grammar
class LiteralStatement: public Statement {
public:
	LiteralStatement (const Token &value): value(value) {}

	string toString() override {
		return value;
	}

	Value run(ObjectValue &context) override {
		return context.getVariable(value);
	}

	Token value;
};

class StringLiteralStatement: public LiteralStatement {
public:
	StringLiteralStatement(const Token &value): LiteralStatement(value) {}

	Value run(ObjectValue &context) override {
		return value;
	}

};

class ArgumentStatement: public Statement {
public:
	vector<StatementPointer> statements;

	Value run(ObjectValue &context) override {
		auto array = new ObjectValue();

		for (int i = 0; i < statements.size(); ++i) {
			ostringstream ss;
			ss << i;
			array->setVariable(ss.str(), statements[i]->run(context));
		}

		return Value(*array);
	}
};


class Operator: public Statement{
	Operator(string value): value(value) {}

	Value run(ObjectValue &context) override {
		throw "operator not callable";
	}

	string toString() override {
		return value;
	}

	string value;
};


