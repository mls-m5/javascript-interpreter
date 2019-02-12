/*
 * array.h
 *
 *  Created on: 12 feb. 2019
 *      Author: Mattias Larsson Sköld
 */

#pragma once

#include "value.h"

class Array: public ObjectValue {
public:
	Array();
	Array(vector<Value>& values);
	Array(vector<Value> && values);
	virtual ~Array();

	Value getVariable(Value identifier, bool allowReference = true) override;

	Value getOrDefineVariable(Value identifier, bool allowReference = true)
			override;

	Value setVariable(Value identifier, Value value,
			bool allowUndefined = false) override;

	Value defineVariable(Value name, Value value = Value()) override;

	virtual Value deleteVariable(Value identifier);

	bool isArray() override {
		return true;
	}

	void mark() override;

	vector <Value> values;

	//Returns a reference value
	inline Value operator [] (long i) {return &values[i];}
	inline Value at(long i, bool allowReference = true) {
		if (allowReference) {
			return Value(&(values[i]));
		}
		else {
			return Value(values[i]);
		}
	}

	string toString() override ;
};
