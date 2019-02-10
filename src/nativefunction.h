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
	std::function<Value (ObjectValue&)> functionPointer;
	NativeFunction(): Function(window) {}
	NativeFunction(decltype (functionPointer) functionPointer):
		functionPointer(functionPointer), Function(window) {};

	string toString() override {
		return "<native function>";
	}

	Value call(ObjectValue &context) override {
		if (functionPointer) {
//			ActivationGuard(context, this); //Activates this function
			return functionPointer(context).resetReturnFlag();
		}
		else {
			throw "function not implemented";
		}
	}
};



