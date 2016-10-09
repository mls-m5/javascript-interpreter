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

class ObjectValueBase {
public:
	virtual ~ObjectValueBase() {};

	//Todo: Implement garbage collection
};

#pragma once
class Value: ObjectValueBase {
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

	Value (const class ObjectValue &value) {
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

	Value setValue(const class ObjectValue &value);

	ObjectValue *getObject() {
		return (class ObjectValue *)objectValue.get();
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
		Integer,
//		Reference, //For value transfere only
	} type = Undefined;

	//All types except objects are immutable objects
	union {
		bool boolValue;
		double numberValue;
		long intValue;
	};

	unique_ptr<class ObjectValueBase> objectValue = nullptr;
};

class ObjectValue: public ObjectValueBase {
public:
	~ObjectValue() {}

	Value run(class Context& context) {
		return Value();
	}

	virtual ObjectValue *copy() const {
//		return new ObjectValue(*this);
		throw "trying to copy pure object value";
	};

	bool alive = true;
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

	ObjectValue *copy() const override {
		return new StringValue(*this);
	}
};

class FunctionCallValue: public ObjectValue {
public:

	class Identifier identifier;
};

inline Value Value::run(class Context& context) {
	if (objectValue) {
		return ((ObjectValue*)objectValue.get())->run(context);
	} else {
		return Value();
	}
}

inline Value& Value::operator =(const Value& value) {
//	cout << "assignment of value " << endl;
	switch (value.type) {
	case Object:
	case String:
		objectValue.reset(((ObjectValue*)value.objectValue.get())->copy());
		break;
	case Undefined:
		type = Undefined;
		objectValue.reset(nullptr);
		break;
//	case Reference:
//		type = Reference;
//		objectValue.reset(value.objectValue);
//		break;
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

inline Value Value::setValue(const ObjectValue &value) {
	type = Object;
	objectValue.reset(value.copy());
	return *this;
}
