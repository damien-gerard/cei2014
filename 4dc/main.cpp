#include <iostream>

#include "include/token.h"
#include "include/lexer.h"
#include "include/parser.h"
#include "include/ast.h"
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


      // while ((t = lex.nextToken()) != TokenType::ENDF) {
        // Logger::info << "->" << t << endl;
      // }
    Parser parser(lex);
    parser.parse();
      if(!parser.ast()){
        exit(EXIT_FAILURE);
      }
    Logger::info << *parser.ast() <<std::endl<<std::endl;
    
    }else{
      Logger::error << "Error: cannot find \"" << current_exec_name << "\"" << endl;
      exit(EXIT_FAILURE);
    }

  }

  if (readSTDIN) {
    Lexer lex(cin);
    // while ((t = lex.nextToken()) != TokenType::ENDF) {
      // Logger::info << "->" << t << endl;
    // }
    Parser parser(lex);
  parser.parse();
    if(!parser.ast()){
      exit(EXIT_FAILURE);
    }
  Logger::info << *parser.ast() <<std::endl<<std::endl;
  }

  // AST* ast =  new BlocAST({
                // new IfAST(
                  // new OpAST("<=",
                    // new OpAST("*",
                      // new GlobaleVariableAST("test"),
                      // new LiteralAST("2")
                    // ),
                    // new LiteralAST("10")
                  // ),
                  // new BlocAST({
                    // new StatementExprAST(
                      // new OpAST(":=",
                        // new LocalVariableAST("foo"),
                        // new OpAST("-",
                          // new PersistentVariableAST("bar"),
                          // new LiteralAST("4")
                        // )
                      // )
                    // ),
                    // new StatementExprAST(
                      // new OpAST(":=",
                        // new PersistentVariableAST("foo"),
                        // new OpAST("+",
                          // new LocalVariableAST("bar"),
                          // new LiteralAST("7")
                        // )
                      // )
                    // )
                  // }),
                  // new BlocAST({
                    // new StatementExprAST(
                      // new OpAST(":=",
                        // new LocalVariableAST("foo"),
                        // new OpAST("/",
                          // new PersistentVariableAST("bar"),
                          // new LiteralAST("2")
                        // )
                      // )
                    // )
                  // })
                // ),
                // new StatementExprAST(
                  // new OpAST(":=",
                    // new GlobaleVariableAST("test"),
                    // new OpAST("-",
                      // new OpAST("+",
                        // new LocalVariableAST("foo"),
                        // new PersistentVariableAST("bar")
                      // ),
                      // new LiteralAST("57")
                    // )
                  // )
                // )}
              // );
        
  // Logger::info << *ast;
  // delete ast;
  return 0;
}
