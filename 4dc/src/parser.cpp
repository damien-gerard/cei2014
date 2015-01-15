#include "../include/parser.h"
#include "../include/lexer.h"
#include "../include/util/logger.h"
#include <typeinfo>
#include <sstream>

Parser::Parser(Lexer& lex)
: _lexer(lex)
{
  this->eatToken();
}

Parser::~Parser(){
  if(this->_ast){
    delete this->_ast;
  }
}

Token* Parser::eatToken() {
  Logger::info << "Token: " << this->_tok.type() << std::endl;
  this->_tok = this->_lexer.nextToken();
  return &this->_tok;
}

Token* Parser::eatToken(const TokenType& type) {
  Logger::info << "Token: " << this->_tok.type() << std::endl;
  if (this->_tok != type) {
    std::stringstream ss;
    ss << "Unexpected token " << this->_tok << ", expected " << type;
    Logger::error << "Parse Error: " << ss << std::endl;
    return AST::Error<Token>(ss.str());
  }
  this->_tok = this->_lexer.nextToken();
  return &this->_tok;
}


/***
 * Parser Methods
 */
ExprAST* Parser::number() {
  ExprAST* result = new LiteralAST(this->_tok.str());
  this->eatToken();
  return result;
}


ExprAST* Parser::parenthesis() {
  // Consomme la parenthèse ouvrante
  this->eatToken();
  ExprAST* V = this->expression();

  if (!V) return nullptr; // Propage l'erreur

  if (!this->eatToken(TokenType::RIGHTP)){
    Logger::error << "Parse Error: right parenthesis ')' expected" << std::endl;
  return nullptr;
  }

  return V;
}
ExprAST* Parser::persistantVariable() {
  // Consomme le diamand
  this->eatToken();

  std::string idName = this->_tok.str();
  
  // Consomme l'identifiant
  this->eatToken();
  
  return new PersistentVariableAST(idName);
}

ExprAST* Parser::localVariable() {
  // Consomme le dollar
  this->eatToken();
  
  std::string idName = this->_tok.str();
  
  // Consomme l'identifiant
  if(!this->eatToken(TokenType::NUM)){
    Logger::error << "Parse Error: method parameter non-implemented" << std::endl;
    return new LocalVariableAST(idName);
  }
  
  return new LocalVariableAST(idName);
}

ExprAST* Parser::identifier() {
  std::string idName = this->_tok.str();

  // Consomme l'identifieur
  this->eatToken();

  // Simple identifer
  if (this->_tok != TokenType::LEFTP) {
    return new GlobaleVariableAST(idName);
  }

  // Appel de fonction
  // Consomme la parenthèse ouvrante
  this->eatToken();
  std::vector<ExprAST*> args;
  if (this->_tok != TokenType::RIGHTP) {
    while (true) {
      ExprAST* arg = this->expression();
      if (!arg) return nullptr; // Propage l'erreur
      args.push_back(arg);

      if (this->_tok == TokenType::RIGHTP) {
        break;
      }

      if (!this->eatToken(TokenType::COMMA)){
        Logger::error << "Parse Error: right parenthesis ')' expected" << std::endl;
        return nullptr;
      }
    }
  }

  // Consomme la parenthèse fermante
  this->eatToken();


  return new CallAST(idName, args);
}

StatementAST* Parser::ifstatement() {
  ExprAST *ifAST;
  BlocAST *thenAST;
  BlocAST* elseAST;
  // Consomme le token IF
  this->eatToken();
  
  // Consomme la parenthèse ouvrante
  if (!this->eatToken(TokenType::LEFTP)){
    Logger::error << "Parse Error: left parenthesis '(' expected" << std::endl;
    return nullptr;
  }
	
  // Parse la condition
  ifAST = this->expression();
  if (!ifAST) return nullptr;
	
  // Consomme la parenthèse fermante
  if (!this->eatToken(TokenType::RIGHTP)){
    Logger::error << "Parse Error: right parenthesis ')' expected" << std::endl;
    return nullptr;
  }
  
  if(this->_tok==TokenType::ENDL) this->eatToken();

  // Parse le bloc THEN
  thenAST = this->bloc();
  if (!thenAST) return nullptr;

  // Consomme le token ELSE
  if (this->_tok==TokenType::ELSE){
    this->eatToken();
	
    if(this->_tok==TokenType::ENDL) this->eatToken();
	
    // Parse le bloc ELSE
    elseAST = this->bloc();
    if (!elseAST) return nullptr;
	
	// Consomme le token ENDIF
    this->eatToken();
	
	return new IfAST(ifAST,thenAST, elseAST);
  }

  // Consomme le token ENDIF
  this->eatToken();
    
  return new IfAST(ifAST, thenAST);
}


