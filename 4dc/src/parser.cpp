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
    ss << "Unexpected token " << this->_tok.type() << "(\"" << this->_tok.str() << "\")"<< ", expected " << type;
    Logger::error << "Parse Error: " << ss.str() << std::endl;
    return AST::Error<Token>(ss.str());
  }
  this->_tok = this->_lexer.nextToken();
  return &this->_tok;
}


/***
 * Parser Methods
 */
ExprAST* Parser::literal() {
  ExprAST* result = new LiteralAST(this->_tok.str());
  this->eatToken();
  return result;
}


ExprAST* Parser::parenthesis() {
  // Consomme la parenthèse ouvrante
  this->eatToken();
  ExprAST* V = this->expression();

  if (!V) return nullptr; // Propage l'erreur

  if (!this->eatToken(TokenType::RIGHTP)) return nullptr;

  return V;
}
VariableAST* Parser::persistantVariable() {
  // Consomme le diamand
  this->eatToken();

  std::string idName = this->_tok.str();
  
  // Consomme l'identifiant
  this->eatToken(TokenType::ID);
  
  return new PersistentVariableAST(idName);
}

VariableAST* Parser::localVariable() {
  // Consomme le dollar
  this->eatToken();
  
  std::string idName = this->_tok.str();
  // Consomme l'identifiant
  if(this->_tok == TokenType::NUM){
    std::string number = this->_tok.str();
    this->eatToken();
    Logger::warning << "Parse Warning: method parameter $" << number << " non-implemented" << std::endl;
    return new LocalVariableAST(idName);
  }
  if(!this->eatToken(TokenType::ID)) return nullptr;
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
  return this->callFunction(idName);

}

ExprAST* Parser::callFunction(std::string functionName){

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

      if (!this->eatToken(TokenType::SEMICOL)){
        Logger::error << "Parse Error: right parenthesis ')' expected" << std::endl;
        return nullptr;
      }
    }
  }

  // Consomme la parenthèse fermante
  this->eatToken();


  return new CallAST(functionName, args);  
}


///////////////////
/// Statements ////
///////////////////

