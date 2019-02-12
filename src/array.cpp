/*
 * array.cpp
 *
 *  Created on: 12 feb. 2019
 *      Author: Mattias Larsson Sköld
 */

#include "value.h"
#include "array.h"
#include "nativefunction.h"
#include <sstream>

static ObjectValue *globalArray;
static ObjectValue *arrayPrototype;

void _initArray() {
	globalArray = new ObjectValue;
	window.setVariable("Array", globalArray);
	arrayPrototype = new ObjectValue;
	globalArray->setVariable("prototype", arrayPrototype);
	arrayPrototype->setVariable("forEach", new NativeFunction([] (ObjectValue &context) {
		auto arguments = context.getArguments().getObject();
		if (!arguments || arguments->children.empty()) {
			throw RuntimeException(arguments->toString() + "is not a function");
		}
		auto function = arguments->children.front().second.getObject();
		if (!function) {
			throw RuntimeException(arguments->toString() + "is not a function");
		}

		if (auto _this = context.getThis()) {
			if (_this->isArray()) {
				auto array = static_cast<Array*>(_this);
				for (int i = 0; i < array->values.size(); ++i) {
					auto newArgs = Array({array->values[i], i, _this});
					auto closure = new Closure(&context, newArgs,_this);
					function->call(*closure);
				}
			}
		}
		return UndefinedValue;
	}));

	arrayPrototype->setVariable("map", new NativeFunction([] (ObjectValue &context) -> Value {
		auto arguments = context.getArguments().getObject();
		if (!arguments || arguments->children.empty()) {
			throw RuntimeException(arguments->toString() + "is not a function");
		}
		auto function = arguments->children.front().second.getObject();
		if (!function) {
			throw RuntimeException(arguments->toString() + "is not a function");
		}

		if (auto _this = context.getThis()) {
			if (_this->isArray()) {
				auto array = static_cast<Array*>(_this);
				auto newArray = new Array;
				newArray->values.resize(array->values.size());
				for (int i = 0; i < array->values.size(); ++i) {
					auto newArgs = Array({array->values[i], i, _this});
					auto closure = new Closure(&context, newArgs,_this);
					newArray->values[i] = function->call(*closure).getValue();
				}
				return newArray;
			}
		}
		return UndefinedValue;
	}));

	arrayPrototype->setVariable("toString", new NativeFunction([] (ObjectValue &context) -> Value {
		return context.getThis()->toString();
	}));
}

Array::Array(vector<Value> && values) :
		ObjectValue(arrayPrototype),
		values(move(values)) {
}

Array::Array(vector<Value>& values) :
		ObjectValue(arrayPrototype),
		values(values) {
}

Array::Array():
	ObjectValue(arrayPrototype){
}

Array::~Array() {
}

Value Array::getVariable(Value identifier, bool allowReference) {
	if (!identifier.isNaN()) {
		auto i = identifier.toInteger();
		if (i >= 0 && i < values.size()) {
			return at(i, allowReference);
		}
	}
	else if (identifier.type == identifier.String && identifier.stringValue && identifier.stringValue->value == "length") {
		//This is a temporary solution until i implement property methods
		return Value((long)values.size());
	}
	return ObjectValue::getVariable(identifier, allowReference);
}

Value Array::getOrDefineVariable(Value identifier, bool allowReference) {
	if (!identifier.isNaN()) {
		auto i = identifier.toInteger();
		if (i >= 0) {
			if (i < values.size()) {
				return at(i, allowReference);
			}
			return defineVariable(identifier);
		}
	}

	return ObjectValue::getOrDefineVariable(identifier, allowReference);
}

Value Array::setVariable(Value identifier, Value value, bool allowUndefined) {
	if (!identifier.isNaN()) {
		auto i = identifier.toInteger();
		if (i >= 0 && i < values.size()) {
			values[i] = value.getValue();
			return &values[i];
		}
	}

	return ObjectValue::setVariable(identifier, value, allowUndefined);
}

Value Array::defineVariable(Value identifier, Value value) {
	//For ObjectValues this function throws an error when a variable is defined
	//But for an array there is no case when that should be illegal
	//Therefore this function does not throw an array when a variable is already defined
	if (!identifier.isNaN()) {
		auto i = identifier.toInteger();
		if (i >= 0) {
			if (i < values.size()) {
				auto &val = values[i];
				val = value.getValue();
				return &val;
			}
			else {
				values.resize(i + 1);
				values.back() = value.getValue();
				return &values.back();
			}
		}
	}

	return ObjectValue::defineVariable(identifier, value);
}

Value Array::deleteVariable(Value identifier) {
	if (!identifier.isNaN()) {
		auto i = identifier.toInteger();
		if (i >= 0 && i < values.size()) {
			(*this)[i] = UndefinedValue;
			return true;
		}
	}

	return ObjectValue::deleteVariable(identifier);
}

void Array::mark() {
	for (auto& value : values) {
		if (auto o = value.getObject()) {
			o->mark();
		}
	}
	ObjectValue::mark();
}

string Array::toString() {
	ostringstream ss;
	ss << "[ ";

	for (auto &value: values) {
		ss << value.toString() << ", ";
	}

	for (auto &child: children) {
		ss << child.first << ": " << child.second.toString() << ", ";
	}
	ss << "]";
	return ss.str();
}

