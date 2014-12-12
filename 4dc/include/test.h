#ifndef TEST_H
#define TEST_H

#include "lexer.h"
#include <list>
#include <string>

enum TestResult{
	NONE,
	PASS,
	MUSTFAIL,
	NOTIMPLEMENTEDFAIL
};

class Test{
public:
	Test(std::string filename, TestResult expectedResult);
	
	void runTestOnLexer();
private:
	std::string _filename;
	TestResult _expected_result;
	TestResult _true_result;
};

class Tests{
public :
	Tests();
	~Tests();
	
	void runAllTestsOnLexer();
	
private:
	
	void loadTestsList();
	void add(std::string filename, TestResult expectedResult);
	
	std::string _tests_directory;
	std::list<Test*>* _tests_list;
};



#endif // TEST_H
