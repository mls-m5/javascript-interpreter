
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
			PropertyAccessor accessor(words[1], words[2].toString());
			Assignment ass(accessor, words[3]);

			printValue(ass.run(window).toString());
		}
		else if (first == "call") {
			FunctionCall call(words[1], words[2]);
			call.run(window);
		}
		else if (first == "let" || first == "var") {
			Value defaultValue;
			if (words.size() > 3 || words[2].toString() == "=") {
				defaultValue = words[3];
			}
			VariableDeclaration declaration(words[1].toString(), defaultValue);
			printValue(declaration.run(window));
		}
		else if (first == "delete") {
			DeleteStatement deleteStatement(words[1].toString());
			printValue(deleteStatement.run(window));
		}
		else if (words.size() == 3 && words[1].toString() == "=") {
			Assignment ass(words[0], words[2]);
			cout << ass.run(window).toString() << endl;
		}
		else if (words.size() == 3 && words[1].toString() == ".") {
			PropertyAccessor accessor(words[0], words[2].toString());
			printValue(accessor.run(window));
		}
		else if(words.size() == 1) {
			printValue(words[0].run(window));
		}
	}
};


int main(int argc, char const *argv[])
{
	cout << "mjavascript 0.0003" << endl;

	SimpleInterpreter interpreter;

	while (!cin.eof()) {
		cout << ">> ";
		cout.flush();
		string line;
		getline(cin, line);
		interpreter.interpret(line);
	}
}