BlocAST* Parser::bloc() {

  std::vector<StatementAST*> statements;
  
  while(true){
    switch(this->_tok.type()){
      case TokenType::ENDF:
      case TokenType::ELSE:
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
  
  switch(this->_tok.type()){
  case TokenType::IF:
    return this->ifstatement();
  case TokenType::FOR:
    return this->forstatement();
  case TokenType::WHILE:
    return this->whilestatement();
  case TokenType::REPEAT:
    return this->repeatstatement();
  //AFFECTATION OR EXPRESSION STATEMENTS
  default:
    ExprAST* expr = this->expression();
    if(!expr) return nullptr;
    
    if(this->_tok == TokenType::AFFECT){
      //verifie que expr est une variable.
      //@todo
      
      // On consumme l'affectation
      this->eatToken();
      
      ExprAST* affectedExpr = this->expression();      
      if(!affectedExpr) return nullptr;
      
      return new AffectationAST((VariableAST*) expr, affectedExpr);
    }
    
    //sinon l'expression doit se terminer a la fin de la ligne ou du fichier
    if(this->_tok!=TokenType::ENDF && !this->eatToken(TokenType::ENDL)) return nullptr;
    
    return new StatementExprAST(expr);
  }
}

/////// IF ////////

StatementAST* Parser::ifstatement() {
  ExprAST *ifAST;
  BlocAST *thenAST;
  BlocAST* elseAST;
  // Consomme le token IF
  this->eatToken();
  
  // Consomme la parenthèse ouvrante
  if (!this->eatToken(TokenType::LEFTP)) return nullptr;
  
  // Parse la condition
  ifAST = this->expression();
  if (!ifAST) return nullptr;
  
  // Consomme la parenthèse fermante
  if (!this->eatToken(TokenType::RIGHTP)) return nullptr;
  
  if(this->_tok == TokenType::ENDL) this->eatToken();

  // Parse le bloc THEN
  thenAST = this->bloc();
  if (!thenAST) return nullptr;

  // Consomme le token ELSE
  if (this->_tok == TokenType::ELSE){
    this->eatToken();
  
    if(this->_tok == TokenType::ENDL) this->eatToken();
  
    // Parse le bloc ELSE
    elseAST = this->bloc();
    if (!elseAST) return nullptr;
  
  // Consomme le token ENDIF
    this->eatToken(TokenType::ENDIF);
  if(this->_tok == TokenType::ENDL) this->eatToken();
  
  return new IfAST(ifAST,thenAST, elseAST);
  }

  // Consomme le token ENDIF
  this->eatToken(TokenType::ENDIF);
  if(this->_tok == TokenType::ENDL) this->eatToken();
    
  return new IfAST(ifAST, thenAST);
}

///// FOR /////

StatementAST* Parser::forstatement() {
  VariableAST * variableAST;
  ExprAST *beginAST;
  ExprAST *endAST;
  ExprAST *incrementAST;
  BlocAST *bodyAST;
  // Consomme le token For
  this->eatToken();
  
  // Consomme la parenthèse ouvrante
  if (!this->eatToken(TokenType::LEFTP)) return nullptr;

  
  // Parse la variable index
  std::string indexName = this->_tok.str();
  TokenType indexType = this->_tok.type();
  this->eatToken();
  switch (indexType) {
  case TokenType::DOLLAR:
    indexName = this->_tok.str();
    if(this->eatToken(TokenType::ID)) variableAST = new LocalVariableAST(indexName);
  break;
  case TokenType::DIAMOND:
    indexName = this->_tok.str();
    if(this->eatToken(TokenType::ID)) variableAST = new PersistentVariableAST(indexName);
  break;
  case TokenType::ID:
    variableAST = new GlobaleVariableAST(indexName);
  break;
  default:
    break;
  }
  
  if(!variableAST){
    Logger::error << "Parse Error: variable token expected and not : "<< indexType << std::endl;
    variableAST =  AST::Error<GlobaleVariableAST>("variable token expected in the for arguements");
  return nullptr;
  }
  
  //Consomme le semi-colon
  if(!this->eatToken(TokenType::SEMICOL)) return nullptr;

  // Parse l'expression de debut de boucle
  beginAST = this->expression();
  if (!beginAST) return nullptr;

  //Consomme le semi-colon
  if(!this->eatToken(TokenType::SEMICOL)) return nullptr;

  // Parse l'expression de fin de boucle
  endAST = this->expression();
  if (!beginAST) return nullptr;

  if(this->_tok == TokenType::SEMICOL){  // y a t'il une expression pour l'incrementation ?
    //Consomme le semi-colon
  this->eatToken();
    
    // Parse l'expression de debut de boucle
    incrementAST = this->expression();
    if (!incrementAST) return nullptr;
  }else{
    incrementAST= new LiteralAST("1");
  }
  
  // Consomme la parenthèse fermante
  if (!this->eatToken(TokenType::RIGHTP)) return nullptr;
  
  if(this->_tok == TokenType::ENDL) this->eatToken();

  // Parse le bloc THEN
  bodyAST = this->bloc();
  if (!bodyAST) return nullptr;

  // Consomme le token ENDFOR
  this->eatToken(TokenType::ENDFOR);
  if(this->_tok == TokenType::ENDL) this->eatToken();
    
  return new ForAST(variableAST, beginAST, endAST, incrementAST, bodyAST);
}

////// WHILE ///////

StatementAST* Parser::whilestatement() {
  ExprAST *whileAST;
  BlocAST *loopAST;
  
  // Consomme le token WHILE
  this->eatToken();
  
  // Consomme la parenthèse ouvrante
  if (!this->eatToken(TokenType::LEFTP)) return nullptr;
  
  // Parse la condition
  whileAST = this->expression();
  if (!whileAST) return nullptr;
  
  // Consomme la parenthèse fermante
  if (!this->eatToken(TokenType::RIGHTP)) return nullptr;
  
  if(this->_tok == TokenType::ENDL) this->eatToken();

  // Parse le bloc de la boucle
  loopAST = this->bloc();
  if (!loopAST) return nullptr;

  // Consomme le token ENDWHILE
  this->eatToken(TokenType::ENDWHILE);
  if(this->_tok == TokenType::ENDL) this->eatToken();
    
  return new WhileAST(whileAST, loopAST);
}

////// REPEAT ///////

StatementAST* Parser::repeatstatement() {
  ExprAST *untilAST;
  BlocAST *loopAST;
  
  // Consomme le token REPEAT
  this->eatToken();
  
  if(this->_tok == TokenType::ENDL) this->eatToken();

  // Parse le bloc de la boucle
  loopAST = this->bloc();
  if (!loopAST) return nullptr;

  // Consomme le token UNTIL
  this->eatToken(TokenType::UNTIL);
  
  // Consomme la parenthèse ouvrante
  if (!this->eatToken(TokenType::LEFTP)) return nullptr;
  
  // Parse la condition
  untilAST = this->expression();
  if (!untilAST) return nullptr;
  
  // Consomme la parenthèse fermante
  if (!this->eatToken(TokenType::RIGHTP)) return nullptr;
  
  if(this->_tok == TokenType::ENDL) this->eatToken();
    
  return new RepeatAST(untilAST, loopAST);
}

//////////////////
/// Expression ///
//////////////////

ExprAST* Parser::expression() {
  
  ExprAST* LHS = this->uniOpExpr();
  if (!LHS) return nullptr;
  
  if(this->_tok == TokenType::OP){
    
    ExprAST *leftAccumulatorExpr = this->binOpRHS(LHS);
  while(this->_tok == TokenType::OP){
     leftAccumulatorExpr = this->binOpRHS(leftAccumulatorExpr);
  }
  return leftAccumulatorExpr;
  }else{
    return LHS;
  }
}

ExprAST* Parser::uniOpExpr(){
  if(this->_tok == TokenType::OP){
    std::string uniOP = this->_tok.str();
  
  // On consomme l'opérateur  
    this->eatToken();
  ExprAST *expr = this->primary();  //On récupére l'AST de la partie droite de l'opération
    if (!expr){
      Logger::error << "Parse Error: primary expression expected after operator '" << uniOP <<"'"<< std::endl;
      return nullptr;
    }
  return new UniOpAST(uniOP,expr);
  }
  return this->primary();
}

ExprAST* Parser::binOpRHS(ExprAST *LHS) {
  
  std::string binOP = this->_tok.str();

  // On consomme l'opérateur  
  this->eatToken();

  ExprAST *RHS = this->uniOpExpr();  //On récupére l'AST de la partie droite de l'opération
  if (!RHS){
    Logger::error << "Parse Error: 2nd operand expected after operator '" << binOP <<"'"<< std::endl;
    return nullptr;
  }

  return new BinOpAST(binOP, LHS, RHS);
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
  case TokenType::STRING:
    return this->literal();
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




void Parser::parse() {
  if(this->_tok != TokenType::ENDF) {
    this->_ast = this->bloc();
  }
}

AST* Parser::ast() {
  return this->_ast;
}
