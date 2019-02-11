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
	functionPointer(function){
		if (!left) {
			throw "expected left part of binary operator";
		}
		if (!right) {
			throw "expected right part of binary operator";
		}
	}


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

class NewTargetStatement: public Statement {
	Value run(ObjectValue &context) override {
		return context.getNewTarget();
	}
};

class PropertyAssignment: public Statement {
public:
	StatementPointer object;
	StatementPointer member;
	StatementPointer value;
	PropertyAssignment(StatementPointer object, StatementPointer member, StatementPointer value):
		object(object),
		member(member),
		value(value)
	{
	}

	Value run(ObjectValue &context) override {
		auto objectPtr = object->run(context).getObject();
		if (!objectPtr) {
			throw "no object defined";
		}
		auto memberValue = member->run(context);
		auto evalueatedValue = value->run(context);

		auto memberPtr = objectPtr->getOrDefineVariable(memberValue);
		if (memberPtr.type == Value::Reference) {
			*memberPtr.referencePtr = value->run(context);
			return memberPtr;
		}
		else {
			throw RuntimeException("variable " + memberValue.toString() + " not defined");
		}
	}
};

class Assignment: public BinaryStatement {
public:
	Assignment() = default;
	Assignment(StatementPointer left, StatementPointer right):
		BinaryStatement(left, right) {}

	Value run(ObjectValue &context) override {
		auto variable = left->run(context);
		auto value = right->run(context);
		if (variable.type == Value::Reference) {
			*variable.referencePtr = value;
			return variable;
		}
		else {
			throw RuntimeException("variable " + left->toString() + " not defined");
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


class ReturnStatement: public Statement {
public:
	StatementPointer block;

	~ReturnStatement() {}
	ReturnStatement(StatementPointer statement): block(statement) {}
	Value run(ObjectValue &context) override {
		auto value = block->run(context); //Make sure that it is not a reference when returning
		value.setReturnFlag();
		return value;
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
			return object->getVariable(member.run(context));
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

class CodeBlock: public Statement {
public:
	~CodeBlock() {}
	vector<StatementPointer> statements;

	Value run(ObjectValue &context) override {
//		map<string, Value> localVariables;
		Value ret;
		for (auto &statement: statements) {
			ret = statement->run(context);
			if (ret.isReturn()) {
				return ret;
			}
		}
		return ret;
	}
};



class FunctionDeclaration: public Statement {
public:
	StatementPointer block;

	Token name;
	shared_ptr<vector<Token>> argumentNames;

	~FunctionDeclaration() {}


	virtual string toString() {
		return "function";
	}

	//Do special difference except the arguments
	Value run(ObjectValue &context) override {
		Value function(new Function(context, block, argumentNames));
		if (!name.empty()) {
			context.setVariable(name, function);
		}
		return function;
	}
};


class NewStatement: public Statement {
public:
	StatementPointer identifier;
	StatementPointer arguments;

	NewStatement(StatementPointer identifier, StatementPointer arguments):
		identifier(identifier),
		arguments(arguments) {}

	virtual ~NewStatement() {}

	Value run(ObjectValue &context) override {
		auto creatorObject = identifier->run(context).getObject();

		if (creatorObject) {
			auto prototype = creatorObject->getVariable("prototype").getObject();

			if (!prototype) {
				throw RuntimeException("no prototype defined for " + identifier->toString());
			}

			auto newObject = new ObjectValue(prototype);
			auto constructor = prototype->getVariable("constructor").getObject();
			Value argumentsValue;
			if (arguments) {
				auto argumentsValue = arguments->run(context);
			}

			if (!constructor) {
				throw RuntimeException("cannot use new on object: No constructor function defined");
			}

			auto closure = new NewClosure(constructor->getDefinitionContext(), argumentsValue, newObject, creatorObject);
			auto returnValue = constructor->call(*closure);
			if (auto returnObject = returnValue.getObject()) {
				// If the function returns a object that will be returned instead of the
				// object used as this
				return returnObject;
			}
			else {
				return newObject;
			}
		}
		else {
			throw RuntimeException("could not create new value from non object");
		}
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

	Value run(ObjectValue &context) override {
		auto functionPtr = identifier->run(context).getObject();

		if (!functionPtr) {
			throw RuntimeException("trying to call non object/non function");
		}

		auto closure = new Closure(functionPtr->getDefinitionContext(), arguments.run(context), context.getThis());

		return functionPtr->call(*closure);
	}
};


class MethodCall: public FunctionCall {
public:
	StatementPointer object;
	//Inherit arguments and identifier from FunctionCall
	MethodCall(StatementPointer object, StatementPointer member, ArgumentStatement arguments):
		FunctionCall(member),
		object(object)
		{
		this->arguments = arguments;
	}

	Value run(ObjectValue &context) override {
		auto obj = object->run(context).getObject();
		if (obj == nullptr) {
			throw "trying to call method of non object";
		}

		auto method = obj->getVariable(identifier->run(context)).getObject();

		if (!method) {
			throw RuntimeException("trying to run non object as method");
		}

		auto closure = new Closure(method->getDefinitionContext(), arguments.run(context), obj);

		return method->call(*closure);
	}

};


class ObjectDefinition: public Statement {
public:
	//The first must be a literal statement
	vector<std::pair<StatementPointer, StatementPointer>> declarationPairs;

	Value run(ObjectValue &context) override {
		auto object = new ObjectValue();

		for (auto it: declarationPairs) {
			object->setVariable(it.first->toString(), it.second->run(context));
		}

		return Value(*object);
	}
};



void runGarbageCollection();

int getGlobalObjectCount();




