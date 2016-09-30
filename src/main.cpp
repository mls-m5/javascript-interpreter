
#include "virtualmachine.h"

using namespace std;


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
