/*
 * jsfunctions.cpp
 *
 *  Created on: 10 okt. 2016
 *      Author: mattias
 */

#include "virtualmachine.h"

ObjectValue window; //Holder for all the local variables
Value windowValue(window);
Value UndefinedValue;

class NativeFunction: public Function {
public:
	NativeFunction(): Function(window) {}

	string toString() override {
		return "<native function>";
	}
};


static class ConsoleLog: public NativeFunction {
	virtual Value call(ObjectValue &context, Value &arguments) {
		if (auto o = arguments.getObject()) {
			cout << o->getVariable("0").toString() << endl;
		}
		else {
			cout << endl;
		}
		return UndefinedValue;
	}

} consoleLog;


static class Initializer {
public:
	Initializer() {
		auto console = new ObjectValue;
		console->defineVariable("log", consoleLog);
		window.setVariable("console", *console);
		window.setVariable("window", window);
	}

} initializer;
