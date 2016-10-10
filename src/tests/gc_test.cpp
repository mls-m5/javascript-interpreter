/*
 * gc_test.cpp
 *
 *  Created on: 10 okt. 2016
 *      Author: mattias
 */


#include "unittest.h"
#include "../virtualmachine.h"

TEST_SUIT_BEGIN

TEST_CASE("new and delete") {
	auto o = new ObjectValue();

	Value value(*o);

	window.setVariable("apa", value);

	runGarbageCollection();

	auto objectsBefore = getGlobalObjectCount();

	cout << "object count before " << objectsBefore << endl;

	window.deleteVariable("apa");

	runGarbageCollection();

	auto objectsAfter = getGlobalObjectCount();
	cout << "object count after " << objectsAfter << endl;

	ASSERT_EQ(objectsAfter, objectsBefore - 1);
}

TEST_SUIT_END


