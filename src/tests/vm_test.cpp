/*
 * vm_test.cpp
 *
 *  Created on: 30 sep. 2016
 *      Author: mattias
 */


#include "unittest.h"
#include "../virtualmachine.h"
#include "../compiler.h"

//A class that removes the variables when out of scope
class VariableGuard {
public:
	VariableGuard(string variable): variables({variable}) {}
	VariableGuard(const std::initializer_list<string> variables): variables(variables) {}

	~VariableGuard() {
		for (auto it: variables) {
			window.deleteVariable(it);
		}
		runGarbageCollection();
	}

	vector<string> variables;
};

TEST_SUIT_BEGIN


TEST_CASE("assignnment tests") {
	{
		VariableGuard g("x");
		auto statement = Compiler::compile("var x; x = 2");
		statement->run(window);
		auto variable = window.getVariable("x");
		ASSERT_EQ(variable.toString(), "2");
	}
	{
		VariableGuard g("x");
		auto statement = Compiler::compile("var x = 1; x += 2");
		statement->run(window);
		auto variable = window.getVariable("x");
		ASSERT_EQ(variable.toString(), "3");
	}
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
	VariableGuard g("apa");
	ObjectValue object;
	object.defineVariable("x", "heej");

	window.setVariable("apa", object);

	//PropertyAccessor accessor("apa", "x");
	auto value = Compiler::run("apa.x", window);

	ASSERT_EQ(value.toString(), "heej");

	auto value2 = Compiler::run("apa['x']", window);

	ASSERT_EQ(value2.toString(), "heej");
}


TEST_CASE("function call") {
	class TestFunction: public Function {
	public:
		TestFunction() : Function(window){}
		~TestFunction() {}
		bool isCalled = false;
		string argument = "";


		virtual Value call(ObjectValue &context, Value &arguments) override {
			isCalled = true;
			//auto arguments = context.getVariable("arguments");
			argument = arguments.getObject()->getVariable("0").toString();
			return Value();
		}
	};

	TestFunction function;
	window.setVariable("apa", function);

	auto functionCall = Compiler::compile("apa(\"hej\")");

	functionCall->run(window);

	auto newExpression = window.getVariable("apa", false);
	auto newFunction = dynamic_cast<TestFunction*>(newExpression.getObject());

	ASSERT(newFunction, "function pointer is null");
	ASSERT(newFunction->isCalled, "Function is not called");
	ASSERT_EQ(newFunction->argument, "hej");

	window.deleteVariable("apa");

	runGarbageCollection();
}

TEST_CASE("if-statement") {
	{
		auto statement = Compiler::compile("var x = 1; if (1) {x = 2} if (0) {x = 3}");

		statement->run(window);

		auto variable = window.getVariable("x");

		ASSERT_EQ(variable.toString(), "2");

		window.deleteVariable("x");
	}
	{
		auto statement = Compiler::compile("var x = 1; if (0) {x = 2} else {x = 3}");

		statement->run(window);

		auto variable = window.getVariable("x");

		ASSERT_EQ(variable.toString(), "3");

		window.deleteVariable("x");

	}

	{
		auto statement = Compiler::compile("var x = 1; if (0) {x = 2} else if(1) {x = 5} else {x = 4}");

		statement->run(window);

		auto variable = window.getVariable("x");

		ASSERT_EQ(variable.toString(), "5");

		window.deleteVariable("x");

	}

	runGarbageCollection();
}

TEST_CASE("unary statements") {
	auto statement = Compiler::compile("var x = 0; ++x");
	statement->run(window);

	auto variable = window.getVariable("x");

	ASSERT_EQ(variable.toString(), "1");

	window.deleteVariable("x");
	runGarbageCollection();
}

TEST_CASE("while loop") {
	auto statement = Compiler::compile("var x = 0; while (x < 3) { ++x }");
	statement->run(window);

	auto variable = window.getVariable("x");
	ASSERT_EQ(variable.toString(), "3");

	window.deleteVariable("x");
	runGarbageCollection();
}

TEST_CASE("for loop") {
	auto statement = Compiler::compile("for (let i = 0; i < 2; ++i) {}");
	statement->run(window);
	VariableGuard guard("i");

	auto variable = window.getVariable("i");

	ASSERT_EQ(variable.toNumber(), 2);
}

TEST_CASE("property accessor") {
	auto statement = Compiler::compile("x.y = 2");
	VariableGuard g("x");

	auto o = new ObjectValue;
	o->defineVariable("y", "0");
	window.defineVariable("x", *o);

	statement->run(window);

	auto result = o->getVariable("y");
	ASSERT_EQ(result.toString(), "2");
}

