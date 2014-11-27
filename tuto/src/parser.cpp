#include "../include/parser.h"
#include "../include/lexer.h"
#include <typeinfo>
#include <sstream>

Parser::Parser(Lexer& lex)
: _lexer(lex)
{
    this->_precedence['<'] = 10;
    this->_precedence['+'] = 20;
    this->_precedence['-'] = 30;
    this->_precedence['*'] = 40;

    this->eatToken();
}

Token* Parser::eatToken() {
    this->_tok = this->_lexer.nextToken();
    return &this->_tok;
}

Token* Parser::eatToken(const TokenType& type) {
    if (this->_tok != type) {
      std::stringstream ss;
      ss << "Unexpected token " << this->_tok << ", expected " << type;
      return AST::Error<Token>(ss.str());
    }
    this->_tok = this->_lexer.nextToken();
    return &this->_tok;
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

    if (!this->eatToken(Token::RIGHTP)) return nullptr;

    return V;
}


ExprAST* Parser::identifier() {
    std::string idName = this->_tok.str();

    // Consomme l'identifieur
    this->eatToken();

    // Simple identifer
    if (this->_tok != Token::LEFTP) {
        return new VariableAST(idName);
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

            if (!this->eatToken(Token::COMMA)) return nullptr;
        }
    }

    // Consomme la parenthèse fermante
    this->eatToken();


    return new CallAST(idName, args);
}

ExprAST* Parser::ifexpr() {
    ExprAST *ifAST, *thenAST, *elseAST;
    // Consomme le token IF
    this->eatToken();

    // Parse la condition
    ifAST = this->expression();
    if (!ifAST) return nullptr;

    // Consomme le token THEN
    if (!this->eatToken(Token::THEN)) return nullptr;

    // Parse l'expression THEN
    thenAST = this->expression();
    if (!thenAST) return nullptr;

    // Consomme le token ELSE
    if (!this->eatToken(Token::ELSE)) return nullptr;

    // Parse l'expression ELSE
    elseAST = this->expression();
    if (!elseAST) return nullptr;

    return new IfAST(ifAST, thenAST, elseAST);
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
    case Token::IF:
        return this->ifexpr();
    default:
        return AST::Error<ExprAST>("unknown token when expecting an expression");
    }
}

PrototypeAST* Parser::prototype() {
    std::string fnName = this->_tok.str();
    if (!this->eatToken(Token::ID)) return nullptr;

    if (!this->eatToken(Token::LEFTP)) return nullptr;

    std::vector<std::string> argNames;
    while (this->_tok == Token::ID) {
        argNames.push_back(this->_tok.str());
        this->eatToken();
    }

    if (!this->eatToken(Token::RIGHTP)) return nullptr;

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
    bool semicol = false;
    AST* ast = nullptr;
    while (!ast) {
      switch (this->_tok) {
      case Token::END:
          return AST::Error("EOF");
      case Token::SEMICOL:
          semicol = true;
          this->eatToken();
          break;
      case Token::DEF:
          ast = this->functionDef();
          break;
      case Token::EXTERN:
          ast = this->externDef();
          break;
      default:
          ast = this->expression();
      }
      if (this->_tok == Token::END) return ast;

      // parseLine renvoie toujours un objet AST,
      // sauf si le fichier est à la fin
      if (!ast && !semicol) {
          std::cerr << "Parse error" << std::endl;
          this->eatToken();
      }
      semicol = false;
    }
    return ast;
}

void Parser::parse() {
    while (this->_tok != Token::END) {
        AST* ast = this->parseLine();
        if (!ast) {
            if (this->_tok != Token::END) {
            std::cerr << "Internal error" << std::endl;
                this->eatToken();
            }
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

