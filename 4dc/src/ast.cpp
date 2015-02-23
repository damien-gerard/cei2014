#include "../include/ast.h"
#include <iostream>
#include <sstream>
#include "../include/util/logger.h"
#include "../include/util/util.h"
#include "../include/builder.h"
#include "../include/builtins.h"

using namespace std;
using namespace llvm;

const string NAMED_PREFIX_BEGIN   = "> ";
const string NAMED_PREFIX_MIDDLE  = "| ";
const string NAMED_PREFIX_END     = "  ";
const string PREFIX_BEGIN         = "+> ";
const string PREFIX_MIDDLE        = "|  ";
const string PREFIX_END           = "   ";
 
/**
 * AST
 */
AST::~AST() = default;

string AST::toString(const std::string& firstPrefix, const std::string& prefix) const
{
  return this->_toString(firstPrefix, prefix);
}

bool AST::_isVar() const
{
  return false;
}


ostream& operator<<(ostream& out, const AST& ast)
{
  return out << ast.toString("", "");
}

/**
 * BlocAST
 */
BlocAST::BlocAST(StatementAST* statement)
  : BlocAST({statement})
{}
BlocAST::BlocAST(std::initializer_list<StatementAST*> statements)
  : _statements(statements.begin(), statements.end())
{}
BlocAST::BlocAST(const std::vector<StatementAST*>& statements)
  : _statements(statements)
{}

BlocAST::~BlocAST()
{
  for (auto& statement : this->_statements) {
    delete statement;
  }
}

void BlocAST::_taggingPass(
              std::map<int, VarType>& argVars,
              std::map<std::string, VarType>& localVars,
              std::map<std::string, VarType>& globaleVars,
              std::map<std::string, VarType>& persistentVars
            )
{
  for (auto& statement : this->_statements) {
    statement->taggingPass(argVars, localVars, globaleVars, persistentVars);
  }
}

BasicBlock* BlocAST::Codegen(Builder& b, Function* f)
{
  return this->Codegen(b, "block", f);
}
BasicBlock* BlocAST::Codegen(Builder& b, const std::string& name, Function* f)
{
  assert(f != nullptr);
  BasicBlock* block = BasicBlock::Create(b.context(), name, f);
  assert(block != nullptr);
  b.currentBlock() = block;
  b.irbuilder().SetInsertPoint(block);
  for (auto& statement : this->_statements) {
    statement->Codegen(b);
  }
  //block = b.irbuilder().GetInsertBlock();
  return block;
}

string BlocAST::_toString(const string& firstPrefix, const string& prefix) const
{
  string nextFirstPrefix, nextPrefix;
  nextFirstPrefix = prefix + PREFIX_BEGIN;
  nextPrefix = prefix + PREFIX_MIDDLE;
  stringstream ss;
  ss << firstPrefix << "Bloc" << endl;
  int length = this->_statements.size();
  for (int i = 0 ; i < length ; ++i) {
    if (i + 1 == length) {
      nextPrefix = prefix + PREFIX_END;
    }
    ss << this->_statements[i]->toString(nextFirstPrefix, nextPrefix);
  }
  return ss.str();
}

/**
 * StatementAST
 */
StatementAST::~StatementAST() = default;

/**
 * StatementExprAST
 */
StatementExprAST::StatementExprAST(ExprAST* expr)
  : _expr(expr)
{}

StatementExprAST::~StatementExprAST()
{
  delete this->_expr;
}

void StatementExprAST::_taggingPass(
              std::map<int, VarType>& argVars,
              std::map<std::string, VarType>& localVars,
              std::map<std::string, VarType>& globaleVars,
              std::map<std::string, VarType>& persistentVars
            )
{
  this->_expr->taggingPass(argVars, localVars, globaleVars, persistentVars);
}

Value* StatementExprAST::Codegen(Builder& b)
{
  return this->_expr->Codegen(b);
}

