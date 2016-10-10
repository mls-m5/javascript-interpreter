
#include "virtualmachine.h"
#include <sstream>

using namespace std;

void printValue(Value value) {
	cout << value.toString() << endl;
}

class SimpleInterpreter {
public:
	static vector<string> split(string line) {
		std::istringstream ss(line);

		std::vector<std::string> vec;
		string word;
		while (!ss.eof()) {
			ss >> word;
			vec.push_back(word);
		}
		return vec;
	}

	void interpret(string line) {
//		cout << "interpretting " << line << endl;
		auto words = split(line);

		if (words.empty()) {
			return;
		}
		auto first = words[0];
//		cout << "command: " << first << endl;
		if (first == "assign") {
			Assignment ass (words[1], words[2]);
			cout << ass.run(window).toString() << endl;
		}
		else if (first == "assmem") {
//			Assignment ass()
			PropertyAccessor accessor(words[1], words[2]);
			printValue(accessor.run(window).toString());
		}
		else if (first == "call") {
			FunctionCall call(words[1], words[2]);
			call.run(window);
		}
		if (first == "delete") {
			DeleteStatement deleteStatement(words[1]);
			printValue(deleteStatement.run(window));
		}
		else if (words.size() == 3 && words[1] == "=") {
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
