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

class NativeFunction: public FunctionDeclaration {
public:
	string toString() override {
		return "<native function>";
	}
};

static class ConsoleLog: public NativeFunction {
	Value run(ObjectValue &context) override {
		auto arguments = context.getVariable("arguments", false);
		if (arguments.type != arguments.Undefined) {
			cout << arguments.getObject()->getVariable("0").run(context).toString() << endl;
		}
		else {
			throw "no arguments to console log";
		}
//		cout << arguments.run(context).toString() << endl;
		return UndefinedValue;
	}

} consoleLog;


static class Initializer {
public:
	Initializer() {
		auto console = new ObjectValue;
		console->defineVariable("log", consoleLog);
		window.setVariable("console", *console);
		window.setVariable("log", consoleLog); //Todo: remove this.. only for debugging
	}

} initializer;