string StatementExprAST::_toString(const string& firstPrefix, const string& prefix) const
{
  string nextFirstPrefix, nextPrefix;
  nextFirstPrefix = prefix + PREFIX_BEGIN;
  nextPrefix = prefix + PREFIX_END;
  stringstream ss;
  ss  << firstPrefix << "Statement::Expression" << endl
      << this->_expr->toString(nextFirstPrefix, nextPrefix);
  return ss.str();
}

/**
 * AffectationAST
 */
AffectationAST::AffectationAST(VariableAST* variableAST, ExprAST* expr)
  : _variableAST(variableAST), _expr(expr)
{}

AffectationAST::~AffectationAST()
{
  delete this->_variableAST;
  delete this->_expr;
}

void AffectationAST::_taggingPass(
              std::map<int, VarType>& argVars,
              std::map<std::string, VarType>& localVars,
              std::map<std::string, VarType>& globaleVars,
              std::map<std::string, VarType>& persistentVars
            )
{
  this->_variableAST->taggingPass(argVars, localVars, globaleVars, persistentVars);
  this->_expr->taggingPass(argVars, localVars, globaleVars, persistentVars);
  this->_variableAST->setType(this->_expr->getType());
}

Value* AffectationAST::Codegen(Builder& b)
{
  return nullptr;
}

string AffectationAST::_toString(const string& firstPrefix, const string& prefix) const
{
  string nextPrefix = prefix + NAMED_PREFIX_MIDDLE;
  stringstream ss;
  ss  << firstPrefix << "Statement::Affectation" << endl
      << this->_variableAST->toString(prefix + "+[Variable]" + NAMED_PREFIX_BEGIN, nextPrefix + "     ");

  nextPrefix = prefix + NAMED_PREFIX_END;
  ss  << this->_expr->toString(prefix + "+[EXPR]" + NAMED_PREFIX_BEGIN, nextPrefix + "       ");
  
  return ss.str();
}

/**
 * IfAST
 */
IfAST::IfAST(ExprAST* condAST, BlocAST* thenAST)
  : IfAST(condAST, thenAST, nullptr)
{}
IfAST::IfAST(ExprAST* condAST, BlocAST* thenAST, BlocAST* elseAST)
  : _condAST(condAST), _thenAST(thenAST), _elseAST(elseAST)
{}

IfAST::~IfAST()
{
  delete this->_condAST;
  delete this->_thenAST;
  if (this->_elseAST) delete this->_elseAST;
}

void IfAST::_taggingPass(
              std::map<int, VarType>& argVars,
              std::map<std::string, VarType>& localVars,
              std::map<std::string, VarType>& globaleVars,
              std::map<std::string, VarType>& persistentVars
            )
{
  this->_condAST->taggingPass(argVars, localVars, globaleVars, persistentVars);
  if(_condAST->getType() != VarType::BOOLEAN){
    Logger::error << "AST error: the condition of 'if' must be a boolean, not a " << _condAST->getType() << std::endl;
  }
  this->_thenAST->taggingPass(argVars, localVars, globaleVars, persistentVars);
  if (this->_elseAST) {
    this->_elseAST->taggingPass(argVars, localVars, globaleVars, persistentVars);
  }
}

Value* IfAST::Codegen(Builder& b)
{
  return nullptr;
}

string IfAST::_toString(const string& firstPrefix, const string& prefix) const
{
  string nextPrefix = prefix + NAMED_PREFIX_MIDDLE;
  stringstream ss;
  ss  << firstPrefix << "Statement::IF" << endl
      << this->_condAST->toString(prefix + "+[IF]" + NAMED_PREFIX_BEGIN, nextPrefix + "     ");

  if (!this->_elseAST) {
    nextPrefix = prefix + NAMED_PREFIX_END;
  }
  ss  << this->_thenAST->toString(prefix + "+[THEN]" + NAMED_PREFIX_BEGIN, nextPrefix + "       ");

  if (this->_elseAST) {
    nextPrefix = prefix + NAMED_PREFIX_END;
    ss<< this->_elseAST->toString(prefix + "+[ELSE]" + NAMED_PREFIX_BEGIN, nextPrefix + "       ");
  }
  return ss.str();
}

