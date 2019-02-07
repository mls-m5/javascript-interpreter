/*
 * jsfunctions.cpp
 *
 *  Created on: 10 okt. 2016
 *      Author: mattias
 */

#include "virtualmachine.h"
#include "nativefunction.h"
#include <functional>

Window window; //Holder for all the local variables
Value windowValue(window);
Value UndefinedValue;


void _initObject();
void _initDate();


static class Initializer {
public:
	Initializer() {
		_initObject();
		_initDate();
		window.prototype = ObjectValue::Prototype();

		auto console = new ObjectValue;
		console->defineVariable("log", new NativeFunction([](ObjectValue &context, Value &arguments) {
			if (auto o = arguments.getObject()) {
				if (o->children.size() > 0) {
					cout << o->children[0].second.toString();
					for (int i = 1; i < o->children.size(); ++i) {
						cout << " " << o->children[i].second.toString();
					}
				}
				cout << endl;
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
