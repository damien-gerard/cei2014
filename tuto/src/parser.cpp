#include "../include/parser.h"
#include "../include/lexer.h"
#include <typeinfo>

Parser::Parser(Lexer& lex)
: _lexer(lex), _tok(lex.nextToken())
{
    this->_precedence['<'] = 10;
    this->_precedence['+'] = 20;
    this->_precedence['-'] = 30;
    this->_precedence['*'] = 40;
}

Token Parser::eatToken() {
    this->_tok = this->_lexer.nextToken();
    return this->_tok;
}


int Parser::getPrec() {
    if (!isascii((this->chr()))) {
        return -1;
    }

    int prec = this->_precedence[this->chr()];
    if (prec <= 0) return -1;
    return prec;
}


/***
 * Parser Methods
 */
ExprAST* Parser::number() {
    ExprAST* result = new NumberAST(this->_tok.dbl());
    this->eatToken();
    return result;
}


ExprAST* Parser::parenthesis() {
    // Consomme la parenthèse ouvrante
    this->eatToken();
    ExprAST* V = this->expression();

    if (!V) return nullptr; // Propage l'erreur

    if (this->_tok != Token::RIGHTP) {
        return AST::Error<ExprAST>("expected ')'");
    }

    // Consomme la parenthèse fermante
    this->eatToken();
    return V;
}


ExprAST* Parser::identifier() {
    std::string idName = this->_lexer._str;

    // Consomme l'identifieur
    this->eatToken();

    // Simple identifer
    if (this->_tok != Token::RIGHTP) {
        return new VariableAST(this->_lexer._str);
    }

    // Appel de fonction
    // Consomme la parenthèse ouvrante
    this->eatToken();
    std::vector<ExprAST*> args;
    if (this->_tok != Token::RIGHTP) {
        while (true) {
            ExprAST* arg = this->expression();
            if (!arg) return nullptr; // Propage l'erreur
            args.push_back(arg);

            if (this->_tok == Token::RIGHTP) {
                break;
            }

            if (this->_tok != Token::COMMA) {
                return AST::Error<ExprAST>("Expected ')' or ',' in argument list");
            }

            this->eatToken();
        }
    }

    this->eatToken();


    return new CallAST(idName, args);
}


ExprAST* Parser::expression() {
    ExprAST *LHS = this->primary();
    if (!LHS) return nullptr;

    return this->binOpRHS(0, LHS);
}


ExprAST* Parser::binOpRHS(int ExprPrec, ExprAST *LHS) {
    while (true) {
        int tokPrec = this->getPrec();

        // Also true if end of expression
        if (tokPrec < ExprPrec) {
            return LHS;
        }

        int binOP = this->_tok.chr();
        this->eatToken();

        ExprAST *RHS = this->primary();
        if (!RHS) return nullptr;

        int nextPrec = this->getPrec();
        if (tokPrec < nextPrec) {
            RHS = this->binOpRHS(tokPrec + 1, RHS);
            if (!RHS) return nullptr;
        }

        LHS = new OpAST(binOP, LHS, RHS);
    }
}


ExprAST* Parser::primary() {
    switch (this->_tok) {
    case Token::ID:
        return this->identifier();
    case Token::NUMBER:
        return this->number();
    case Token::LEFTP:
        return this->parenthesis();
    default:
        return AST::Error<ExprAST>("unknown token when expecting an expression");
    }
}

PrototypeAST* Parser::prototype() {
    if (this->_tok != Token::ID) {
        return AST::Error<PrototypeAST>("Expected function name in prototype");
    }

    std::string fnName = this->_tok.str();
    this->eatToken();

    if (this->_tok != Token::LEFTP) {
        return AST::Error<PrototypeAST>("Expected '(' in prototype");
    }

    std::vector<std::string> argNames;
    while (this->eatToken() == Token::ID) {
        argNames.push_back(this->_tok.str());
    }

    if (this->_tok != Token::RIGHTP) {
        return AST::Error<PrototypeAST>("Expected ')' in prototype");
    }

    this->eatToken();

    return new PrototypeAST(fnName, argNames);
}


FunctionAST* Parser::functionDef() {
    this->eatToken();
    PrototypeAST* proto = this->prototype();
    if (!proto) return nullptr;

    if (ExprAST *E = this->expression()) {
        return new FunctionAST(proto, E);
    }
    return nullptr;
}


PrototypeAST* Parser::externDef() {
    this->eatToken();
    return this->prototype();
}



FunctionAST* Parser::topLevelExpr() {
    if (ExprAST* E = this->expression()) {
        PrototypeAST* proto = new PrototypeAST("", std::vector<std::string>());
        return new FunctionAST(proto, E);
    }
    return nullptr;
}



AST* Parser::parseLine() {
    switch (this->_tok) {
    case Token::END:
        return AST::Error("EOF");
    case Token::SEMICOL:
        this->eatToken();
        return parseLine();
    case Token::DEF:
        return this->functionDef();
    case Token::EXTERN:
        return this->externDef();
    default:
        return this->expression();
    }
}

void Parser::parse() {
    while (this->_tok != Token::END) {
        AST* ast = this->parseLine();
        if (!ast) {
            if (this->_tok != Token::END) {
                this->eatToken();
            }
            std::cerr << "Parse error" << std::endl;
        } else if (typeid(*ast) == typeid(PrototypeAST)) {
            std::cerr << "Extern declaration" << std::endl;
        } else if (typeid(*ast) == typeid(FunctionAST)) {
            std::cerr << "Function definition" << std::endl;
        } else if (typeid(*ast) == typeid(DefinitionAST)) {
            this->eatToken();
            std::cerr << "Definition error" << std::endl;
        } else {
            std::cerr << "Expression" << std::endl;
        }
    }
}

