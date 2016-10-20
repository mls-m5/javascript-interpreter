/*
 * vm_test.cpp
 *
 *  Created on: 30 sep. 2016
 *      Author: mattias
 */


#include "unittest.h"
#include "../virtualmachine.h"
#include "../compiler.h"

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
	auto id1 = StatementPointer(new LiteralStatement("apa"));
	auto id2 = StatementPointer(new LiteralStatement("bepa"));

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
		string argument = "";

		Value run(ObjectValue &context) override {
			isCalled = true;
			auto arguments = context.getVariable("arguments");
			argument = arguments.getObject()->getVariable("0").toString();
			return Value();
		}
	};

	TestFunction function;
	window.setVariable("apa", function);

	auto functionCall = StatementPointer(Compiler::compile("apa(\"hej\")"));

	functionCall->run(window);

	auto newExpression = window.getVariable("apa", false);
	auto newFunction = dynamic_cast<TestFunction*>(newExpression.getStatement());

	ASSERT(newFunction, "function pointer is null");
	ASSERT(newFunction->isCalled, "Function is not called");
	ASSERT_EQ(newFunction->argument, "hej");

	window.deleteVariable("apa");

	runGarbageCollection();
}

TEST_CASE("code block") {
	auto codeBlock = StatementPointer(Compiler::compile("{var y; y = 3;}"));
	codeBlock->run(window);

	auto variable = window.getVariable("y");
	ASSERT_EQ(variable.toString(), "3");

	window.deleteVariable("x");
	runGarbageCollection();
}

TEST_CASE("binary statements") {
	auto statement = StatementPointer(Compiler::compile("4 + 5"));
	auto variable = statement->run(window);
	ASSERT_EQ(variable.toString(), "9");
}

TEST_CASE("variable declaration") {
	auto variableDeclaration = StatementPointer(Compiler::compile("var x"));
	auto assignment = StatementPointer(Compiler::compile("x = 1"));

	variableDeclaration->run(window);
	assignment->run(window);

	auto variable = window.getVariable("x");

	ASSERT_NE(variable.type, variable.Undefined);
	ASSERT_EQ(variable.toString(), "1");

	window.deleteVariable("x");
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


