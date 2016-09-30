/*
 * vaule.h
 *
 *  Created on: 30 sep. 2016
 *      Author: mattias
 */

#include <string>
#include <memory>
#include "identifier.h"
using namespace std;

#pragma once
class Value {
public:
	Value() = default;
	Value (const Value & value) {
		operator=(value);
	}
	Value (Value && ) = default;

	//Todo: Improve performance by adding copy by reference
	Value (const string & value) {
		setValue(value);
	}

	Value (const char *value) {
		setValue(value);
	}

	Value (const long & value) {
		setValue(value);
	}

	Value run(class Context& context);
	//Declared after object value


	Value& operator =(const Value& value);

	~Value() {
		//
	}

	Value setValue(string value);

	Value setValue(long value) {
		type = String;
		objectValue.reset();
		intValue = value;
		return *this;
	}

	string toString();

	enum VariableType {
		Boolean,
		Null,
		Undefined,
		Number,
		String,
		Symbol,
		Object,
		Integer
	} type = Undefined;

	//All types except objects are immutable objects
	union {
		bool boolValue;
		double numberValue;
		long intValue;
	};

	unique_ptr<class ObjectValue> objectValue = nullptr;
};


class ObjectValue {
public:
	virtual ~ObjectValue() {};

	Value run(class Context& context) {
		return Value();
	}
	//Todo: Implement garbage collection

	virtual ObjectValue *copy() {return new ObjectValue(*this);};
};


class StringValue: public ObjectValue {
public:
	StringValue() = default;
	StringValue(const StringValue &) = default;
	StringValue(StringValue &&) = default;
	StringValue(string value): value(value) {
	}
	virtual ~StringValue() {};
	std::string value;

	ObjectValue *copy() override {
		return new StringValue(*this);
	}
};

class FunctionCallValue: public ObjectValue {
public:

	class Identifier identifier;
};

inline Value Value::run(class Context& context) {
	if (objectValue) {
		return objectValue->run(context);
	} else {
		return Value();
	}
}

inline Value& Value::operator =(const Value& value) {
//	cout << "assignment of value " << endl;
	switch (value.type) {
	case Object:
	case String:
		objectValue.reset(value.objectValue->copy());
		break;
	case Undefined:
		type = Undefined;
		objectValue.reset(nullptr);
		break;
	default:
		throw "not implemented";
	}
	type = value.type;
	return *this;
}

inline Value Value::setValue(string value) {
	type = String;
	objectValue.reset(new StringValue(value));
	return *this;
}

inline string Value::toString() {
	switch (type) {
	case Undefined:
		return "undefined";
	case String:
		return ((StringValue*) (objectValue.get()))->value;
	default:
		return "not implemented";
	}
}
