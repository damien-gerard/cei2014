#include <iostream>

#include "include/token.h"
#include "include/lexer.h"
#include "include/parser.h"
#include "include/ast.h"
#include "include/builder.h"
#include "include/util/logger.h"
#include <fstream>

using namespace std;

void compileFile(istream&, Builder&);

int main(int argc, char *argv[])
{
  Builder builder;
  builder.createJIT();
  builder.setOptimizer(builder.getStandardOptimizer());

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
      compileFile(myfile, builder);
    
    }else{
      Logger::error << "Error: cannot find \"" << current_exec_name << "\"" << endl;
      exit(EXIT_FAILURE);
    }

  }

  if (readSTDIN) {
    compileFile(cin, builder);
  }
  return 0;
}

void compileFile(istream& file, Builder& builder) {
  Lexer lexer(file);
  Parser parser(lexer);
  parser.parse();
  AST* ast = parser.ast();
  if (!ast) {
    exit(EXIT_FAILURE);
  }
  Logger::info << *ast << endl << endl;
  //builder.build(ast);
}
