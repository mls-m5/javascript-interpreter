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

	window.deleteVariable("bepa");
	window.deleteVariable("cepa");
}

TEST_CASE("simple assignment expression test") {
	auto assignment1 = Compiler::compile("apa = bepa");

	window.defineVariable("bepa", "hejsan");
	window.defineVariable("apa", "dasan");
	assignment1->run(window);

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

	auto functionCall = Compiler::compile("apa(\"hej\")");

	functionCall->run(window);

	auto newExpression = window.getVariable("apa", false);
	auto newFunction = dynamic_cast<TestFunction*>(newExpression.getStatement());

	ASSERT(newFunction, "function pointer is null");
	ASSERT(newFunction->isCalled, "Function is not called");
	ASSERT_EQ(newFunction->argument, "hej");

	window.deleteVariable("apa");

	runGarbageCollection();
}

TEST_CASE("function declaration") {
	try {
		auto statement = Compiler::compile("var x = 'apa'; function apa() {x = 'bepa'}");
		auto callStatement = Compiler::compile("apa()");
		statement->run(window);

		auto variable = window.getVariable("x");
		ASSERT_EQ(variable.toString(), "apa");

		callStatement->run(window);
		variable = window.getVariable("x");
		ASSERT_EQ(variable.toString(), "bepa");

		runGarbageCollection();

	} catch (CompilationException &e) {
		cout << e.what << ":" << e.token << endl;
		ASSERT(0, e.what);
	}

	window.deleteVariable("apa");
	window.deleteVariable("x");

}

TEST_CASE("code block") {
	auto codeBlock = Compiler::compile("{var y; y = 3;}");
	codeBlock->run(window);

	auto variable = window.getVariable("y");
	ASSERT_EQ(variable.toString(), "3");

	window.deleteVariable("y");
	runGarbageCollection();
}

TEST_CASE("binary statements") {
	auto statement = Compiler::compile("4 + 5");
	auto variable = statement->run(window);
	ASSERT_EQ(variable.toString(), "9");
}

TEST_CASE("aritmetic statements") {
	window.defineVariable("x", 2);
	window.defineVariable("y", 3);
	window.defineVariable("z", 4);
	auto statement = Compiler::compile("x + y * z");

	auto variable = statement->run(window);
	ASSERT_EQ(variable.toString(), "14");
	window.deleteVariable("x");
	window.deleteVariable("y");
	window.deleteVariable("z");
	runGarbageCollection();
}


TEST_CASE("variable declaration") {
	auto variableDeclaration = Compiler::compile("var x");
	auto assignment = Compiler::compile("x = 1");

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