/*
Value* IfAST::Codegen(Builder& b)
{
  IRBuilder<>& builder = b.irbuilder();

  Value *condV = this->_condAST->Codegen(b);

  if (!condV) {
    return nullptr;
  }

  // Convert condition to a bool by comparing equal to 0.0.
  condV = builder.CreateFCmpONE(
            condV,
            ConstantFP::get(getGlobalContext(), APFloat(0.0)),
            "ifcond");

  Function *f = builder.GetInsertBlock()->getParent();

  // Create blocks for the then and else cases.  Insert the 'then' block at the
  // end of the function.
  BasicBlock *thenBB = BasicBlock::Create(getGlobalContext(), "then", f);
  BasicBlock *elseBB = BasicBlock::Create(getGlobalContext(), "else");
  BasicBlock *mergeBB = BasicBlock::Create(getGlobalContext(), "ifcont");

  builder.CreateCondBr(condV, thenBB, elseBB);

  // Emit then value.
  builder.SetInsertPoint(thenBB);

  Value *thenV = this->_thenAST->Codegen(b);
  if (!thenV) {
    return nullptr;
  }

  builder.CreateBr(mergeBB);
  // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
  thenBB = builder.GetInsertBlock();

  // Emit else block.
  f->getBasicBlockList().push_back(elseBB);
  builder.SetInsertPoint(elseBB);

  Value *elseV = this->_elseAST->Codegen(b);
  if (!elseV) {
    return nullptr;
  }

  builder.CreateBr(mergeBB);
  // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
  elseBB = builder.GetInsertBlock();

  // Emit merge block.
  f->getBasicBlockList().push_back(mergeBB);
  builder.SetInsertPoint(mergeBB);
  PHINode *PN = builder.CreatePHI(Type::getDoubleTy(getGlobalContext()), 2,
                                  "iftmp");

  PN->addIncoming(thenV, thenBB);
  PN->addIncoming(elseV, elseBB);
  return PN;
}
*/

/**
 * ForAST
 */
ForAST::ForAST(VariableAST* variableAST, ExprAST* beginAST, ExprAST* endAST, ExprAST* incrementAST, BlocAST* loopAST)
  : _variableAST(variableAST), _beginAST(beginAST), _endAST(endAST), _incrementAST(incrementAST), _loopAST(loopAST)
{}

ForAST::~ForAST()
{
  delete this->_variableAST;
  delete this->_beginAST;
  delete this->_endAST;
  delete this->_incrementAST;
  delete this->_loopAST;
}


void ForAST::_taggingPass(
              std::map<int, VarType>& argVars,
              std::map<std::string, VarType>& localVars,
              std::map<std::string, VarType>& globaleVars,
              std::map<std::string, VarType>& persistentVars
            )
{
  this->_variableAST->taggingPass(argVars, localVars, globaleVars, persistentVars);
  this->_variableAST->setType(VarType::INT);
  this->_beginAST->taggingPass(argVars, localVars, globaleVars, persistentVars);
  if(_beginAST->getType() != VarType::INT){
    Logger::error << "AST error: the begin exression of 'for' must be an int, not a " << _beginAST->getType() << std::endl;
  }
  this->_endAST->taggingPass(argVars, localVars, globaleVars, persistentVars);
  if(_endAST->getType() != VarType::INT){
    Logger::error << "AST error: the end exression of 'for' must be an int, not a " << _endAST->getType() << std::endl;
  }
  this->_incrementAST->taggingPass(argVars, localVars, globaleVars, persistentVars);
  if(_incrementAST->getType() != VarType::INT){
    Logger::error << "AST error: the increment exression of 'for' must be an int, not a " << _incrementAST->getType() << std::endl;
  }
}

Value* ForAST::Codegen(Builder& b)
{
  return nullptr;
}

