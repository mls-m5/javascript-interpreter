/*
 * vaule.h
 *
 *  Created on: 30 sep. 2016
 *      Author: mattias
 */

#pragma once

#include <string>
#include <memory>
#include <sstream>
#include <limits>
#include "token.h"
using namespace std;

class Value {
public:
	Value() = default;
	Value (const Value & value) {
		operator=(value);
	}
	Value (Value && value) {
		type = value.type;
		switch (type) {
		case String:
			stringValue = value.stringValue; //Transfer ownership of string
			value.stringValue = 0;
			break;
		case Object:
			objectPtr = value.objectPtr;
			break;
		case Integer:
			intValue = value.intValue;
			break;
		case Number:
			numberValue = value.numberValue;
			break;
		case StatementPointer:
			statementPtr = value.statementPtr;
			break;
		case Reference:
			referencePtr = value.referencePtr;
			break;
		case Undefined:
			break;
		default:
			throw "not implemented error in Value move constructor";
		}
	}

	Value (Value *reference) {
		//Pass the value by reference
		type = Reference;
		if (reference->type == Reference) {
			referencePtr = reference->referencePtr;
		}
		else {
			referencePtr = reference;
		}
	}

	//Skips possible identifiers
	Value &getValue() {
		if (type == Reference) {
			return *referencePtr;
		}
		else {
			return *this;
		}
	}

	Value (const string & value) {
		setValue(value);
	}

	Value (const char *value) {
		setValue(value);
	}

	Value (const long & value) {
		setValue(value);
	}

	Value (const int & value) {
		setValue((long)value);
	}

	Value (const double & value) {
		setValue(value);
	}

	Value (class ObjectValue &value) {
		setValue(value);
	}

	Value (class Statement &value) {
		setValue(value);
	}

	//Declared after object value
	Value run(class ObjectValue& context);


	//Returns identifier value
	Value operator =(const Value& value);

	~Value() {
		clear();
	}

	Value setValue(string value);

	Value setValue(long value) {
		clear();
		type = Integer;
		intValue = value;
		return *this;
	}

	Value setValue(double value) {
		clear();
		type = Number;
		numberValue = value;
		return *this;
	}

	Value setValue(class ObjectValue &value);

	Value setValue(class Statement &value) {
		clear();
		type = StatementPointer;
		statementPtr = &value;
		return *this;
	}

	ObjectValue *getObject() {
		if (type == Object) {
			return objectPtr;
		}
		else if (type == Reference) {
			return referencePtr->getObject();
		}
		else {
			return nullptr;
		}
	}

	Statement *getStatement() {
		if (type == StatementPointer) {
			return statementPtr;
		}
		else {
			return nullptr;
		}
	}

#define VALUE_OPERATOR(op) \
	Value operator op(Value &v) { \
		auto value = v.getValue(); \
		switch(type) { \
		case Integer: \
			if (value.type == Integer) { \
				return intValue op value.intValue; \
			} \
			else if (value.type == Number) { \
				return (double) intValue op value.numberValue; \
			} \
			else { \
				return intValue op value.toNumber(); \
			} \
			break; \
		case Number: \
			if (value.type == Number) { \
				return numberValue op value.numberValue; \
			} \
			else if(value.type == Integer) { \
				return numberValue op value.intValue; \
			} \
			else { \
				return numberValue op value.toNumber(); \
			} \
			break; \
		case Reference: \
			return getValue() op value; \
		} \
		return toNumber() op v.toNumber(); \
	}

	VALUE_OPERATOR(+)
	VALUE_OPERATOR(-)
	VALUE_OPERATOR(*)
	VALUE_OPERATOR(/)

	double toNumber() {
		//Not implemented
		cout << "warning: conversion to number is not implemented";
		return numeric_limits<double>::quiet_NaN();
	}

	Value call(ObjectValue& context, class Value& arguments);

	string toString();

	void clear();

	enum VariableType {
		Boolean,
		Null,
		Undefined,
		Integer,
		Number,
		String,
		Symbol,
		Object,
		Reference, //Only used for return value
		StatementPointer,
	} type = Undefined;

	//All types except objects are immutable objects
	union {
		bool boolValue;
		double numberValue;
		class ObjectValue* objectPtr;
		class StringValue* stringValue;
		class Statement* statementPtr; //Points to a placie in the abstract source tree
		Value *referencePtr;
		long intValue;
	};
};

extern Value UndefinedValue;
extern class ObjectValue window;
extern Value windowValue;

class ObjectValue{
public:
	virtual ~ObjectValue() {}

	virtual Value run(ObjectValue& context) {
		return Value();
	}

	// custom new operator for the object value
	static void* operator new(std::size_t sz);

