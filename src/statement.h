/*
 * statement.h
 *
 *  Created on: 16 okt. 2016
 *      Author: mattias
 */

#pragma once

#include <memory>
#include <string>



typedef std::shared_ptr<class Statement> StatementPointer;

class Statement {
public:
	virtual ~Statement() {}
	virtual Value run(ObjectValue &context) {
		throw "abstract class statement called";
	}

	virtual string toString() {
		return "statement";
	}
};

class ThisStatement: public Statement {
	Value run(ObjectValue &context) override {
		return context.thisPointer();
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


class NumberLiteralStatement: public LiteralStatement {
public:
	Value numberValue;

	NumberLiteralStatement(const Token &value): LiteralStatement(value) {
		if (value == "NaN") {
			numberValue = NaN;
			return;
		}
		else if (value == "Infinity") {
			numberValue = Infinity;
			return;
		}
		std::istringstream ss(value);

		double v;
		ss >> v;
		numberValue = v;
	}

	Value run(ObjectValue &context) override {
		return numberValue;
	}
};

class BooleanLiteralStatement: public LiteralStatement {
public:
	Value boolValue;

	BooleanLiteralStatement(const Token &value): LiteralStatement(value) {
		if (value == "true") {
			boolValue = true;
		}
		else if (value == "false") {
			boolValue = false;
		}
		else {
			throw "value not boolean in boolean literal constructor";
		}
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


