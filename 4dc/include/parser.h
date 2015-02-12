#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "ast.h"
#include <map>
class Lexer;
class Token;
class Parser
{
  public:
    Parser(Lexer&);
  ~Parser();
    void parse();
  AST* ast();
  protected:
  private:
    Lexer& _lexer;
    Token _tok;
    AST* _ast;
    int _currentLine;
    
    std::string getErrorHeader();

    Token* eatToken();
    Token* eatToken(const TokenType&);
  
    ExprAST* expression();
    ExprAST* binOpRHS(ExprAST* LHS);
    ExprAST* primary();
    ExprAST* uniOpExpr();
    ExprAST* literal(VarType vtype);
    ExprAST* parenthesis();
    ExprAST* identifier();
    ExprAST* callFunction(std::string functionName);

    VariableAST* persistantVariable();
    VariableAST* localVariable();


    BlocAST* bloc();
    StatementAST* statement();  
    StatementAST* ifstatement();
    StatementAST* forstatement();
    StatementAST* whilestatement();
    StatementAST* repeatstatement();
  
    PrototypeAST* prototype();
    // FunctionAST* functionDef();
    // PrototypeAST* externDef();
    // FunctionAST* topLevelExpr();
};

#endif // PARSER_H
