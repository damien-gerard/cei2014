#include <iostream>

#include "include/token.h"
#include "include/lexer.h"
#include <fstream>

using namespace std;

int main(int argc, char *argv[])
{
  Token t;
  
  
  for(int i =1; i < argc; i++){
	std::string current_exec_name = argv[i];
	cout << "/tests/lexerTests/" << current_exec_name << " : ";
	std::ifstream myfile("../../tests/lexerTests/"+current_exec_name); 
	if(myfile){
	Lexer lex(myfile);

	cout << "done\n";
	
	while ((t = lex.nextToken()) != TokenType::ENDF) {
	cout << "->" << t << endl;
	}
	}else{
		cout << "not found\n";
	}

	
  }
  
  Lexer lex(cin);
  while ((t = lex.nextToken()) != TokenType::ENDF) {
		cout << "->" << t << endl;
  }
  
  return 0;
}
