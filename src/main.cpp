
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
			auto statement = Compiler::compile(line);
			statement->run(window);
		}
	}
}