	Value getVariable(string identifier, bool allowReference = true) {
		if (this == &window) {
			if (identifier == "window") {
				return windowValue;
			}
		}
		for (auto &it: children) {
			if (it.first == identifier) {
				if (allowReference) {
					return Value(&it.second); //Returns identifier
				}
				else {
					return it.second;
				}
			}
		}

		if (this->parent) {
			//Continue search
			return this->parent->getVariable(identifier);
		}
		else {
			return UndefinedValue; //Undefined
		}
	}

	vector<pair<string, Value>>::iterator getVariableIterator(string identifier) {
		for (auto it = children.begin(); it != children.end(); ++it) {
			if ((*it).first == identifier) {
				return it;
			}
		}

		return children.end();
	}

	Value setVariable(string identifier, Value value) {
		if (value.type == Value::Undefined) {
			throw "value not defined";
		}
		auto it = getVariableIterator(identifier);
		if (it == children.end()) {
			children.push_back(pair<string, Value>(identifier, value.getValue()));
			return &children.back().second;
		}
		else {
			it->second = value;
			return &it->second;
		}
	}

	//Declare and throw error if already defined
	Value defineVariable(string name, Value value = Value()) {
		auto it = getVariableIterator(name);
		if (it == children.end()) {
			children.push_back(pair<string, Value>(name, value));
			return &children.back().second;
		}
		else {
			throw "variable already declared";
		}
	}

	void deleteVariable(string identifier) {
		auto it = getVariableIterator(identifier);
		if (it == children.end()) {
			return;
		}

		children.erase(it);
	}

	operator string () {
		return toString();
	}


	virtual string toString() {
		string ret = "{";

		for (auto it: children) {
			ret += (it.first + ": " + it.second.toString() + ", ");
		}

		ret += "}";

		return ret;
	}

	vector<pair<string, Value>> children;
	ObjectValue *parent = nullptr;
	bool alive = true;
};


class StringValue {
public:
	StringValue() = default;
	StringValue(const StringValue &) = default;
	StringValue(StringValue &&) = default;
	StringValue(string value): value(value) {
	}
	virtual ~StringValue() {};
	std::string value;
};

#include "statement.h"

class Identifier: public Statement{
public:
	Identifier() = default;
	Identifier(const Identifier&) = default;
	Identifier(Identifier &&) = default;
	//If createNew is true a variable is created if it does not exist
	Identifier(Token name, bool createNew = false) : name(name), createNew(createNew) {};
	Identifier(const char name[]) : name(name) {};
	Token name;
	bool createNew = false;

	Value run(ObjectValue& context) override {
		return context.getVariable(name);
	}
	string toString() override {
		return name;
	}

	//Todo: Add member functions and stuff
	//Todo add posibility to use fixed place in memory
};


inline Value Value::run(class ObjectValue& context) {
	if (type == StatementPointer) {
		return statementPtr->run(context);
	} else if (type == Object) {
		return objectPtr->run(context);
	} else if (type == Reference) {
		return referencePtr->run(context);
	} else {
		return *this;
	}
}

inline Value Value::operator =(const Value& value) {
//	if (type == Reference) {
//#error "this seems to be wrong"
//		*referencePtr = value;
//	}
	clear();
	switch (value.type) {
	case Object:
		objectPtr = value.objectPtr;
		break;
	case String:
		stringValue = new StringValue(*value.stringValue);
		break;
	case StatementPointer:
		statementPtr = value.statementPtr;
		break;
	case Undefined:
		break;
	case Reference:
		return *this = *value.referencePtr;
	case Integer:
		intValue = value.intValue;
		break;
	case Number:
		numberValue = value.numberValue;
		break;
	default:
		throw "assignment not implemented";
	}
	type = value.type;
	return this;
}

inline Value Value::setValue(string value) {
	clear();
	type = String;
	stringValue = new StringValue(value);
	return *this;
}

inline Value Value::call(ObjectValue& context, class Value& arguments) {
	if (type == StatementPointer) {
		if (!statementPtr) {
			throw "trying to call null statement";
		}
		return statementPtr->call(context, arguments);
	} else if (type == Reference) {
		return referencePtr->call(context, arguments);
	} else {
		throw "value is not callable";
	}
}

inline string Value::toString() {
	switch (type) {
	case Undefined:
		return "Undefined";
	case String:
		return stringValue->value;
	case Object:
		return objectPtr->toString();
	case StatementPointer:
		return statementPtr->toString();
	case Reference:
		return referencePtr->toString();
	case Integer:
	{
		ostringstream ss;
		ss << intValue;
		return ss.str();
	}
	case Number:
	{
		ostringstream ss;
		ss << numberValue;
		return ss.str();
	}
	default:
		return "string conversion not implemented";
	}
}

inline Value Value::setValue(ObjectValue &value) {
	type = Object;
	objectPtr = &value;
	return *this;
}


inline void Value::clear() {
	//Delete the value;
	switch (type) {
	case String:
		//The string value is "owned" by the Value class
		delete stringValue; //The only type to be deleted
		break;
	}

	type = Undefined;
}