string ForAST::_toString(const string& firstPrefix, const string& prefix) const
{
  string nextFirstPrefix, nextPrefix;
  nextFirstPrefix = prefix + PREFIX_BEGIN;
  nextPrefix = prefix + PREFIX_MIDDLE;
  stringstream ss;
  ss  << firstPrefix << "Statement::FOR" << endl
      << this->_variableAST->toString(nextFirstPrefix, nextPrefix)
      << this->_beginAST->toString(nextFirstPrefix, nextPrefix)
      << this->_endAST ->toString(nextFirstPrefix, nextPrefix)
      << this->_incrementAST  ->toString(nextFirstPrefix, nextPrefix);
  nextPrefix = prefix + PREFIX_END;
  ss  << this->_loopAST ->toString(nextFirstPrefix, nextPrefix);
  return ss.str();
}


/**
 * WhileAST
 */
WhileAST::WhileAST(ExprAST* condAST, BlocAST* loopAST)
  : _condAST(condAST), _loopAST(loopAST)
{}

WhileAST::~WhileAST()
{
  delete this->_condAST;
  delete this->_loopAST;
}

void WhileAST::_taggingPass(
              std::map<int, VarType>& argVars,
              std::map<std::string, VarType>& localVars,
              std::map<std::string, VarType>& globaleVars,
              std::map<std::string, VarType>& persistentVars
            )
{
  this->_condAST->taggingPass(argVars, localVars, globaleVars, persistentVars);
  if(_condAST->getType() != VarType::BOOLEAN){
    Logger::error << "AST error: the condition of 'while' must be a boolean, not a " << _condAST->getType() << std::endl;
  }
  this->_loopAST->taggingPass(argVars, localVars, globaleVars, persistentVars);
}

Value* WhileAST::Codegen(Builder& b)
{
  return nullptr;
}

string WhileAST::_toString(const string& firstPrefix, const string& prefix) const
{
  string nextPrefix = prefix + PREFIX_MIDDLE;
  stringstream ss;
  ss  << firstPrefix << "Statement::WHILE" << endl
      << this->_condAST ->toString(prefix + "+[WHILE]" + NAMED_PREFIX_BEGIN, nextPrefix + "        ");
  nextPrefix = prefix + PREFIX_END;
  ss  << this->_loopAST ->toString(prefix + "+[DO]" + NAMED_PREFIX_BEGIN, nextPrefix + "     ");
  return ss.str();
}

/**
 * RepeatAST
 */
RepeatAST::RepeatAST(ExprAST* condAST, BlocAST* loopAST)
  : _condAST(condAST), _loopAST(loopAST)
{}

RepeatAST::~RepeatAST()
{
  delete this->_condAST;
  delete this->_loopAST;
}

void RepeatAST::_taggingPass(
              std::map<int, VarType>& argVars,
              std::map<std::string, VarType>& localVars,
              std::map<std::string, VarType>& globaleVars,
              std::map<std::string, VarType>& persistentVars
            )
{
  this->_loopAST->taggingPass(argVars, localVars, globaleVars, persistentVars);
  this->_condAST->taggingPass(argVars, localVars, globaleVars, persistentVars);
  if(_condAST->getType() != VarType::BOOLEAN){
    Logger::error << "AST error: the condition of 'while' must be a boolean, not a " << _condAST->getType() << std::endl;
  }
}

Value* RepeatAST::Codegen(Builder& b)
{
  return nullptr;
}

string RepeatAST::_toString(const string& firstPrefix, const string& prefix) const
{
  string nextPrefix = prefix + PREFIX_MIDDLE;
  stringstream ss;
  ss  << firstPrefix << "Statement::RepeatAST" << endl
      << this->_loopAST ->toString(prefix + "+[REPEAT]" + NAMED_PREFIX_BEGIN, nextPrefix + "     ");
  nextPrefix = prefix + PREFIX_END;
  ss  << this->_condAST ->toString(prefix + "+[UNTIL]" + NAMED_PREFIX_BEGIN, nextPrefix + "        ");
  return ss.str();
}

/**
 * ExprAST
 */
ExprAST::~ExprAST() = default;

VarType ExprAST::getType() const{
  return this->_vtype;
}

void ExprAST::setType(VarType vtype){
  this->_vtype = vtype;
}


/**
 * LiteralAST
 */
LiteralAST::LiteralAST(const std::string& val, VarType vtype)
  : _val(val)
{
  this->_vtype = vtype;
}
LiteralAST::~LiteralAST() = default;

