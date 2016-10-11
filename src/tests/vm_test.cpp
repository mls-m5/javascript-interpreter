/*
 * vm_test.cpp
 *
 *  Created on: 30 sep. 2016
 *      Author: mattias
 */


#include "unittest.h"
#include "../virtualmachine.h"

TEST_SUIT_BEGIN


TEST_CASE("simple assignment test") {
	Value value, value2;
	value.setValue("hej");
	value2 = value;
	ASSERT_EQ(value.toString(), value2.toString());

}

TEST_CASE("simple context test") {
	ObjectValue context;

	context.setVariable("bepa", "tja");
	context.setVariable("cepa", "re");
	auto value = context.getVariable("bepa");
	ASSERT_EQ(value.toString(), "tja");
}

TEST_CASE("simple assignment expression test") {
	Identifier id1("apa");
	Identifier id2("bepa");
	Assignment assignment1(id1, id2);

	window.defineVariable("bepa", "hejsan");
	window.defineVariable("apa", "dasan");
	assignment1.run(window);

	auto value = window.getVariable("apa");

	ASSERT_EQ(value.toString(), "hejsan");

	window.deleteVariable("bepa");
	window.deleteVariable("apa");

	runGarbageCollection();
}

TEST_CASE("delete statement") {
	window.setVariable("apa", "xxx");

	DeleteStatement deleteStatement("apa");

	deleteStatement.run(window);

	ASSERT_EQ(window.getVariable("apa").getValue().type, Value::Undefined);

	runGarbageCollection();
}

TEST_CASE("property accessor") {
	ObjectValue object;
	object.defineVariable("x", "heej");

	window.setVariable("apa", object);

	Identifier id("apa");

	PropertyAccessor accessor(id, "x");

	auto value = accessor.run(window);
	ASSERT_EQ(value.toString(), "heej");

	window.deleteVariable("apa");
	runGarbageCollection();

}


TEST_CASE("function call") {
	class TestFunction: public FunctionDeclaration {
	public:
		~TestFunction() {}
		bool isCalled = false;

		Value run(ObjectValue &context) override {
			isCalled = true;
			return Value();
		}
	};

	TestFunction function;
	window.setVariable("apa", function);

	Identifier id("apa");

	FunctionCall functionCall(id);

	functionCall.run(window);

	auto newExpression = window.getVariable("apa", false);
	auto newFunction = dynamic_cast<TestFunction*>(newExpression.getStatement());

	ASSERT(newFunction, "function pointer is null");
	ASSERT(newFunction->isCalled, "Function is not called");

	window.deleteVariable("apa");

	runGarbageCollection();
}

TEST_CASE("identifier test") {
	Identifier identifier("apa");

	window.defineVariable("apa", "hej");

	auto ret = identifier.run(window);

	ASSERT_EQ(ret.toString(), "hej");



	window.deleteVariable("apa");
	runGarbageCollection();
}


TEST_SUIT_END


