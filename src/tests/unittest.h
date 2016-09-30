/*
 * unittest.h
 *
 *  Created on: 6 okt 2014
 *      Author: Mattias Larsson Sköld
 *      https://github.com/mls-m5/mls-unit-test
 */


#pragma once

#include <map>
#include <string>
#include <iostream>

typedef void (*testFunction)();

extern std::map<std::string, testFunction> testMap;
extern int test_result;
extern const char* test_file_name;
static std::map<std::string, std::string> test_results;

inline int runTests(){
	using std::cout;
	using std::endl;
	int numFailed = 0;
	int numSucceded = 0;
	cout << "==== Starts test suit " << test_file_name <<  " ====" << endl << endl;

	for (auto it: testMap){
		cout << "--- running test " << it.first << " ---" << endl;
		test_result = 0;
		try {
			it.second();
		}
		catch (...) {
			std::cerr << "error" << endl;
			test_result = -2;
		}
		if (test_result == -1) {
			cout << " --> not impl" << endl << endl;
			test_results[it.first] = "not implemented";
		}
		if (test_result == -2) {
			cout << " --> crashed" << endl << endl;
			test_results[it.first] = "crashed!";
		}
		else if (test_result){
			cout << " --> failed" << endl << endl << endl << endl;
			numFailed ++;
			test_results[it.first] = "failed";
		}
		else{
			cout << " --> success " << endl << endl << endl << endl;
			test_results[it.first] = "succeded";
			numSucceded ++;
		}
	}

	cout << endl;
	cout << "results: " << endl;
	for (auto it: test_results) {
		cout << it.first;
		for (int i = it.first.size(); i < 40; ++i) {
			cout << " ";
		}
		cout << " :" << it.second << endl;
	}
	cout << endl;
	if (numFailed){
		cout << "MISSLYCKADES";
	}
	else{
		cout << "LYCKADES... hej då";
	}
	cout << endl;
	cout << "Failed: " << numFailed << " Succeded: " << numSucceded << endl;

	return numFailed > 0;
}

#define TEST_SUIT_BEGIN  std::map<std::string, testFunction> testMap; int test_result; const char *test_file_name = __FILE__; void initTests(){

//Remember to return 0 on success!!!
#define TEST_CASE(name) ; testMap[name] = []() -> void

#define TEST_SUIT_END ; } int main() { initTests(); return runTests(); }

#define PRINT_INFO std::cout << __FILE__ << ":" << __LINE__ << ": " ;
#define ASSERT(x, error) if (!(x)) { PRINT_INFO; test_result ++; std::cout << #x << ": " << error << std::endl; return; }
#define ASSERT_EQ(x, y) if ((x != y)) { PRINT_INFO; test_result ++; std::cout << #x << " = " << x << \
	" is not equal to "	<< #y << " = " << y << std::endl; test_result ++; return; }
#define ASSERT_NE(x, y) if ((x == y)) { PRINT_INFO; std::cout << #x << " = " << x << \
	" is equal to "	<< #y << " = " << y << std::endl; test_result ++; return; }

#define ASSERT_GT(x, y) if (!(x > y)) { PRINT_INFO; std::cout << #x << " = " << x << \
	" is not greater than "	<< #y << " = " << y << std::endl; test_result ++; return; }

#define ASSERT_LT(x, y) if (!(x < y)) { PRINT_INFO; std::cout << #x << " = " << x << \
	" is not less than "	<< #y << " = " << y << std::endl; test_result ++; return; }

#define ERROR(error) PRINT_INFO; std::cout << error << std::endl; test_result ++; return;

//The not implemented error is used to flag for wanted features not implemented
#define ERROR_NOT_IMPLEMENTED() PRINT_INFO; std::cout << "not implemented" << std::endl; test_result = -1; return;
