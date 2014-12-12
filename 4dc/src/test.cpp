#include "../include/test.h"
#include "../include/lexer.h"
#include <iostream>
#include <fstream>

//Test
Test::Test(std::string filename, TestResult expectedResult){	
	this->_filename = filename;
	this->_expected_result = expectedResult;
	this->_true_result = TestResult::NONE;
};

void Test::runTestOnLexer(){
	std::ifstream myfile(this->_filename);
	if (myfile.is_open())
	{
		try{
			Lexer lexer(myfile);
			this->_true_result = TestResult::PASS;
			
		}catch( std::string notImplementedException){
			std::cout << "file " << this->_filename << ": notImplementedFail\n";
			this->_true_result = TestResult::NOTIMPLEMENTEDFAIL;
		}
		//catch(std::exception e){
		//	std::cout << "file " << this->_filename << ": Fail\n";
		//	this->_true_result = TestResult::MUSTFAIL;
		//}
	}else{
		std::cout << "file " << this->_filename << ": not read\n";
	};
};


//Tests
Tests::Tests(){
	this->_tests_list = new std::list<Test*>();
	this->loadTestsList();
};

Tests::~Tests(){
	for( Test * test : *(this->_tests_list)){
		delete test;
	};
	delete this->_tests_list;
};

void Tests::add(std::string filename, TestResult expected){
	this->_tests_list->push_back(new Test(this->_tests_directory + filename,expected));
};

void Tests::runAllTestsOnLexer(){
	std::cout << "Begin tests on lexer\n";
	for( Test * test : *(this->_tests_list)){
		test->runTestOnLexer();
	};
};