void LiteralAST::_taggingPass(
              std::map<int, VarType>& argVars,
              std::map<std::string, VarType>& localVars,
              std::map<std::string, VarType>& globaleVars,
              std::map<std::string, VarType>& persistentVars
            )
{}

string LiteralAST::_toString(const string& firstPrefix, const string& prefix) const
{
  stringstream ss;
  ss  << firstPrefix << "Expression::Literal " << this->_val << endl;
  return ss.str();
}

Value* LiteralAST::Codegen(Builder& b)
{
  return ConstantInt::get(b.context(), APInt(32, Util::str2long(this->_val), true));
}


/**
 * VariableAST
 */
VariableAST::VariableAST(){
  this->_vtype = VarType::NOTDEFINE;
}
 
VariableAST::~VariableAST() = default;

bool VariableAST::_isVar() const
{
  return true;
}

/*
Value* VariableAST::Codegen(Builder& b)
{
  Value* V = b.namedValues()[this->_name];
  return V ? V : AST::Error<Value>("Unknown variable name");
}
*/
/**
 * LocalVariableAST
 */
LocalVariableAST::LocalVariableAST(const std::string& name)
  : _name(name)
{}
LocalVariableAST::~LocalVariableAST() = default;

void LocalVariableAST::_taggingPass(
              std::map<int, VarType>& argVars,
              std::map<std::string, VarType>& localVars,
              std::map<std::string, VarType>& globaleVars,
              std::map<std::string, VarType>& persistentVars
            )
{
  int num = 0;
  if (strConvert(this->_name, num) && num >= 0) {
    argVars[num] = this->getType();
  } else {
    localVars[this->_name] = this->getType();
  }
}

string LocalVariableAST::_toString(const string& firstPrefix, const string& prefix) const
{
  stringstream ss;
  ss  << firstPrefix << "Expression::Variable $" << this->_name << endl;
  return ss.str();
}

Value* LocalVariableAST::Codegen(Builder& b)
{
  Value* V = b.localVars()[this->_name];
  return V ? V : AST::Error<Value>("Unknown local variable name");
}

/**
 * GlobaleVariableAST
 */
GlobaleVariableAST::GlobaleVariableAST(const std::string& name)
  : _name(name)
{}
GlobaleVariableAST::~GlobaleVariableAST() = default;

void GlobaleVariableAST::_taggingPass(
              std::map<int, VarType>& argVars,
              std::map<std::string, VarType>& localVars,
              std::map<std::string, VarType>& globaleVars,
              std::map<std::string, VarType>& persistentVars
            )
{
  globaleVars[this->_name] = this->getType();
}

string GlobaleVariableAST::_toString(const string& firstPrefix, const string& prefix) const
{
  stringstream ss;
  ss  << firstPrefix << "Expression::Variable " << this->_name << endl;
  return ss.str();
}

Value* GlobaleVariableAST::Codegen(Builder& b)
{
  Value* V = b.globalVars()[this->_name];
  return V ? V : AST::Error<Value>("Unknown global variable name");
}


/**
 * PersistentVariableAST
 */
PersistentVariableAST::PersistentVariableAST(const std::string& name)
  : _name(name)
{}
PersistentVariableAST::~PersistentVariableAST() = default;

void PersistentVariableAST::_taggingPass(
              std::map<int, VarType>& argVars,
              std::map<std::string, VarType>& localVars,
              std::map<std::string, VarType>& globaleVars,
              std::map<std::string, VarType>& persistentVars
            )
{
  persistentVars[this->_name] = this->getType();
}

string PersistentVariableAST::_toString(const string& firstPrefix, const string& prefix) const
{
  stringstream ss;
  ss  << firstPrefix << "Expression::Variable <>" << this->_name << endl;
  return ss.str();
}

Value* PersistentVariableAST::Codegen(Builder& b)
{
  Value* V = b.persistentVars()[this->_name];
  return V ? V : AST::Error<Value>("Unknown persistent variable name");
}


/**
 * UniOpAST
 */
