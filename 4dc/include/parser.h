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

    Token* eatToken();
    Token* eatToken(const TokenType&);
	
    ExprAST* expression();
    ExprAST* binOpRHS(ExprAST* LHS);
    ExprAST* primary();
    ExprAST* literal();
    ExprAST* parenthesis();
    ExprAST* identifier();
    ExprAST* persistantVariable();
    ExprAST* localVariable();


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
