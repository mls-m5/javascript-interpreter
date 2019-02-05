/*
 * nativefunction.h
 *
 *  Created on: 5 feb. 2019
 *      Author: Mattias Larsson Sköld
 */

#pragma once

#include <functional>
#include "value.h"

class NativeFunction: public Function {
public:
	std::function<Value (ObjectValue&, Value&)> functionPointer;
	NativeFunction(): Function(window) {}
	NativeFunction(decltype (functionPointer) functionPointer):
		functionPointer(functionPointer), Function(window) {};

	string toString() override {
		return "<native function>";
	}

	Value call(ObjectValue &context, Value &arguments) override {
		if (functionPointer) {
			return functionPointer(context, arguments);
		}
		else {
			throw "function not implemented";
		}
	}
};



