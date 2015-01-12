#include "../include/ast.h"
#include <iostream>
#include <sstream>
#include "../include/util/util.h"
//#include "../include/builder.h"

using namespace std;
//using namespace llvm;

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
 * forAST
 */
forAST::forAST(StatementAST* beginAST, ExprAST* condAST, StatementAST* endAST, BlocAST* loopAST)
  : _beginAST(beginAST), _endAST(endAST), _condAST(condAST), _loopAST(loopAST)
{}

forAST::~forAST()
{
  delete this->_beginAST;
  delete this->_endAST;
  delete this->_condAST;
  delete this->_loopAST;
}

string forAST::_toString(const string& firstPrefix, const string& prefix) const
{
  string nextFirstPrefix, nextPrefix;
  nextFirstPrefix = prefix + PREFIX_BEGIN;
  nextPrefix = prefix + PREFIX_MIDDLE;
  stringstream ss;
  ss  << firstPrefix << "Statement::FOR" << endl
      << this->_beginAST->toString(nextFirstPrefix, nextPrefix)
      << this->_condAST ->toString(nextFirstPrefix, nextPrefix)
      << this->_endAST  ->toString(nextFirstPrefix, nextPrefix);
  nextPrefix = prefix + PREFIX_END;
  ss  << this->_loopAST ->toString(nextFirstPrefix, nextPrefix);
  return ss.str();
}

/**
 * whileAST
 */
whileAST::whileAST(ExprAST* condAST, BlocAST* loopAST)
  : _condAST(condAST), _loopAST(loopAST)
{}

whileAST::~whileAST()
{
  delete this->_condAST;
  delete this->_loopAST;
}

string whileAST::_toString(const string& firstPrefix, const string& prefix) const
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
 * ExprAST
 */
ExprAST::~ExprAST() = default;


/**
 * LiteralAST
 */
LiteralAST::LiteralAST(const std::string& val)
  : _val(val)
{}
LiteralAST::~LiteralAST() = default;

string LiteralAST::_toString(const string& firstPrefix, const string& prefix) const
{
  stringstream ss;
  ss  << firstPrefix << "Expression::Literal " << this->_val << endl;
  return ss.str();
}
/*
Value* LiteralAST::Codegen(Builder& b)
{
  return ConstantFP::get(getGlobalContext(), APFloat(this->_val));
}
*/

/**
 * VariableAST
 */
VariableAST::~VariableAST() = default;
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

string LocalVariableAST::_toString(const string& firstPrefix, const string& prefix) const
{
  stringstream ss;
  ss  << firstPrefix << "Expression::Variable $" << this->_name << endl;
  return ss.str();
}


/**
 * GlobaleVariableAST
 */
GlobaleVariableAST::GlobaleVariableAST(const std::string& name)
  : _name(name)
{}
GlobaleVariableAST::~GlobaleVariableAST() = default;

string GlobaleVariableAST::_toString(const string& firstPrefix, const string& prefix) const
{
  stringstream ss;
  ss  << firstPrefix << "Expression::Variable " << this->_name << endl;
  return ss.str();
}


/**
 * PersistentVariableAST
 */
PersistentVariableAST::PersistentVariableAST(const std::string& name)
  : _name(name)
{}
PersistentVariableAST::~PersistentVariableAST() = default;

string PersistentVariableAST::_toString(const string& firstPrefix, const string& prefix) const
{
  stringstream ss;
  ss  << firstPrefix << "Expression::Variable <>" << this->_name << endl;
  return ss.str();
}


/**
 * OpAST
 */
OpAST::OpAST(const std::string& op, ExprAST* lhs, ExprAST* rhs)
  : _str(op), _lhs(lhs), _rhs(rhs)
{}
OpAST::~OpAST()
{
  delete this->_lhs;
  delete this->_rhs;
}

string OpAST::_toString(const string& firstPrefix, const string& prefix) const
{
  string nextFirstPrefix, nextPrefix;
  nextFirstPrefix = prefix + PREFIX_BEGIN;
  nextPrefix = prefix + PREFIX_MIDDLE;
  stringstream ss;
  ss  << firstPrefix << "Expression::OP " << this->_str << endl
      << this->_lhs->toString(nextFirstPrefix, nextPrefix);
  nextPrefix = prefix + PREFIX_END;
  ss  << this->_rhs->toString(nextFirstPrefix, nextPrefix);
  return ss.str();
}
/*
Value* OpAST::Codegen(Builder& b)
{

  Value *L = this->_lhs->Codegen(b);
  Value *R = this->_rhs->Codegen(b);
  if (!L || !R) {
    return nullptr;
  }

  switch (this->_chr) {
  case '+':
    return b.irbuilder().CreateFAdd(L, R, "addtmp");
  case '-':
    return b.irbuilder().CreateFSub(L, R, "subtmp");
  case '*':
    return b.irbuilder().CreateFMul(L, R, "multmp");
  case '<':
    L = b.irbuilder().CreateFCmpULT(L, R, "cmptmp");
    // Convert bool 0/1 to double 0.0 or 1.0
    return b.irbuilder().CreateUIToFP(L, Type::getDoubleTy(getGlobalContext()),
                                      "booltmp");
  default:
    return AST::Error<Value>("invalid binary operator");
  }
}
*/

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

string CallAST::_toString(const string& firstPrefix, const string& prefix) const
{
  string nextFirstPrefix, nextPrefix;
  nextFirstPrefix = prefix + PREFIX_BEGIN;
  nextPrefix = prefix + PREFIX_MIDDLE;
  stringstream ss;
  ss << firstPrefix << "Expression::Call " << this->_name << endl;
  int length = this->_args.size();
  for (int i ; i < length ; ++i) {
    if (i + 1 == length) {
      nextPrefix = prefix + PREFIX_END;
    }
    ss << this->_args[i]->toString(nextFirstPrefix, nextPrefix);
  }
  return ss.str();
}
/*
Value* CallAST::Codegen(Builder& b)
{
  // Look up the name in the global module table.
  Function *CalleeF = b.module().getFunction(this->_name);
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
*/

/**
 * DefinitionAST
 */



/**
 * PrototypeAST
 */
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
/*
Function* PrototypeAST::Codegen(Builder& b)
{
  // Make the function type:  double(double,double) etc.
  std::vector<Type*> Doubles(this->_args.size(),
                             Type::getDoubleTy(getGlobalContext()));
  FunctionType *FT = FunctionType::get(Type::getDoubleTy(getGlobalContext()),
                                       Doubles, false);

  Function *F = Function::Create(FT, Function::ExternalLinkage, this->_name, &b.module());


  // If F conflicted, there was already something named 'Name'.  If it has a
  // body, don't allow redefinition or reextern.
  if (F->getName() != this->_name) {
    // Delete the one we just made and get the existing one.
    F->eraseFromParent();
    F = b.module().getFunction(this->_name);

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
    b.namedValues()[*argsi] = fargsi;
  }
  return F;
}
*/

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
/*
Function* FunctionAST::Codegen(Builder& b)
{
  b.namedValues().clear();

  Function *f = this->_proto->Codegen(b);
  if (!f) {
    return nullptr;
  }

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
}
*/
