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
        AST* parseLine();
    protected:
    private:
        Lexer& _lexer;
        Token _tok;
        std::map<char, int> _precedence;

        Token* eatToken();
        Token* eatToken(const TokenType&);
        inline char chr() {return this->_tok.chr();}
        int getPrec();

        ExprAST* number();
        ExprAST* parenthesis();
        ExprAST* identifier();
        ExprAST* ifexpr();
        ExprAST* expression();
        ExprAST* binOpRHS(int ExprPrec, ExprAST *LHS);
        ExprAST* primary();
        PrototypeAST* prototype();
        FunctionAST* functionDef();
        PrototypeAST* externDef();
        FunctionAST* topLevelExpr();
};

#endif // PARSER_H