TEST_CASE("function declaration") {
	try {
		{
			VariableGuard guard({"x", "apa"});
			auto statement = Compiler::compile("var x = 'apa'; function apa() {x = 'bepa'}");
			auto callStatement = Compiler::compile("apa()");
			statement->run(window);

			auto variable = window.getVariable("x");
			ASSERT_EQ(variable.toString(), "apa");

			callStatement->run(window);
			variable = window.getVariable("x");
			ASSERT_EQ(variable.toString(), "bepa");
		}
		{
			VariableGuard guard({"x", "apa"});
			auto statement = Compiler::compile("var x = 'apa'; function apa(y, z, a, b) {x = y}");
			auto callStatement = Compiler::compile("apa('bepa')");
			statement->run(window);

			auto variable = window.getVariable("x");
			ASSERT_EQ(variable.toString(), "apa");

			callStatement->run(window);
			variable = window.getVariable("x");
			ASSERT_EQ(variable.toString(), "bepa");
		}
	} catch (CompilationException &e) {
		cout << e.what << ":" << e.token << endl;
		ASSERT(0, e.what);
	}
}

TEST_CASE("code block") {
	VariableGuard g("y");
	auto codeBlock = Compiler::compile("{var y; y = 3;}");
	codeBlock->run(window);

	auto variable = window.getVariable("y");
	ASSERT_EQ(variable.toString(), "3");
}

TEST_CASE("binary statements") {
	{
		auto statement = Compiler::compile("4 + 5");
		auto variable = statement->run(window);
		ASSERT_EQ(variable.toString(), "9");
	}
	{
		auto statement = Compiler::compile("2 == 2");
		auto variable = statement->run(window);
		ASSERT_EQ(variable.toString(), "true");
	}
	{
		auto statement = Compiler::compile("2 != 3");
		auto variable = statement->run(window);
		ASSERT_EQ(variable.toString(), "true");
	}
	{
		auto statement = Compiler::compile("2 <= 3");
		auto variable = statement->run(window);
		ASSERT_EQ(variable.toString(), "true");
	}

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

TEST_CASE("object test") {
	VariableGuard g("x");
	auto objectDeclaration = Compiler::compile("var x = {x: 1, y: 2, z: 3}");

	auto ret = objectDeclaration->run(window).getObject();


	ASSERT(ret, "not an object");
	std::cout << ret->toString() << std::endl;
	ASSERT_EQ(ret->getVariable("x").toString(), "1");
}

TEST_CASE("empty object") {
	VariableGuard g("x");
	Compiler::run("var x = {}", window);

	//The test is that the program should not crash
}

TEST_CASE("function as arguments") {
	VariableGuard g({"x", "apa"});
	auto statement = Compiler::compile("var x = 0; function apa(x) {x()}; apa(function() {x = 2});");
	statement->run(window);

	auto variable = window.getVariable("x");
	ASSERT_EQ(variable.toString(), "2");
}

TEST_CASE("simple closures") {
	VariableGuard g({"x"});
	Compiler::run("var x = 0; function apa() {x = 8}; apa()", window);

	auto variable = window.getVariable("x");
	ASSERT_EQ(variable.toString(), "8");
}

TEST_CASE("console log does not chrash") {
	Compiler::run("console.log('hej')", window);
}

TEST_CASE("function in function to get variable") {
	VariableGuard g({"x", "f", "f2"});
	Compiler::run("function f() {var x = 9; return function() {return x}};", window);
	Compiler::run("var f2 = f();", window);

	runGarbageCollection();

	auto value = Compiler::run("var x = f2()", window);

	ASSERT_EQ(value.toString(), "9");
}

TEST_CASE("function with return statement") {
	VariableGuard g({"x", "f"});
	auto value = Compiler::run("function f() {return 4; 8;}; var x = f()", window);
	ASSERT_EQ(value.toString(), "4");
}

TEST_CASE("function return local variable") {
	VariableGuard g({"x", "f"});
	auto value = Compiler::run("function f() {var ret = 12; return ret;}; var x = f()", window);
	ASSERT_EQ(value.toString(), "12");
}

TEST_CASE("object prototype") {
	VariableGuard g({"x", ""});
	auto value = Compiler::run("{}", window);
	ASSERT_EQ(value.getObject()->prototype, ObjectValue::Prototype());

	auto toStringFunction = Compiler::run("{}.toString", window);
	ASSERT(toStringFunction.getObject(), "function not found");
}


TEST_SUIT_END


