/*
 * function.cpp
 *
 *  Created on: 7 feb. 2019
 *      Author: Mattias Larsson Sköld
 */


#include "value.h"
#include "nativefunction.h"

static ObjectValue *functionRoot;

ObjectValue *Function::Root() {
	return functionRoot;
}

void _initFunction() {
	functionRoot = new ObjectValue;

	window.setVariable("Function", functionRoot);

	auto prototype = new ObjectValue;

	functionRoot->setVariable("prototype", prototype);

	prototype->setVariable("constructor", new NativeFunction([](ObjectValue &context) -> Value {

	}));
}