UniOpAST::UniOpAST(const std::string& op, ExprAST* expr)
  : _str(op), _expr(expr)
{}
UniOpAST::~UniOpAST()
{
  delete this->_expr;
}

void UniOpAST::_taggingPass(
              std::map<int, VarType>& argVars,
              std::map<std::string, VarType>& localVars,
              std::map<std::string, VarType>& globaleVars,
              std::map<std::string, VarType>& persistentVars
            )
{
  this->_expr->taggingPass(argVars, localVars, globaleVars, persistentVars);
  this->setType(this->_expr->getType());
}

string UniOpAST::_toString(const string& firstPrefix, const string& prefix) const
{
  string nextFirstPrefix, nextPrefix;
  nextFirstPrefix = prefix + PREFIX_BEGIN;
  nextPrefix = prefix + PREFIX_MIDDLE;
  stringstream ss;
  ss  << firstPrefix << "Expression::UniOP " << this->_str << endl;
  nextPrefix = prefix + PREFIX_END;
  ss  << this->_expr->toString(nextFirstPrefix, nextPrefix);
  return ss.str();
}

Value* UniOpAST::Codegen(Builder& b)
{

  Value *L = this->_expr->Codegen(b);
  if (!L) return nullptr;
  
  if (this->_str == "+") return L;
  if (this->_str == "-") return b.irbuilder().CreateNeg(L, "negtmp");
  return AST::Error<Value>("invalid unary operator");
}

/**
 * BinOpAST
 */
BinOpAST::BinOpAST(const std::string& op, ExprAST* lhs, ExprAST* rhs)
  : _str(op), _lhs(lhs), _rhs(rhs)
{}
BinOpAST::~BinOpAST()
{
  delete this->_lhs;
  delete this->_rhs;
}

void BinOpAST::_taggingPass(
              std::map<int, VarType>& argVars,
              std::map<std::string, VarType>& localVars,
              std::map<std::string, VarType>& globaleVars,
              std::map<std::string, VarType>& persistentVars
            )
{
  VarType opType = VarType::NOTDEFINE;
  this->_lhs->taggingPass(argVars, localVars, globaleVars, persistentVars);
  this->_rhs->taggingPass(argVars, localVars, globaleVars, persistentVars);
  if(this->_lhs->getType() != VarType::NOTDEFINE){
    if(this->_rhs->getType()!=VarType::NOTDEFINE && this->_lhs->getType() != this->_rhs->getType() ){
      Logger::error << "AST error: implicite cast of " << this->_rhs->getType() << " in " << this->_lhs->getType() << std::endl;
    }
    opType = this->_lhs->getType();
  }else{
    opType = this->_rhs->getType();
  }   

  if(this->_str != "<" &&
     this->_str != ">" &&
     this->_str != "=" &&
     this->_str != "<=" &&
     this->_str != ">=" &&
     this->_str != "&" &&
     this->_str != "|")
  {
    this->setType(opType);
  }else{
    this->setType(VarType::BOOLEAN);
  }
}

string BinOpAST::_toString(const string& firstPrefix, const string& prefix) const
{
  string nextFirstPrefix, nextPrefix;
  nextFirstPrefix = prefix + PREFIX_BEGIN;
  nextPrefix = prefix + PREFIX_MIDDLE;
  stringstream ss;
  ss  << firstPrefix << "Expression::BinOP " << this->_str << endl
      << this->_lhs->toString(nextFirstPrefix, nextPrefix);
  nextPrefix = prefix + PREFIX_END;
  ss  << this->_rhs->toString(nextFirstPrefix, nextPrefix);
  return ss.str();
}

