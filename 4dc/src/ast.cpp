#include "../include/ast.h"
#include <iostream>
#include <sstream>
#include "../include/util/logger.h"
#include "../include/util/util.h"
#include "../include/builder.h"
#include "../include/builtins.h"
#include "../include/functionsignature.h"

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
  BasicBlock* block = b.currentBlock();
  if (block == nullptr) {
    assert(f != nullptr);
    block = BasicBlock::Create(b.context(), name, f);
    b.currentBlock() = block;
  }
  assert(block != nullptr);
  b.irbuilder().SetInsertPoint(block);
  for (auto& statement : this->_statements) {
    if (!statement->Codegen(b)) return nullptr;
  }
  block = b.irbuilder().GetInsertBlock();
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

bool StatementExprAST::Codegen(Builder& b)
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

bool AffectationAST::Codegen(Builder& b)
{
  assert(this->_variableAST != nullptr);
  assert(this->_expr != nullptr);
  return this->_variableAST->CodegenMute(b, this->_expr->Codegen(b));
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

bool IfAST::Codegen(Builder& b)
{
  IRBuilder<>& builder = b.irbuilder();

  Value *condV = this->_condAST->Codegen(b);

  if (!condV) {
    return false;
  }

  Function *f = builder.GetInsertBlock()->getParent();

  // Create blocks for the then and else cases.  Insert the 'then' block at the
  // end of the function.
  BasicBlock *thenBB = BasicBlock::Create(b.context(), "if.then", f);
  BasicBlock *elseBB = BasicBlock::Create(b.context(), "if.else");
  BasicBlock *mergeBB = BasicBlock::Create(b.context(), "if.cont");

  builder.CreateCondBr(condV, thenBB, elseBB);

  // Emit then value.
  builder.SetInsertPoint(thenBB);
  b.currentBlock() = thenBB;

  Value *thenV = this->_thenAST->Codegen(b);
  if (!thenV) {
    return false;
  }

  builder.CreateBr(mergeBB);
  // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
  thenBB = builder.GetInsertBlock();

  // Emit else block.
  f->getBasicBlockList().push_back(elseBB);
  builder.SetInsertPoint(elseBB);
  b.currentBlock() = elseBB;

  if (this->_elseAST && !this->_elseAST->Codegen(b)) {
    return false;
  }

  builder.CreateBr(mergeBB);
  // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
  elseBB = builder.GetInsertBlock();

  // Emit merge block.
  f->getBasicBlockList().push_back(mergeBB);
  builder.SetInsertPoint(mergeBB);
  b.currentBlock() = mergeBB;
  return true;
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
    if(_beginAST->getType() == VarType::NOTDEFINE){
      _beginAST->setType(VarType::INT);
    }else if(_beginAST->getType() == VarType::NUMBER){
      _beginAST->setType(VarType::INT);
      Logger::warning << "AST warning: the begin exression of 'for' has been implicitly convert from NUMBER into a int " << std::endl;
    }else{
      Logger::error << "AST error: the begin exression of 'for' must be an int, not a " << _beginAST->getType() << std::endl;
    }
  }
  this->_endAST->taggingPass(argVars, localVars, globaleVars, persistentVars);
  if(_endAST->getType() != VarType::INT){
    if(_endAST->getType() == VarType::NOTDEFINE){
      _endAST->setType(VarType::INT);
    }else if(_endAST->getType() == VarType::NUMBER){
      _endAST->setType(VarType::INT);
      Logger::warning << "AST warning: the end exression of 'for' has been implicitly convert from NUMBER into a int " << std::endl;
    }else{
      Logger::error << "AST error: the end exression of 'for' must be an int, not a " << _endAST->getType() << std::endl;
    }
  }
  this->_incrementAST->taggingPass(argVars, localVars, globaleVars, persistentVars);
  if(_incrementAST->getType() != VarType::INT){
    if(_incrementAST->getType() == VarType::NOTDEFINE){
      _incrementAST->setType(VarType::INT);
    }else if(_incrementAST->getType() == VarType::NUMBER){
      _incrementAST->setType(VarType::INT);
      Logger::warning << "AST warning: the increment exression of 'for' has been implicitly convert from NUMBER into a int " << std::endl;
    }else{
      Logger::error << "AST error: the increment exression of 'for' must be an int, not a " << _incrementAST->getType() << std::endl;
    }  
  }
}

