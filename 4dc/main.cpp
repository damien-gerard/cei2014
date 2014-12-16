#include <iostream>

#include "include/token.h"
#include "include/lexer.h"
#include "include/util/logger.h"
#include <fstream>

using namespace std;

int main(int argc, char *argv[])
{
  Token t;

  bool readSTDIN = true;
  for(int i =1; i < argc; i++){
    readSTDIN = false;
    std::string current_exec_name = argv[i];
    if (current_exec_name == "-") {
      readSTDIN = true;
      break;
    }
    Logger::info << "\"" << current_exec_name << "\" :" << endl;
    std::ifstream myfile(current_exec_name);
    if(myfile){
      Lexer lex(myfile);


      while ((t = lex.nextToken()) != TokenType::ENDF) {
        Logger::info << "->" << t << endl;
      }
    }else{
      Logger::error << "Error: cannot find \"" << current_exec_name << "\"" << endl;
      exit(EXIT_FAILURE);
    }

  }

  if (readSTDIN) {
    Lexer lex(cin);
    while ((t = lex.nextToken()) != TokenType::ENDF) {
      Logger::info << "->" << t << endl;
    }
  }

  return 0;
}
