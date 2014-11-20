#include "../include/ast.h"
#include <iostream>


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

Value* NumberAST::Codegen() {
    return 0;
}


/**
 * VariableAST
 */
VariableAST::VariableAST(const std::string& name)
: _name(name)
{}
VariableAST::~VariableAST() = default;

Value* VariableAST::Codegen() {
    return 0;
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

Value* OpAST::Codegen() {
    return 0;
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

Value* CallAST::Codegen() {
    return 0;
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

