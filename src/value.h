/*
 * vaule.h
 *
 *  Created on: 30 sep. 2016
 *      Author: mattias
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <limits>
#include "token.h"
#include "exceptions.h"
#include <cmath>
using namespace std;

static constexpr const double NaN = numeric_limits<double>::quiet_NaN();
static constexpr const double Infinity = numeric_limits<double>::infinity();
class Function;

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
		case Boolean:
			boolValue = value.boolValue;
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
		setValue((string)value);
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

	Value (bool value) {
		setValue(value);
	}

	Value (class ObjectValue &value) {
		setValue(value);
	}

	Value (class ObjectValue *value) {
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

	Value setValue(bool value) {
		clear();
		type = Boolean;
		boolValue = value;
		return *this;
	}

	Value setValue(class ObjectValue *value) {
		return setValue(*value);
	}

	Value setValue(class ObjectValue &value);


	operator bool();

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


#define VALUE_NUMERIC_OPERATOR(op) \
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

	VALUE_NUMERIC_OPERATOR(+)
	VALUE_NUMERIC_OPERATOR(-)
	VALUE_NUMERIC_OPERATOR(*)
	VALUE_NUMERIC_OPERATOR(/)
	VALUE_NUMERIC_OPERATOR(<)
	VALUE_NUMERIC_OPERATOR(>)
	VALUE_NUMERIC_OPERATOR(>=)
	VALUE_NUMERIC_OPERATOR(<=)

#define VALUE_EQUALITY_OPERATOR(op) \
	Value operator op(Value &v) { \
		auto value = v.getValue(); \
		switch(type) { \
		case Number: \
			return numberValue op v.toNumber(); \
		case Integer: \
			if (v.type == Integer) { \
				return intValue op v.intValue; \
			} \
			else { \
				return intValue op v.toNumber(); \
			} \
		case Object: \
			if (v.type == Object) { \
				return objectPtr op v.objectPtr; \
			} \
			else { \
				return false; \
			} \
		case Reference: \
			return *referencePtr op value; \
		} \
		return false; \
	}

	VALUE_EQUALITY_OPERATOR(==)
	VALUE_EQUALITY_OPERATOR(!=)

	Value operator += (Value &v) {
		if (type != Reference) {
			throw RuntimeException("type is not reference in assignment");
		}
		Value value = v.getValue();
		return (*referencePtr = *this + v);
	}


#define VALUE_PREFIX_OPERATOR(op) \
	Value operator op() { \
		switch(type) { \
		case Integer: \
			return op intValue; \
			return *this; \
		case Number: \
			op numberValue; \
			return *this; \
		case Reference: \
			op (*referencePtr); \
			return *this; \
		} \
		return op (*this = toNumber()); \
	}

	VALUE_PREFIX_OPERATOR(++)
	VALUE_PREFIX_OPERATOR(--)

	Value propertyAccessor(Value& v);

	Value operator!() {
		return !bool(*this);
	}

	Value unaryPlus() {
		return toNumber();
	}

	Value unaryMinus() {
		return -toNumber();
	}

	//Convert a value to a number
	double toNumber();

	Value call(ObjectValue& context, class Value arguments, ObjectValue *thisPointer);

	string toString();

	void clear();

	void setReturnFlag() {
		type = (VariableType) (type | ReturnFlag);
	}

	void resetReturnFlag() {
		type = (VariableType) (type & ~ReturnFlag); //~Flips all bits
	}

	bool isReturn() {
		return type & ReturnFlag;
	}

	enum VariableType {
		Boolean,
		Null,
		Undefined,
		Integer,
		Number,
		String,
		Symbol,
		Object,
		Reference, //Only used for return value, not for storage
		ReturnFlag = 1 << 15, //This is special, is set if value is sent through a return-statement
		NotReturnFlag = ~ReturnFlag,
	} type = Undefined;

	//All types except objects are immutable objects
	union {
		bool boolValue;
		double numberValue;
		class ObjectValue* objectPtr;
		class StringValue* stringValue;
		Value *referencePtr;
		long intValue;
	};
};



extern Value UndefinedValue;



// Object value:
// A special type of value that is handling objects
// The class is garbage collected and has a custom new operator
// That means, if a ObjectValue is defined without new keyword,
// it is not affected by the garbage collector, but if the new keyword is used
// the object is automatically handled. Delete should therefore _never_ be used
// on a ObjectValue variable.
class ObjectValue {
public:
	virtual ~ObjectValue() {}

	virtual Value run(ObjectValue& context) {
		return Value();
	}

	// custom new operator for the object value
	static void* operator new(std::size_t sz);

	// Mark as active by garbage collector
	virtual void mark() {
		if (alive) {
			return; //already marked
		}
		alive = true;
		for (auto &ptr: children) {
			auto o = ptr.second.getObject();
			if (o) {
				o->mark();
			}
		}
		if (prototype) {
			prototype->mark();
		}
	}

	static ObjectValue *Root();
	//The objectprototype used by all other objects,
	// also accessible through getGetvariable("prototype")
	static ObjectValue *Prototype();

	virtual ObjectValue *thisPointer() { return nullptr; };

	virtual Value call(ObjectValue& context, class Value& arguments, ObjectValue *thisPointer) {
		throw RuntimeException("object is not a function");
	}

	//Old function
	virtual Value getVariableStr(string identifier, bool allowReference = true) final {
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

		if (this->prototype) {
			//Continue search
			return this->prototype->getVariableStr(identifier);
		}
		else {
			return UndefinedValue; //Undefined
		}
	}

	//This function is the one that should be used in the future
	virtual Value getVariable(Value identifier, bool allowReference = true) {
		return getVariableStr(identifier.toString(), allowReference);
	}


	virtual Value getOrDefineVariable(Value identifier, bool allowReference = true) {
		auto stringIdentifier = identifier.toString();
		for (auto &it: children) {
			if (it.first == stringIdentifier) {
				if (allowReference) {
					return Value(&it.second); //Returns identifier
				}
				else {
					return it.second;
				}
			}
		}

		//Do not traverse prototypes if it is a definition
		return defineVariable(identifier);
	}


	vector<pair<string, Value>>::iterator getVariableIterator(Value identifier) {
		auto stringIdentifier = identifier.toString();

		for (auto it = children.begin(); it != children.end(); ++it) {
			if ((*it).first == stringIdentifier) {
				return it;
			}
		}

		return children.end();
	}

	Value setVariable(string identifier, Value value, bool allowUndefined = false) {
		if (value.type == Value::Undefined && !allowUndefined) {
			throw RuntimeException("value not defined when setting variable");
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
	virtual Value defineVariable(Value name, Value value = Value()) {
		auto it = getVariableIterator(name);
		if (it == children.end()) {
			children.push_back(pair<string, Value>(name.toString(), value));
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
			if (it.second.type == Value::Object && it.second.objectPtr == this) {
				ret += (it.first + ": [circular reference], ");
			}
			else {
				ret += (it.first + ": " + it.second.toString() + ", ");
			}
		}

		ret += "}";

		return ret;
	}

	vector<pair<string, Value>> children;
	// prototype represents the x.__proto__ value
	// not to be confused with the x.prototype value that
	// is saved as a child of the object
	ObjectValue *prototype = Prototype();
	bool alive = true;
};





class Window: public ObjectValue{
	ObjectValue *thisPointer() override {
		return this;
	}
};

extern Window window;
extern Value windowValue;





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

class Closure: public ObjectValue {
public:
	ObjectValue *_this;
	ObjectValue *parent;
	Closure(ObjectValue *parent, ObjectValue *_this):
		_this(_this),
		parent(parent) {
	}

	~Closure() {}

	void mark() override {
		ObjectValue::mark();
		if (_this) {
			_this->mark();
		}
		if (parent) {
			parent->mark();
		}
	}

	Value getVariable(Value identifier, bool allowReference = true) override {
		auto var = ObjectValue::getVariable(identifier, allowReference);

		if (var.type != var.Undefined) {
			return var;
		}
		else {
			if (parent) {
				return parent->getVariable(identifier, allowReference);
			}
			else {
				return UndefinedValue;
			}
		}
	}
	ObjectValue *thisPointer() override {
		return _this;
	}
};

class Function: public ObjectValue {
public:
	ObjectValue *definitionContext;
	shared_ptr<vector<Token>> argumentNames;
	StatementPointer block;
	Function(ObjectValue &context, StatementPointer block, shared_ptr<vector<Token>> arguments): definitionContext(&context), block(block), argumentNames(arguments){
	}
	Function(ObjectValue &context): definitionContext(&context), block(nullptr) {}

	virtual ~Function() {}

	void mark() override {
		this->ObjectValue::mark();
		definitionContext->mark();
	}

	static void setActive(ObjectValue *o); //Implemented in virtualmachine.cpp

	struct ActivationGuard {
		Closure *closure;
		Function *self;
		ActivationGuard(Closure *closure, Function *self): closure(closure), self(self) {
			setActive(closure);
		}

		~ActivationGuard() {
			setActive(self);
		}
	};

	Value call(ObjectValue &context, Value &arguments, ObjectValue *thisPointer) override {
		auto closure = new Closure(definitionContext, thisPointer);
		closure->defineVariable("arguments", arguments);

		if (auto o = arguments.getObject()) {
			for (int i = 0; i < argumentNames->size(); ++i) {
				Value index(i);
				auto argument = o->getVariable(index);
				closure->setVariable(argumentNames->at(i), argument, true);
			}
		}

		ActivationGuard(closure, this); //Activates this function
		auto ret = block->run(*closure);
		ret.resetReturnFlag();
		return ret;
	}

	string toString() override {
		stringstream ss;
		ss << "function ( ";
		bool first = true;
		for (const auto &name: *argumentNames) {
			if (!first) {
				ss << ", ";
			}
			first = false;
			ss << name;
		}
		ss << " ) { ... }";

		return ss.str();
	}
};

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
	if (type == Object) {
		return objectPtr->run(context);
	} else if (type == Reference) {
		return referencePtr->run(context);
	} else {
		return *this;
	}
}

inline Value Value::operator =(const Value& value) {
	clear();
	switch (value.type & ~ReturnFlag) {
	case Object:
		objectPtr = value.objectPtr;
		break;
	case String:
		stringValue = new StringValue(*value.stringValue);
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
	case Boolean:
		boolValue = value.boolValue;
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

inline Value::operator bool() {
	if (type == Boolean) {
		return boolValue;
	} else if (type == Number) {
		return numberValue != 0;
	} else if (type == Integer) {
		return intValue != 0;
	} else if (type == String) {
		return !stringValue->value.empty();
	} else if (type == Object) {
		return objectPtr;
	} else {
		return false;
	}
}

inline Value Value::propertyAccessor(Value& v) {
	if (type == Object) {
		return objectPtr->getVariable(v);
	} else if (type == Reference) {
		return referencePtr->propertyAccessor(v);
	} else {
		throw "try to access property of non object";
	}
}

inline double Value::toNumber() {
	switch (type) {
	case Reference:
		return referencePtr->toNumber();
	case Integer:
		return intValue;
	case Number:
		return numberValue;
	case String: {
		auto& str = stringValue->value;
		if (str.empty()) {
			return NaN;
		}
		int numPeriods = 0;
		for (auto c : str) {
			if (!isdigit(c) && c != '.') {
				return NaN;
			} else if (c == '.') {
				++numPeriods;
				if (numPeriods > 1) {
					return NaN;
				}
			}
		}
		istringstream ss(str);
		double value;
		ss >> value;
		return value;
	}
	}
	return NaN;
}

inline Value Value::call(ObjectValue& context, class Value arguments, ObjectValue *thisPointer) {
	if (type == Object) {
		if (!objectPtr) {
			throw "trying to call null statement";
		}
		return objectPtr->call(context, arguments, thisPointer);
	} else if (type == Reference) {
		return referencePtr->call(context, arguments, thisPointer);
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
	case Reference:
		return referencePtr->toString();
	case Boolean:
		if (boolValue) {
			return "true";
		}
		else {
			return "false";
		}
	case Integer:
	{
		ostringstream ss;
		ss << intValue;
		return ss.str();
	}
	case Number:
	{
		if (numberValue != numberValue) {
			return "NaN";
		}
		else if (isinf(numberValue)) {
			return "Infinity";
		}
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

