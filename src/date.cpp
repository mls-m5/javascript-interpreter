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

static const string internalValueName = "__internalTime__";

static long getTimeMillis() {
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void _initDate() {
	struct Date: public NativeFunction {
		Date():
			NativeFunction([this](ObjectValue &context) {
				auto millis = getTimeMillis();
				if (context.getNewTarget()) {
					if (auto _this = context.getThis()) {
						_this->setVariable(internalValueName, getTimeMillis());
					}
				}
				return Value(getTimeString(millis));
			}){}
	};

	date = new Date();

	date->setVariable("now", new NativeFunction([](ObjectValue &context) {
		return getTimeMillis();
	}));

	auto prototype = date->getVariable("prototype").getObject();

	prototype->setVariable("getTime", new NativeFunction([](ObjectValue &context) {
		if (auto _this = context.getThis()) {
			auto millis = context.getThis()->getVariable(internalValueName).toNumber();
			return (long)millis;
		}
		else {
			throw RuntimeException("cannot call Date.getTime without object");
		}
	}));

	prototype->setVariable("toString", new NativeFunction([](ObjectValue &context) {
		if (auto _this = context.getThis()) {
			auto millis = _this->getVariable(internalValueName).toNumber();
			return getTimeString(millis);
		}
		else {
			throw RuntimeException("cannot call Date.toString without object");
		}
	}));

	window.setVariable("Date", date);
}




