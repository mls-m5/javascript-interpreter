/*
 * array.cpp
 *
 *  Created on: 12 feb. 2019
 *      Author: Mattias Larsson Sköld
 */

#include "value.h"
#include "array.h"
#include <sstream>


Array::~Array() {
}

Value Array::getVariable(Value identifier, bool allowReference) {
	if (!identifier.isNaN()) {
		auto i = identifier.toInteger();
		if (i > 0 && i < values.size()) {
			return at(i, allowReference);
		}
	}
	return ObjectValue::getVariable(identifier, allowReference);
}

Value Array::getOrDefineVariable(Value identifier, bool allowReference) {
	if (!identifier.isNaN()) {
		auto i = identifier.toInteger();
		if (i > 0) {
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
		if (i > 0 && i < values.size()) {
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
		if (i > 0) {
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
		if (i > 0 && i < values.size()) {
			(*this)[i] = UndefinedValue;
			return true;
		}
	}

	return ObjectValue::deleteVariable(identifier);
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

