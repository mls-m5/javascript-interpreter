/*
 * jsfunctions.cpp
 *
 *  Created on: 10 okt. 2016
 *      Author: mattias
 */

#include "virtualmachine.h"
#include "nativefunction.h"
#include <functional>

ObjectValue window; //Holder for all the local variables
Value windowValue(window);
Value UndefinedValue;


void _initObject();


static class Initializer {
public:
	Initializer() {
		_initObject();

		auto console = new ObjectValue;
		console->defineVariable("log", new NativeFunction([](ObjectValue &context, Value &arguments) {
			if (auto o = arguments.getObject()) {
				cout << o->getVariable("0").toString() << endl;
			}
			else {
				cout << endl;
			}
			return UndefinedValue;
		}));
		window.setVariable("console", *console);
		window.setVariable("window", window);
	}

} initializer;