bool ForAST::Codegen(Builder& b)
{
  IRBuilder<>& builder = b.irbuilder();
  
  // Find the function to generate the code in
  Function *f = builder.GetInsertBlock()->getParent();
  assert(f != nullptr);
  
  // Creates all the blocks
  BasicBlock *initBB = BasicBlock::Create(b.context(), "for.init", f);
  BasicBlock *condBB = BasicBlock::Create(b.context(), "for.cond");
  BasicBlock *condAscBB = BasicBlock::Create(b.context(), "for.condAsc");
  BasicBlock *condDscBB = BasicBlock::Create(b.context(), "for.condDes");
  BasicBlock *loopBB = BasicBlock::Create(b.context(), "for.body");
  BasicBlock *endBB = BasicBlock::Create(b.context(), "for.cont");
  

  
  // Generate the link between previous code and the loop
  builder.CreateBr(initBB);
  builder.SetInsertPoint(initBB);
  b.currentBlock() = initBB;

  //init increment
  Value *beginV = this->_beginAST->Codegen(b);
  Value *initV = this->_variableAST->CodegenMute(b, beginV);

  if (!initV) {
    return false;
  }

  // Create the entry cond block
  builder.CreateBr(condBB);
  f->getBasicBlockList().push_back(condBB);
  builder.SetInsertPoint(condBB);
  b.currentBlock() = condBB;
  
  // Generate condition ( end > begin ? ascendant : descendant)
  Value *endV = this->_endAST->Codegen(b);
  Value *condV = builder.CreateICmpSGT (endV, beginV, "for.cond.expr");
  if (!condV) {
    return false;
  }
  builder.CreateCondBr(condV, condAscBB, condDscBB);
  
  // Generate Cond bloc Ascendant
  f->getBasicBlockList().push_back(condAscBB);
  builder.SetInsertPoint(condAscBB);
  b.currentBlock() = condAscBB;
  
  // Generate condition ( $var <= end ? loop : endbb)
  Value *condAscV = builder.CreateICmpSLE (this->_variableAST->Codegen(b), endV, "for.cond.expr.asc");
  if (!condV) {
    return false;
  }
  builder.CreateCondBr(condAscV, loopBB, endBB);
  
    // Generate the link between previous code and the loop
  f->getBasicBlockList().push_back(condDscBB);
  builder.SetInsertPoint(condDscBB);
  b.currentBlock() = condDscBB;
  
  // Generate condition ( $var >= end ? loop : endbb)
  Value *condDscV = builder.CreateICmpSGE (this->_variableAST->Codegen(b), endV, "for.cond.expr.dsc");
  if (!condV) {
    return false;
  }
  builder.CreateCondBr(condDscV, loopBB, endBB);

  // Emit Loop Body block
  f->getBasicBlockList().push_back(loopBB);
  builder.SetInsertPoint(loopBB);
  b.currentBlock() = loopBB;
  
  
  // Generate loop body
  if (!this->_loopAST->Codegen(b)) {
    return false;
  }
  
    //increment the increment
    Value *incV = this->_incrementAST->Codegen(b);
    Value *newIV = b.irbuilder().CreateAdd(this->_variableAST->Codegen(b), incV, "for.i", loopBB);
 
    if (!newIV) {
      return false;
    }
  
    Value *incrementV = this->_variableAST->CodegenMute(b, newIV);
    
    if (!incrementV) {
      return false;
    }

  
  builder.CreateBr(condBB);

  
  // Emit loop continuation block
  f->getBasicBlockList().push_back(endBB);
  builder.SetInsertPoint(endBB);
  b.currentBlock() = endBB;
  return true;
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
    if(_condAST->getType() == VarType::NOTDEFINE){
      _condAST->setType(VarType::BOOLEAN);
    }else{
      Logger::error << "AST error: the condition of 'while' must be a boolean, not a " << _condAST->getType() << std::endl;
    }
  }
  this->_loopAST->taggingPass(argVars, localVars, globaleVars, persistentVars);
}

