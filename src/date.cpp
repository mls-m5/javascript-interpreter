/*
 * console.cpp
 *
 *  Created on: 7 feb. 2019
 *      Author: Mattias Larsson Sköld
 */

#include "value.h"
#include "nativefunction.h"

#include <ctime>
#include <chrono>

static ObjectValue *date;

using namespace std::chrono;


static string getTimeString(long ms) {
	time_t rawtime = ms / 1000;
	struct tm * timeinfo;
	char buffer[100];

//	time (&rawtime);
	timeinfo = localtime(&rawtime);

	//Should be this format
	//'Thu Feb 07 2019 10:51:38 GMT+0100 (GMT+01:00)'
	strftime(buffer,sizeof(buffer),"%a %b %d %Y %H:%M:%S GMT%z",timeinfo);
	std::string str(buffer);

	return str;
}

static long getTimeMillis() {
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void _initDate() {
	struct Date: public NativeFunction {
		Date():
			NativeFunction([this](ObjectValue &context, Value &arguments) {
				auto millis = getTimeMillis();
				if (auto _this = context.thisPointer()) {
					_this->prototype == this->prototype;
					context.setVariable("value", getTimeMillis());
				}
				return Value(getTimeString(millis));
			}){}
	};

	date = new Date();

	date->setVariable("now", new NativeFunction([](ObjectValue &context, Value &arguments) {
		return getTimeMillis();
	}));

	ObjectValue *prototype = new ObjectValue;
	prototype->setVariable("getTime", new NativeFunction([](ObjectValue &context, Value &arguments) {
		if (auto _this = context.thisPointer()) {
			auto millis = context.thisPointer()->getVariable("__internalTime__").toNumber();
			return getTimeString(millis);
		}
		else {
			throw RuntimeException("cannot call Date.getTime without object");
		}
	}));


	window.setVariable("Date", date);
}




