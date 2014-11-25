#ifndef BUILDER_H
#define BUILDER_H

#include "ast.h"
#include <map>
#include <string>

/*
namespace llvm {
    class Module;
    template<>
    class IRBuilder;
}

*/
class Parser;
namespace llvm {
  class ExecutionEngine;
}
class Builder
{
    public:
        Builder(Parser&);
        Builder(Parser&, llvm::ExecutionEngine*);
        Builder(const std::string&, Parser&);
        Builder(const std::string&, Parser&, llvm::ExecutionEngine*);
        ~Builder();

        void build();

        friend llvm::Value* VariableAST::Codegen(Builder&);
        friend llvm::Value* OpAST::Codegen(Builder&);
        friend llvm::Value* CallAST::Codegen(Builder&);
        friend llvm::Function* PrototypeAST::Codegen(Builder&);
        friend llvm::Function* FunctionAST::Codegen(Builder&);
    protected:
        inline llvm::Module& module() {return *this->_mod;}
        inline llvm::IRBuilder<>& irbuilder() {return this->_irb;}
        //inline llvm::ExecutionEngine* irbuilder() {return this->_jit;}
        inline std::map<std::string, llvm::Value*>& namedValues() {return this->_namedValues;}
    private:
        Parser& _parser;
        llvm::Module* _mod;
        llvm::IRBuilder<> _irb;
        llvm::ExecutionEngine* _jit;
        std::map<std::string, llvm::Value*> _namedValues;
};

#endif // BUILDER_H