bool WhileAST::Codegen(Builder& b)
{
  IRBuilder<>& builder = b.irbuilder();
  
  // Find the function to generate the code in
  Function *f = builder.GetInsertBlock()->getParent();
  assert(f != nullptr);
  
  // Creates all the blocks
  BasicBlock *condBB = BasicBlock::Create(b.context(), "while.cond", f);
  BasicBlock *loopBB = BasicBlock::Create(b.context(), "while.body");
  BasicBlock *endBB = BasicBlock::Create(b.context(), "while.cont");
  
  // Generate the link between previous code and the loop
  builder.CreateBr(condBB);
  builder.SetInsertPoint(condBB);
  b.currentBlock() = condBB;
  
  // Generate condition
  Value *condV = this->_condAST->Codegen(b);
  if (!condV) {
    return false;
  }
  builder.CreateCondBr(condV, loopBB, endBB);

  // Emit Loop Body block
  f->getBasicBlockList().push_back(loopBB);
  builder.SetInsertPoint(loopBB);
  b.currentBlock() = loopBB;
  
  // Generate loop body
  if (!this->_loopAST->Codegen(b)) {
    return false;
  }
  builder.CreateBr(condBB);
  
  // Emit loop continuation block
  f->getBasicBlockList().push_back(endBB);
  builder.SetInsertPoint(endBB);
  b.currentBlock() = endBB;
  return true;
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
    if(_condAST->getType() == VarType::NOTDEFINE){
      _condAST->setType(VarType::BOOLEAN);
    }else{
      Logger::error << "AST error: the condition of 'while' must be a boolean, not a " << _condAST->getType() << std::endl;
    }
  }
}

bool RepeatAST::Codegen(Builder& b)
{
  IRBuilder<>& builder = b.irbuilder();
  
  // Find the function to generate the code in
  Function *f = builder.GetInsertBlock()->getParent();
  assert(f != nullptr);
  
  // Creates all the blocks
  BasicBlock *loopBB = BasicBlock::Create(b.context(), "repeat.body", f);
  BasicBlock *endBB = BasicBlock::Create(b.context(), "repeat.cont");
  
  // Generate the link between previous code and the loop
  builder.CreateBr(loopBB);
  builder.SetInsertPoint(loopBB);
  b.currentBlock() = loopBB;
  
  // Generate loop body
  if (!this->_loopAST->Codegen(b)) {
    return false;
  }
  
  // Generate condition
  Value *condV = this->_condAST->Codegen(b);
  if (!condV) {
    return false;
  }
  builder.CreateCondBr(condV, endBB, loopBB);
  
  // Emit loop continuation block
  f->getBasicBlockList().push_back(endBB);
  builder.SetInsertPoint(endBB);
  b.currentBlock() = endBB;
  return true;
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
  if (Util::strConvert(this->_name, num) && num >= 0) {
    argVars[num] = this->getType();
  }
  localVars[this->_name] = this->getType();
}

string LocalVariableAST::_toString(const string& firstPrefix, const string& prefix) const
{
  stringstream ss;
  ss  << firstPrefix << "Expression::Variable $" << this->_name << endl;
  return ss.str();
}

