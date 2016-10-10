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
		case Identifier:
			identifierPtr = value.identifierPtr;
			break;
		default:
			throw "not implemented error in Value move constructor";
		}
	}

	Value (Value *identifier) {
		//Pass the value by reference
		type = Identifier;
		if (identifier->type == Identifier) {
			identifierPtr = identifier->identifierPtr;
		}
		else if(identifier->type == Undefined) {
			type = Undefined;
		}
		else {
			identifierPtr = identifier;
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

	Value call(ObjectValue& context, class Value& arguments);

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
		Identifier, //Only used for return value
		StatementPointer,
	} type = Undefined;

	//All types except objects are immutable objects
	union {
		bool boolValue;
		double numberValue;
		class ObjectValue* objectPtr;
		class StringValue* stringValue;
		class Statement* statementPtr; //Points to a placie in the abstract source tree
		Value *identifierPtr;
		long intValue;
	};
};

extern Value UndefinedValue;
extern class ObjectValue window;
extern Value windowValue;

class ObjectValue{
public:
	virtual ~ObjectValue() {}

	Value run(class Context& context) {
		return Value();
	}

	// custom new operator for the object value
	static void* operator new(std::size_t sz);

	Value getVariable(Identifier identifier) {
		if (this == &window) {
			if (identifier.name == "window") {
				return windowValue;
			}
		}
		for (auto &it: children) {
			if (it.first == identifier.name) {
				return Value(&it.second); //Returns identifier
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

	vector<pair<string, Value>>::iterator getVariableIterator(Identifier identifier) {
		for (auto it = children.begin(); it != children.end(); ++it) {
			if ((*it).first == identifier.name) {
				return it;
			}
		}

		return children.end();
	}

	void setVariable(Identifier identifier, Value value) {
		if (value.type == Value::Undefined) {
			throw "value not defined";
		}
		auto it = getVariableIterator(identifier);
		if (it == children.end()) {
			children.push_back(pair<string, Value>(identifier.name, value));
		}
		else {
			it->second = value;
		}
	}

	void deleteVariable(Identifier identifier) {
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
		localContext.setVariable("arguments", expression);
		return this->run(localContext);
	}
};

inline Value Value::run(class ObjectValue& context) {
	if (type == StatementPointer) {
		return statementPtr->run(context);
	} else {
		return *this;
	}
}

inline Value Value::operator =(const Value& value) {
	if (type == Identifier) {
		*identifierPtr = value;
	}
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
	default:
		throw "not implemented";
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
		return statementPtr->call(context, arguments);
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

