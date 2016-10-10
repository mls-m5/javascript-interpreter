/*
 * virtualmachine.cpp
 *
 *  Created on: 30 sep. 2016
 *      Author: mattias
 */


#include "virtualmachine.h"
#include <vector>
#include <memory>
#include <algorithm>


std::vector<std::unique_ptr<ObjectValue>> javascriptMemory;

ObjectValue window; //Holder for all the local variables
Value UndefinedValue;


void markAllChildren(ObjectValue *object) {
	if (object->alive) {
		return; //already marked
	}
	object->alive = true;
	for (auto &ptr: object->children) {
		auto o = ptr.second.getObject();
		if (o) {
			markAllChildren(o);
		}
	}
}

void runGarbageCollection() {
	for (auto &ptr: javascriptMemory) {
		ptr->alive = false;
	}

	markAllChildren(&window);

	//Remove all that has not been marked as alive
	auto f = [](std::unique_ptr<ObjectValue> & value){return value->alive == false;};
	javascriptMemory.erase(std::remove_if(javascriptMemory.begin(),
	                              javascriptMemory.end(), f));
}