Value* BinOpAST::Codegen(Builder& b)
{

  Value *L = this->_lhs->Codegen(b);
  Value *R = this->_rhs->Codegen(b);
  if (!L || !R) {
    return nullptr;
  }
  
  auto* BB = b.currentBlock();
  assert(BB != nullptr);
  // Opérations arithmétiques
  if (this->_str == "+") return b.irbuilder().CreateAdd(L, R, "addtmp", BB);
  if (this->_str == "-") return b.irbuilder().CreateSub(L, R, "subtmp", BB);
  if (this->_str == "*") return b.irbuilder().CreateMul(L, R, "multmp", BB);
  if (this->_str == "/") return b.irbuilder().CreateSDiv(L, R, "divtmp", BB);
  
  // Comparaisons
  if (this->_str == "<")  return b.irbuilder().CreateICmpSLT(L, R, "lttmp");
  if (this->_str == "<=") return b.irbuilder().CreateICmpSLE(L, R, "letmp");
  if (this->_str == ">")  return b.irbuilder().CreateICmpSGT(L, R, "gttmp");
  if (this->_str == ">=") return b.irbuilder().CreateICmpSGE(L, R, "getmp");
  
  // Opérateurs logiques
  if (this->_str == "&&")  return b.irbuilder().CreateAnd(L, R, "ortmp");
  if (this->_str == "||") return b.irbuilder().CreateOr(L, R, "andtmp");
  if (this->_str == "^")  return b.irbuilder().CreateXor(L, R, "xortmp");
  return AST::Error<Value>("invalid binary operator");
}


/**
 * CallAST
 */
CallAST::CallAST(const std::string& name, const std::vector<ExprAST*>& args)
  : _name(name), _args(args)
{}
CallAST::~CallAST()
{
  auto it = this->_args.begin();
  auto endIT = this->_args.end();
  for (; it != endIT ; ++it) {
    delete *it;
  }
  this->_args.clear();
}

void CallAST::_taggingPass(
              std::map<int, VarType>& argVars,
              std::map<std::string, VarType>& localVars,
              std::map<std::string, VarType>& globaleVars,
              std::map<std::string, VarType>& persistentVars
            )
{
  for (auto& arg : this->_args) {
    arg->taggingPass(argVars, localVars, globaleVars, persistentVars);
  }
}

string CallAST::_toString(const string& firstPrefix, const string& prefix) const
{
  string nextFirstPrefix, nextPrefix;
  nextFirstPrefix = prefix + PREFIX_BEGIN;
  nextPrefix = prefix + PREFIX_MIDDLE;
  stringstream ss;
  ss << firstPrefix << "Expression::Call " << this->_name << endl;
  int length = this->_args.size();
  for (int i = 0; i < length ; ++i) {
    if (i + 1 == length) {
      nextPrefix = prefix + PREFIX_END;
    }
    ss << this->_args[i]->toString(nextFirstPrefix, nextPrefix);
  }
  return ss.str();
}

Value* CallAST::Codegen(Builder& b)
{
  string& name = this->_name;
  
  if (Builtin::getList().count(name)) {
    name = Builtin::getList()[name]->getName();
  }
  // Look up the name in the global module table.
  Function *CalleeF = b.module().getFunction(name);
  if (CalleeF == 0) {
    return AST::Error<Value>("Unknown function referenced");
  }

  // If argument mismatch error.
  if (CalleeF->arg_size() != this->_args.size()) {
    return AST::Error<Value>("Incorrect # arguments passed");
  }

  std::vector<Value*> ArgsV;

  auto it = this->_args.begin();
  auto endIT = this->_args.end();
  for (; it != endIT; ++it) {
    ArgsV.push_back((*it)->Codegen(b));
    if (!ArgsV.back()) {
      return nullptr;
    }
  }

  return b.irbuilder().CreateCall(CalleeF, ArgsV, "calltmp");
}


/**
 * DefinitionAST
 */

void DefinitionAST::_taggingPass(
              std::map<int, VarType>& argVars,
              std::map<std::string, VarType>& localVars,
              std::map<std::string, VarType>& globaleVars,
              std::map<std::string, VarType>& persistentVars
            )
{}

/**
 * PrototypeAST
 */
PrototypeAST::PrototypeAST(const Builtin& builtin)
  : PrototypeAST(builtin.getName(), builtin.getArgsName())
{}
PrototypeAST::PrototypeAST(const std::string& name, const std::vector<std::string>& args)
  : _name(name), _args(args)
{}
PrototypeAST::~PrototypeAST() = default;

