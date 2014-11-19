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
        void parse();
    protected:
    private:
        Lexer& _lexer;
        Token _tok;
        std::map<char, int> _precedence;

        Token eatToken();
        inline char chr() {return this->_tok.chr();}
        int getPrec();

        ExprAST* number();
        ExprAST* parenthesis();
        ExprAST* identifier();
        ExprAST* expression();
        ExprAST* binOpRHS(int ExprPrec, ExprAST *LHS);
        ExprAST* primary();
        PrototypeAST* prototype();
        FunctionAST* functionDef();
        PrototypeAST* externDef();
        FunctionAST* topLevelExpr();


        AST* parseLine();
};

#endif // PARSER_H
