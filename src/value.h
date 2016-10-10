/*
 * vaule.h
 *
 *  Created on: 30 sep. 2016
 *      Author: mattias
 */

#pragma once

#include <string>
#include <memory>
#include "identifier.h"
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
		case StatementPointer:
			statementPtr = value.statementPtr;
			break;
		default:
			throw "not implemented error in Value move constructor";
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

	Value (class ObjectValue &value) {
		setValue(value);
	}

	Value (class Statement &value) {
		setValue(value);
	}

	//Declared after object value
	Value run(class ObjectValue& context);


	Value& operator =(const Value& value);

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
		else {
			return 0;
		}
	}

	string toString();

	void clear();

	enum VariableType {
		Boolean,
		Null,
		Undefined,
		Number,
		String,
		Symbol,
		Object,
		Integer,
		StatementPointer,
	} type = Undefined;

	//All types except objects are immutable objects
	union {
		bool boolValue;
		double numberValue;
		class ObjectValue* objectPtr;
		class StringValue* stringValue;
		class Statement* statementPtr; //Points to a placie in the abstract source tree
		long intValue;
	};
};

extern Value UndefinedValue;

class ObjectValue{
public:
	virtual ~ObjectValue() {}

	Value run(class Context& context) {
		return Value();
	}

	Value &getVariable(Identifier identifier) {
		for (auto &it: children) {
			if (it.first == identifier.name) {
				return it.second;
			}
		}

		return UndefinedValue; //Undefined
	}

	void setVariable(Identifier identifier, Value value) {
		if (value.type == Value::Undefined) {
			throw "value not defined";
		}
		auto &var = getVariable(identifier);
		if (var.type == Value::Undefined) {
			children.push_back(pair<string, Value>(identifier.name, value));
		}
		else {
			var = value;
		}
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

class Statement{
public:
	virtual ~Statement() {}
	virtual Value run(ObjectValue &context) {
		throw "abstract class statement called";
	}
};

inline Value Value::run(class ObjectValue& context) {
	if (type == StatementPointer) {
		return statementPtr->run(context);
	} else {
		return Value();
	}
}

inline Value& Value::operator =(const Value& value) {
	clear();
	switch (value.type) {
	case Object:
		objectPtr = value.objectPtr;
		break;
	case String:
		stringValue = new StringValue(*value.stringValue);
		break;
	case Undefined:
		break;
	default:
		throw "not implemented";
	}
	type = value.type;
	return *this;
}

inline Value Value::setValue(string value) {
	clear();
	type = String;
	stringValue = new StringValue(value);
	return *this;
}

inline string Value::toString() {
	switch (type) {
	case Undefined:
		return "Undefined";
	case String:
		return stringValue->value;
	default:
		return "not implemented";
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