string PrototypeAST::_toString(const string& firstPrefix, const string& prefix) const
{
  string nextFirstPrefix, nextPrefix;
  nextFirstPrefix = prefix + PREFIX_BEGIN;
  nextPrefix = prefix + PREFIX_END;
  stringstream ss;
  ss  << firstPrefix << "Definition::Prototype " << this->_name << "(";
  Util::join(ss, this->_args, ", ");
  ss  << ")" << endl;
  return ss.str();
}

Function* PrototypeAST::Codegen(Builder& b)
{
  string name = this->_name;
  Type* retType = Type::getInt32Ty(b.context());
  
  // Make the function type:  double(double,double) etc.
  std::vector<Type*> Args(this->_args.size(),
                             Type::getInt32Ty(b.context()));
  FunctionType *FT = FunctionType::get(retType,
                                       Args, false);

  Function *F = Function::Create(FT, Function::ExternalLinkage, name, &b.module());
  assert(F != nullptr);


  // If F conflicted, there was already something named 'Name'.  If it has a
  // body, don't allow redefinition or reextern.
  if (F->getName() != name) {
    // Delete the one we just made and get the existing one.
    F->eraseFromParent();
    F = b.module().getFunction(name);

    // If F already has a body, reject this.
    if (!F->empty()) {
      return AST::Error<Function>("redefinition of function");
    }

    // If F took a different number of args, reject.
    if (F->arg_size() != this->_args.size()) {
      return AST::Error<Function>("redefinition of function with different # args");
    }
  }
  // Set names for all arguments.
  auto argsi  = this->_args.begin();
  auto endi   = this->_args.end();
  auto fargsi = F->arg_begin();
  for (; argsi != endi; ++argsi, ++fargsi) {
    fargsi->setName(*argsi);

    // Add arguments to variable symbol table.
    b.localVars()[*argsi] = fargsi;
  }
  F->setCallingConv(llvm::CallingConv::C);
  return F;
}


/**
 * FunctionAST
 */
FunctionAST::FunctionAST(PrototypeAST* proto, BlocAST* body)
  : _proto(proto), _body(body)
{}
FunctionAST::~FunctionAST()
{
  delete this->_body;
}

string FunctionAST::_toString(const string& firstPrefix, const string& prefix) const
{
  string nextFirstPrefix, nextPrefix;
  nextFirstPrefix = prefix + PREFIX_BEGIN;
  nextPrefix = prefix + PREFIX_MIDDLE;
  stringstream ss;
  ss  << firstPrefix << "Definition::Function " << endl
      << this->_proto->toString(nextFirstPrefix, nextPrefix);
  nextPrefix = prefix + PREFIX_END;
  ss  << this->_body->toString(nextFirstPrefix, nextPrefix);
  return ss.str();
}

Function* FunctionAST::Codegen(Builder& b)
{
  b.localVars().clear();

  Function *f = this->_proto->Codegen(b);
  if (!f) {
    return nullptr;
  }
  
  BasicBlock *block = this->_body->Codegen(b, f);
  assert(block != nullptr);
  if (block) {
    Logger::debug << "block size: " << block->size() << endl;
    b.irbuilder().SetInsertPoint(block);
    // Finish off the function.
    Logger::debug << "block size: " << block->size() << endl;
    b.irbuilder().CreateRet(ConstantInt::get(Type::getInt32Ty(b.context()), 0));
    
    Logger::debug << "Function compiled. Checking..." << endl;
    // Validate the generated code, checking for consistency.
    verifyFunction(*f);
    Logger::debug << "Function checked." << endl;

    //b.optimize(f);

    return f;
  }

  return nullptr;
/*
  // Create a new basic block to start insertion into.
  BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", f);
  b.irbuilder().SetInsertPoint(BB);
  Value *RetVal = this->_body->Codegen(b);
  if (RetVal) {
    // Finish off the function.
    b.irbuilder().CreateRet(RetVal);

    // Validate the generated code, checking for consistency.
    verifyFunction(*f);

    b.optimize(f);

    return f;
  }

  // Error reading body, remove function.
  f->eraseFromParent();
  return nullptr;
  */
}

