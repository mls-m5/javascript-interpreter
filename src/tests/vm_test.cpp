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
	Context context;

	context.setVariable("bepa", "tja");
	context.setVariable("cepa", "re");
	auto value = context.getVariable("bepa");
	ASSERT_EQ(value.toString(), "tja");
}

TEST_CASE("simple assignment expression test") {
	Context context;
	Assignment assignment1("apa", VariableGetter("bepa"));

	context.setVariable("bepa", "hejsan");
	assignment1.run(context);

	auto value = context.getVariable("apa");

	ASSERT_EQ(value.toString(), "hejsan");
}

TEST_CASE("function copy") {
	class TestFunction: public FunctionDeclaration {
	public:
		~TestFunction() {

		}

		Statement *copy() const override {
			return new TestFunction(*this);
		}
	};

	TestFunction function;
	Expression ex(function);

	ASSERT(dynamic_cast<TestFunction*>(ex.statement.get()), "Function is not of the same type")

	Context context;
	context.setVariable("apa", ex);

	auto expressionFromContext = context.getVariable("apa");
	auto newFunction = dynamic_cast<TestFunction*>(expressionFromContext.getObject());

	ASSERT(newFunction, "function is not copied from the context");
}

TEST_CASE("function call") {
	class TestFunction: public FunctionDeclaration {
	public:
		~TestFunction() {}
		bool isCalled = false;

		Value run(Context &context) override {
			isCalled = true;
			return Value();
		}

		Statement *copy() const override {
			return new TestFunction(*this);
		}
	};

	TestFunction function;
	Expression ex(function);
	Context context;
	context.setVariable("apa", ex);

	FunctionCall functionCall("apa");

	functionCall.run(context); //Krashar

	auto newExpression = context.getVariable("apa");
	auto newFunction = dynamic_cast<TestFunction*>(newExpression.getObject());

//	auto functionPointer = dynamic_cast<TestFunction*>(rawPtr);
	ASSERT(newFunction, "function pointer is null");
	ASSERT(newFunction->isCalled, "Function is not called");
}


TEST_SUIT_END


