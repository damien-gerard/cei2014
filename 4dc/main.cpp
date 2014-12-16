#include <iostream>

#include "include/token.h"
#include "include/lexer.h"
#include "include/util/logger.h"
#include <fstream>

using namespace std;
using namespace Logger;

int main(int argc, char *argv[])
{
  Token t;


  for(int i =1; i < argc; i++){
    std::string current_exec_name = argv[i];
    Logger::info << "/tests/lexerTests/" << current_exec_name << " : ";
    std::ifstream myfile("../../tests/lexerTests/"+current_exec_name);
    if(myfile){
      Lexer lex(myfile);

      Logger::info << "done\n";

      while ((t = lex.nextToken()) != TokenType::ENDF) {
        Logger::info << "->" << t << endl;
      }
    }else{
      Logger::info << "not found\n";
    }

  }

  Lexer lex(cin);
  while ((t = lex.nextToken()) != TokenType::ENDF) {
    Logger::info << "->" << t << endl;
  }

  return 0;
}
