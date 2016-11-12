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


void* ObjectValue::operator new(std::size_t sz) {
//	std::cout << "custom new called for object" << '\n';
	auto ptr = ::operator new(sz);
	javascriptMemory.push_back(std::unique_ptr<ObjectValue>((ObjectValue*)ptr));
	return ptr;
}

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
//	for (auto &ptr: javascriptMemory) {
//		ptr->alive = false;
//	}

	window.alive = false; //Prepare window for algorithm
	markAllChildren(&window);

	//Remove all that has not been marked as alive
	auto f = [](const std::unique_ptr<ObjectValue> & value){
		auto alive = value->alive;
		value->alive = false; //Unmark the child
		return !alive;
	};
	javascriptMemory.erase(
			std::remove_if(
					javascriptMemory.begin(),
	                javascriptMemory.end(), f),
			javascriptMemory.end()
	);
}


int getGlobalObjectCount() {
	return javascriptMemory.size();
}

Value getGlobalContext() {
	return window;
}



