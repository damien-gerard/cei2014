#include "../include/ast.h"
#include <iostream>

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


/**
 * VariableAST
 */
VariableAST::VariableAST(const std::string& name)
: _name(name)
{}
VariableAST::~VariableAST() = default;


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

