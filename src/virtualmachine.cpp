/*
 * virtualmachine.cpp
 *
 *  Created on: 30 sep. 2016
 *      Author: mattias
 */


#include "virtualmachine.h"
#include <vector>
#include <memory>


typedef std::shared_ptr<ObjectValue> ObjectValuePtr;


std::vector<ObjectValuePtr> javascriptMemory;


void markAllChildren(ObjectValue *object) {
//	for (auto &ptr: object->)
}

void runGarbageCollection() {
	for (auto &ptr: javascriptMemory) {
		ptr->alive = false;
	}

	for (auto &ptr: javascriptMemory) {
		if (ptr->alive == false) {

		}
	}
}


