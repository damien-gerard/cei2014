#include "../include/ast.h"
#include <iostream>
#include "../include/builder.h"

using namespace llvm;

/**
 * AST
 */

/**
 * BlocExprAST
 */


/**
 * ExprAST
 */


/**
 * NumberAST
 */
NumberAST::NumberAST(double n)
: _val(n)
{}
NumberAST::~NumberAST() = default;

Value* NumberAST::Codegen(Builder& b) {
    return ConstantFP::get(getGlobalContext(), APFloat(this->_val));
}


/**
 * VariableAST
 */
VariableAST::VariableAST(const std::string& name)
: _name(name)
{}
VariableAST::~VariableAST() = default;

Value* VariableAST::Codegen(Builder& b) {
    Value* V = b.namedValues()[this->_name];
    return V ? V : AST::Error<Value>("Unknown variable name");
}

/**
 * OpAST
 */
OpAST::OpAST(char op, ExprAST* lhs, ExprAST* rhs)
: _chr(op), _lhs(lhs), _rhs(rhs)
{}
OpAST::~OpAST()
{
    delete this->_lhs;
    delete this->_rhs;
}

Value* OpAST::Codegen(Builder& b) {

    Value *L = this->_lhs->Codegen(b);
    Value *R = this->_rhs->Codegen(b);
    if (!L || !R) return nullptr;

    switch (this->_chr) {
    case '+': return b.irbuilder().CreateFAdd(L, R, "addtmp");
    case '-': return b.irbuilder().CreateFSub(L, R, "subtmp");
    case '*': return b.irbuilder().CreateFMul(L, R, "multmp");
    case '<':
      L = b.irbuilder().CreateFCmpULT(L, R, "cmptmp");
      // Convert bool 0/1 to double 0.0 or 1.0
      return b.irbuilder().CreateUIToFP(L, Type::getDoubleTy(getGlobalContext()),
                                  "booltmp");
    default: return AST::Error<Value>("invalid binary operator");
    }
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

Value* CallAST::Codegen(Builder& b) {
    // Look up the name in the global module table.
    Function *CalleeF = b.module().getFunction(this->_name);
    if (CalleeF == 0)
        return AST::Error<Value>("Unknown function referenced");

    // If argument mismatch error.
    if (CalleeF->arg_size() != this->_args.size())
        return AST::Error<Value>("Incorrect # arguments passed");

    std::vector<Value*> ArgsV;

    auto it = this->_args.begin();
    auto endIT = this->_args.end();
    for (; it != endIT; ++it) {
        ArgsV.push_back((*it)->Codegen(b));
        if (!ArgsV.back()) return nullptr;
    }

    return b.irbuilder().CreateCall(CalleeF, ArgsV, "calltmp");
}


/**
 * IfAST
 */
IfAST::IfAST(ExprAST* condAST, ExprAST* thenAST, ExprAST* elseAST)
: _condAST(condAST), _thenAST(thenAST), _elseAST(elseAST)
{}
IfAST::~IfAST()
{
    delete this->_condAST;
    delete this->_thenAST;
    delete this->_elseAST;
}

Value* IfAST::Codegen(Builder& b) {
    IRBuilder<>& builder = b.irbuilder();

    Value *condV = this->_condAST->Codegen(b);

    if (!condV) return nullptr;

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
    if (!thenV) return nullptr;

    builder.CreateBr(mergeBB);
    // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
    thenBB = builder.GetInsertBlock();

    // Emit else block.
    f->getBasicBlockList().push_back(elseBB);
    builder.SetInsertPoint(elseBB);

    Value *elseV = this->_elseAST->Codegen(b);
    if (!elseV) return nullptr;

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

Function* PrototypeAST::Codegen(Builder& b) {
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


/**
 * FunctionAST
 */
FunctionAST::FunctionAST(PrototypeAST* proto, ExprAST* body)
: _proto(proto), _body(body)
{}
FunctionAST::~FunctionAST()
{
    delete this->_body;
}

Function* FunctionAST::Codegen(Builder& b) {
    b.namedValues().clear();

    Function *f = this->_proto->Codegen(b);
    if (!f) return nullptr;

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

