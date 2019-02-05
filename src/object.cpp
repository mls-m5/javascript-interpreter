/*
 * object.cpp
 *
 *  Created on: 5 feb. 2019
 *      Author: Mattias Larsson Sköld
 */

#include "value.h"
#include "nativefunction.h"

static ObjectValue *object;
static ObjectValue *rootPrototype;

ObjectValue *ObjectValue::Root() {
	return object;
}

ObjectValue *ObjectValue::Prototype() {
	return rootPrototype;
}

void _initObject() {
	rootPrototype = new ObjectValue;
	rootPrototype->prototype = nullptr;

	object = new ObjectValue;
	// Note that the object has the "prototype" member set to 0 but
	// the "prototype" _variable_ set to rootPrototype
	object->prototype = nullptr;

	window.setVariable("Object", ObjectValue::Root());
	object->setVariable("prototype", rootPrototype);

	rootPrototype->setVariable("toString", new NativeFunction([](ObjectValue &context, Value &arguments) {
		return "<string value representation of object, but which object?>";
	}));

}