ExprAST* Parser::expression() {
  ExprAST *LHS = this->primary();
  if (!LHS) return nullptr;
  
  if(this->_tok == TokenType::OP){
    return this->binOpRHS(LHS);
  }else{
    return LHS;
  }
}



ExprAST* Parser::binOpRHS(ExprAST *LHS) {
  
  std::string binOP = this->_tok.str();

  // On consomme l'opérateur	
  this->eatToken();

  ExprAST *RHS = this->expression();	//On récupére l'AST de la partie droite de l'opération
  if (!RHS){
    Logger::error << "Parse Error: 2nd operand expected after operator '" << binOP <<"'"<< std::endl;
    return nullptr;
  }

  return new OpAST(binOP, LHS, RHS);
}


ExprAST* Parser::primary() {
  switch (this->_tok.type()) {
  case TokenType::DOLLAR:
    return this->localVariable();
  case TokenType::DIAMOND:
    return this->persistantVariable();
  case TokenType::ID:
    return this->identifier();
  case TokenType::NUM:
    return this->number();
  case TokenType::LEFTP:
    return this->parenthesis();
  default:
    Logger::error << "Parse Error: unknown token when expecting an expression : " << this->_tok.type() << std::endl;
    return AST::Error<ExprAST>("unknown token when expecting an expression");
  }
}

PrototypeAST* Parser::prototype() {
  std::string fnName = this->_tok.str();
  if (!this->eatToken(TokenType::ID)) return nullptr;

  if (!this->eatToken(TokenType::LEFTP)) return nullptr;

  std::vector<std::string> argNames;
  while (this->_tok == TokenType::ID) {
    argNames.push_back(this->_tok.str());
    this->eatToken();
  }

  if (!this->eatToken(TokenType::RIGHTP)) return nullptr;

  return new PrototypeAST(fnName, argNames);
}


// FunctionAST* Parser::functionDef() {
  // this->eatToken();
  // PrototypeAST* proto = this->prototype();
  // if (!proto) return nullptr;

  // if (AST *E = this->expression()) {
      // return new FunctionAST(proto, E);
  // }
  // return nullptr;
// }


// PrototypeAST* Parser::externDef() {
  // this->eatToken();
  // return this->prototype();
// }



// FunctionAST* Parser::topLevelExpr() {
  // if (AST* E = this->expression()) {
    // PrototypeAST* proto = new PrototypeAST("", std::vector<std::string>());
    // return new FunctionAST(proto, E);
  // }
  // return nullptr;
// }

BlocAST* Parser::bloc() {
  Logger::info << "test:bloc"<<std::endl;
  std::vector<StatementAST*> statements;
  
  while(true){
    switch(this->_tok.type()){
	case TokenType::ENDF:
	case TokenType::ENDIF:
	case TokenType::ENDFOR:
	case TokenType::ENDWHILE:
	case TokenType::UNTIL:
	  return new BlocAST(statements); // sort du block
	case TokenType::ENDL: // ne lit pas les lignes vides
	  this->eatToken();
	  break;
	default:
	  StatementAST* statement = this->statement();
	  if(!statement) return nullptr;
	  
	  statements.push_back(statement);
	}
  }
  return nullptr;
}

StatementAST* Parser::statement() {
  Logger::info << "test:statement"<<std::endl;
  switch(this->_tok.type()){
  case TokenType::IF:
    return this->ifstatement();
  default:
    ExprAST* expr = this->expression();
	if(!expr) return nullptr;
	
	if(this->_tok!=TokenType::ENDF&&!this->eatToken(TokenType::ENDL)){
	  Logger::error << "Parse Error: ENDL expected after the expression"<< std::endl;
	  return nullptr;
	}
    return new StatementExprAST(expr);
  }
}


void Parser::parse() {
  if(this->_tok != TokenType::ENDF) {
    this->_ast = this->bloc();
  }
}

AST* Parser::ast() {
  return this->_ast;
}