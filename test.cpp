
#include <iostream>
#include <memory>
#include <map>
#include <vector>

using namespace std;

class Identifier {
public:
	Identifier() = default;
	Identifier(const Identifier&) = default;
	Identifier(Identifier &&) = default;
	Identifier(string name) : name(name) {};
	Identifier(const char name[]) : name(name) {};
	string name;
	
	//Todo: Add member functions and stuff
	//Todo add posibility to use fixed place in memory
};

class ObjectValue {
public:
	virtual ~ObjectValue() {};

//	Value &run(class Context& context) {
//		return "";
//	}
	//Todo: Implement garbage collection

	virtual ObjectValue *copy() {return new ObjectValue(*this);};
};

class StringValue: public ObjectValue {
public:
	StringValue() = default;
	StringValue(const StringValue &) = default;
	StringValue(StringValue &&) = default;
	StringValue(string value): value(value) {
	}
	virtual ~StringValue() {};
	std::string value;

	ObjectValue *copy() override {
		return new StringValue(*this);
	}
};

class FunctionCallValue: public ObjectValue {
public:

	class Identifier identifier;
};

class Value {
public:
	Value() = default;
	Value (const Value & value) {
		operator=(value);
	}
	Value (Value && ) = default;

	//Todo: Improve performance by adding copy by reference
	Value (const string & value) {
		setValue(value);
	}

	Value (const char *value) {
		setValue(value);
	}

	Value (const long & value) {
		setValue(value);
	}


	Value &operator=(const Value &value) {
		cout << "assignment of value " << endl;
		switch (value.type) {
		case Object:
		case String:
			objectValue.reset(value.objectValue->copy());
			break;
		case Undefined:
			type = Undefined;
			objectValue.reset(nullptr);
			break;
		 default:
			throw "not implemented";

		}
		type = value.type;
		return *this;
	}

	~Value() {
		//
	}

	Value setValue(string value) {
		type = String;
		objectValue.reset(new StringValue(value));
		return *this;
	}

	Value setValue(long value) {
		type = String;
		objectValue.reset();
		intValue = value;
		return *this;
	}

	string toString() {
		switch (type) {
		case Undefined:
			return "undefined";
		case String:
			return ((StringValue*)objectValue.get())->value;
		default:
			return "not implemented";
		}
	}

	enum VariableType {
		Boolean,
		Null,
		Undefined,
		Number,
		String,
		Symbol,
		Object,
		Integer
	} type = Undefined;
	
	//All types except objects are immutable objects
	union {
		bool boolValue;
		double numberValue;
		long intValue;
	};
	unique_ptr<ObjectValue> objectValue = nullptr;
};


class Context {
public:
	map<string, Value> variables;
	
	Context *parentContext = 0;
	
	Value getVariable(Identifier identifier) {
		auto f = variables.find(identifier.name);

		if (f == variables.end() && parentContext) {
			return parentContext->getVariable(identifier);
		}
		else {
			return f->second;
		}
	}

	Value setVariable(Identifier identifier, Value value) {
		variables[identifier.name] = value;
		return value;
	}
};

class Statement{
public:
	virtual ~Statement() {}
	virtual Value run(Context &context) {
		
	}

	virtual Statement *copy() const  = 0; //{
//		return new Statement(*this);
//	}
};

class Expression {
public:
	Expression(const Expression &e) {
		statement.reset(e.statement->copy());
	}
	Expression(Expression &&e) {
		statement = move(e.statement);
	}
	Expression(const Statement &s) {
		statement.reset(s.copy());
	}

	Value run(Context &context) {
		return statement->run(context);
	}

	unique_ptr<Statement> statement;
};


class Assignment: public Statement {
public:
	~Assignment() {}
	Assignment() = default;
	Assignment(Identifier identifier, Expression expression):
	identifier(identifier),
	expression(expression) {}

	Identifier identifier;
	Expression expression;
	
	Value run(Context &context) override {
		//Todo: in the more performant version this should be calculated in forehand
		auto value = expression.run(context);

		return context.setVariable(identifier, value);
	}

	Statement *copy() const override {
		return new Assignment(*this);
	}
};

class CodeBlock: public Statement {
public:
	~CodeBlock() {}
	vector<Expression> statements;
	
	Value run(Context &context) override {
		map<string, Value> localVariables;
		for (auto &statement: statements) {
			statement.run(context);
		}
		//unload scoped "let" variables
	}	


	Statement *copy() const override {
		return new CodeBlock(*this);
	}
};

class FunctionDeclaration: public CodeBlock {
public:
	~FunctionDeclaration() {}

	//Do special difference except the arguments
	Value run(Context &context) override {
		Context localContext;
		localContext.parentContext = &context;

		this->CodeBlock::run(localContext);

		//unload context variables
	}

	Statement *copy() const override {
		return new FunctionDeclaration(*this);
	}
};

class FunctionCall: public Statement {
	~FunctionCall() {}
	shared_ptr<FunctionDeclaration> function;

	Value run(Context &context) override {
		if (function) {
			function->run(context);
		}
	}

	Statement *copy() const override {
		return new FunctionCall(*this);
	}
};

class VariableGetter: public Statement {
public:
	~VariableGetter() {};
	VariableGetter() = default;
	VariableGetter(const VariableGetter &) = default;
	VariableGetter(VariableGetter &&) = default;
	VariableGetter(Identifier identifier): variableName(identifier) {}


	Value run(Context &context) override {
		return context.getVariable(variableName);
	}


	Statement *copy() const override {
		return new VariableGetter(*this);
	}

	Identifier variableName;
};

int main(int argc, char const *argv[])
{
	cout << "mjavascript 0.0001" << endl;
	//Testing
	Context context;
	Value value, value2;
	value.setValue("hej");
	cout << "apa direct: " << value.toString() << endl;
	value2 = value;
	cout << "apa after copy: " << value2.toString() << endl;



	context.setVariable("bepa", "tja");
	context.setVariable("cepa", "re");
	cout << "bepa: " << context.getVariable("bepa").toString() << endl;
	Assignment assignment1("apa", VariableGetter("bepa"));
	assignment1.run(context);
	cout << "apa: " << context.getVariable("apa").toString() << endl;

	return 0;
}
