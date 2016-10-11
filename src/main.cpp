
#include "virtualmachine.h"
#include <sstream>

using namespace std;

void printValue(Value value) {
	cout << value.toString() << endl;
}

vector<Statement *> statements;

class SimpleInterpreter {
public:
	static Value parseValue(string word) {
		if (word[0] == '\"') {
			return Value(word.substr(1, word.size() - 2));
		}
		else if (isdigit(word[0]) || word[0] == '.') {
			istringstream ss(word);
			int number;
			ss >> number;
			return number;
		}
		else {
			auto statement = new Identifier(word);
			statements.push_back(statement);
			return *statement;
		}
	}

	static vector<Value> split(string line) {
		std::istringstream ss(line);

		std::vector<Value> vec;
		string word;
		while (!ss.eof()) {
			ss >> word;
			vec.push_back(parseValue(word));
		}
		return vec;
	}

	void interpret(string line) {
//		cout << "interpretting " << line << endl;
		auto words = split(line);

		if (words.empty()) {
			return;
		}
		auto first = words[0].toString();
//		cout << "command: " << first << endl;
		if (first == "assign") {
			Assignment ass (words[1], words[2]);
			cout << ass.run(window).toString() << endl;
		}
		else if (first == "assmem") {
//			Assignment ass()
			PropertyAccessor accessor(words[1], words[2]);
			Assignment ass(accessor, words[3]);
			printValue(ass.run(window).toString());
		}
		else if (first == "call") {
			FunctionCall call(words[1], words[2]);
			call.run(window);
		}
		else if (first == "let") {
			VariableDeclaration declaration(words[1].toString());
			declaration.run(window);
		}
		else if (first == "delete") {
			DeleteStatement deleteStatement(words[1].toString());
			printValue(deleteStatement.run(window));
		}
		else if (words.size() == 3 && words[1].toString() == "=") {
			Assignment ass(words[0], words[2]);
			cout << ass.run(window).toString() << endl;
		}
		else if(words.size() == 1) {
			cout << window.getVariable(first).toString() << endl;
		}
	}
};


int main(int argc, char const *argv[])
{
	cout << "mjavascript 0.0002" << endl;

	SimpleInterpreter interpreter;

	while (!cin.eof()) {
		cout << ">> ";
		cout.flush();
		string line;
		getline(cin, line);
		interpreter.interpret(line);
	}

//	//Testing
//	ObjectValue context;
//	Value value, value2;
//	value.setValue("hej");
//	cout << "apa direct: " << value.toString() << endl;
//	value2 = value;
//	cout << "apa after copy: " << value2.toString() << endl;
//
//	context.setVariable("bepa", "tja");
//	context.setVariable("cepa", "re");
//	cout << "bepa: " << context.getVariable("bepa").toString() << endl;
//	Assignment assignment1("apa", VariableGetter("bepa"));
//	assignment1.run(context);
//	cout << "apa: " << context.getVariable("apa").toString() << endl;
//
//	return 0;
}
