
#include "virtualmachine.h"
#include "lexer.h"
//#include <sstream>
#include <fstream>
#include "compiler.h"

using namespace std;

void printValue(Value value) {
	cout << value.toString() << endl;
}

vector<Statement *> statements;

//class SimpleInterpreter {
//public:
//	SimpleLexer lexer;
//
//	static Value parseValue(Token word) {
//		if (word[0] == '\"') {
//			return Value(word.substr(1, word.size() - 2));
//		}
//		else if (word == ".") {
//			auto statement = new Identifier(word);
//			return *statement;
//		}
//		else if (isdigit(word[0]) || word[0] == '.') {
//			istringstream ss(word);
//			int number;
//			ss >> number;
//			return number;
//		}
//		else {
//			auto statement = new Identifier(word);
//			statements.push_back(statement);
//			return *statement;
//		}
//	}
//
//	vector<Value> split(string line) {
//		std::vector<Value> vec;
//
//		auto words = lexer.tokenize(line);
//
//		for (auto &it: words) {
//			vec.push_back(parseValue(it));
//		}
//
//		return vec;
//	}
//
//	void interpret(string line) {
////		cout << "interpretting " << line << endl;
//		auto words = split(line);
//
//		if (words.empty()) {
//			return;
//		}
//		auto first = words[0].toString();
////		cout << "command: " << first << endl;
//		if (first == "assign") {
//			Assignment ass (words[1], words[2]);
//			cout << ass.run(window).toString() << endl;
//		}
//		else if (first == "assmem") {
//			PropertyAccessor accessor(words[1], words[2].toString());
//			Assignment ass(accessor, words[3]);
//
//			printValue(ass.run(window).toString());
//		}
//		else if (words.size() == 4 && words[1].toString() == "(" && words[3].toString() == ")") {
//			FunctionCall call(words[0], words[2]);
//			call.run(window);
//		}
//		else if (first == "let" || first == "var") {
//			Value defaultValue;
//			if (words.size() > 3 || words[2].toString() == "=") {
//				defaultValue = words[3];
//			}
//			VariableDeclaration declaration(words[1].toString(), defaultValue);
//			printValue(declaration.run(window));
//		}
//		else if (first == "delete") {
//			DeleteStatement deleteStatement(words[1].toString());
//			printValue(deleteStatement.run(window));
//		}
//		else if (words.size() == 3 && words[1].toString() == "=") {
//			Assignment ass(words[0], words[2]);
//			cout << ass.run(window).toString() << endl;
//		}
//		else if (words.size() == 3 && words[1].toString() == ".") {
//			PropertyAccessor accessor(words[0], words[2].toString());
//			printValue(accessor.run(window));
//		}
//		else if(words.size() == 1) {
//			printValue(words[0].run(window));
//		}
//	}
//};

int parseFile(string fname) {
	ifstream file(fname);

	auto statement = Compiler::compile(file);
	statement->run(window);
	return 0;
}

int printAst() {
	AstUnit unit(cin);

	unit.print(cout);

	return 0;
}

int printTokens() {
	SimpleLexer lexer;

	auto tokens = lexer.tokenize(cin);

	for (auto it: tokens) {
		cout << it << endl;
	}

	return 0;
}


int main(int argc, char const *argv[])
{
	cout << "mjavascript 0.0003" << endl;

//	SimpleInterpreter interpreter;

	if (argc > 1) {
		string arg1 = argv[1];
		if (arg1 == "--ast") {
			return printAst();
		}
		else if (arg1 == "--token") {
			return printTokens();
		}
		else {
			return parseFile(arg1);
		}
	}


	auto handleErrors = true;

	vector<StatementPointer> statements;

	while (!cin.eof()) {
		cout << ">> ";
		cout.flush();
		string line;
		getline(cin, line);
		if (handleErrors) {
			try {
//				interpreter.interpret(line);
				auto statement = Compiler::compile(line);
				statements.push_back(statement);
				cout << statement->run(window).toString() << endl;
			}
			catch (const char *e) {
				cout << e << endl;
			}
			catch (RuntimeException &e) {
				cout << e.what << endl;
			}
			catch (CompilationException &e) {
				cout << e.token << ": " << e.what << endl;
			}
		}
		else {
//			interpreter.interpret(line);
			auto statement = Compiler::compile(line);
			statement->run(window);
		}
	}
}
