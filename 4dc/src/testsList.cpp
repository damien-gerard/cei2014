#include "../include/test.h"

void Tests::loadTestsList(){
	//dossier de test
	this->_tests_directory = "tests/lexerTests/";

	//Liste des tests
	this->add("test1.4d",TestResult::PASS);
};