Value* LocalVariableAST::Codegen(Builder& b)
{
  AllocaInst* Alloca = b.localVars()[this->_name];
  assert(Alloca != nullptr);
  Value* V = b.irbuilder().CreateLoad(Alloca, "var.local."+_name+".");
  return V ? V : AST::Error<Value>("Unknown local variable name");
}
Value* LocalVariableAST::CodegenMute(Builder& b, llvm::Value* Val)
{
  AllocaInst* Alloca = b.localVars()[this->_name];
  assert(Alloca != nullptr);
  Value* V = b.irbuilder().CreateStore(Val, Alloca);
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
  GlobalVariable* ptr = b.globalVars()[this->_name];
  assert(ptr != nullptr);
  Value* V = b.irbuilder().CreateLoad(ptr, "var.global."+_name+".");
  return V ? V : AST::Error<Value>("Unknown global variable name");
}
Value* GlobaleVariableAST::CodegenMute(Builder& b, llvm::Value* Val)
{
  GlobalVariable* ptr = b.globalVars()[this->_name];
  assert(ptr != nullptr);
  Value* V = b.irbuilder().CreateStore(Val, ptr);
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
  GlobalVariable* ptr = b.persistentVars()[this->_name];
  assert(ptr != nullptr);
  Value* V = b.irbuilder().CreateLoad(ptr, "var.persistent."+_name+".");
  return V ? V : AST::Error<Value>("Unknown persistent variable name");
}
Value* PersistentVariableAST::CodegenMute(Builder& b, llvm::Value* Val)
{
  GlobalVariable* ptr = b.persistentVars()[this->_name];
  assert(ptr != nullptr);
  Value* V = b.irbuilder().CreateStore(Val, ptr);
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
  if (this->_str == "+") return b.irbuilder().CreateAdd(L, R, "op.add", BB);
  if (this->_str == "-") return b.irbuilder().CreateSub(L, R, "op.sub", BB);
  if (this->_str == "*") return b.irbuilder().CreateMul(L, R, "op.mul", BB);
  if (this->_str == "/") return b.irbuilder().CreateSDiv(L, R, "op.div", BB);
  
  // Comparaisons
  if (this->_str == "<")  return b.irbuilder().CreateICmpSLT(L, R, "op.lt");
  if (this->_str == "<=") return b.irbuilder().CreateICmpSLE(L, R, "op.le");
  if (this->_str == ">")  return b.irbuilder().CreateICmpSGT(L, R, "op.gt");
  if (this->_str == ">=") return b.irbuilder().CreateICmpSGE(L, R, "op.ge");

  
  // Opérateurs logiques
  if (this->_str == "&")  return b.irbuilder().CreateAnd(L, R, "op.or");
  if (this->_str == "|") return b.irbuilder().CreateOr(L, R, "op.and");
  //if (this->_str == "^")  return b.irbuilder().CreateXor(L, R, "xortmp");
  
  if (this->_str == "=")  return b.irbuilder().CreateICmpEQ(L, R, "op.eq");
  if (this->_str == "#") return b.irbuilder().CreateICmpNE (L, R, "op.neq");
  
  stringstream ss;
  ss << endl << "Build Error: Invalid binary operator " << _str;
  return AST::Error<Value>(ss.str());
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
    name = Builtin::getList()[name]->signature()->name();
  }
  // Look up the name in the global module table.
  Function *CalleeF = b.module().getFunction(name);
  if (!CalleeF) {
    stringstream ss;
    ss  << endl << "Build Error: Unknown function \"" << name << "\"";
    return AST::Error<Value>(ss.str());
  }

  // If argument mismatch error.
  if (CalleeF->arg_size() != this->_args.size()) {
    stringstream ss;
    ss  << endl << "Build Error: Incorrect args number passed to \"" << name << "\": "
        << CalleeF->arg_size() << " expected, " << this->_args.size() << " passed";
    return AST::Error<Value>(ss.str());
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

  return b.irbuilder().CreateCall(CalleeF, ArgsV, "call." + name);
}


