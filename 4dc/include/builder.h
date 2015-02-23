#ifndef BUILDER_H
#define BUILDER_H

#include "ast.h"
#include "llvm-dependencies.h"
#include <map>
#include <string>

class Parser;
class Builder
{
  public:
    Builder();
    Builder(const std::string&);
    ~Builder();

    void build(AST*);
    void createJIT();
    void setOptimizer(llvm::FunctionPassManager*);

    llvm::FunctionPassManager* getStandardOptimizer();

    friend llvm::BasicBlock* BlocAST::Codegen(Builder&, const std::string&, llvm::Function*);
    friend llvm::Value* LiteralAST::Codegen(Builder&);
    friend llvm::Value* LocalVariableAST::Codegen(Builder&);
    friend llvm::Value* GlobaleVariableAST::Codegen(Builder&);
    friend llvm::Value* PersistentVariableAST::Codegen(Builder&);
    friend llvm::Value* UniOpAST::Codegen(Builder&);
    friend llvm::Value* BinOpAST::Codegen(Builder&);
    friend llvm::Value* CallAST::Codegen(Builder&);
    friend llvm::Function* PrototypeAST::Codegen(Builder&);
    friend llvm::Function* FunctionAST::Codegen(Builder&);
  protected:
    inline llvm::Module& module() {return *this->_mod;}
    inline llvm::LLVMContext& context(){return this->_ctx;}
    inline llvm::BasicBlock*& currentBlock(){return this->_currentBlock;}
    inline llvm::IRBuilder<>& irbuilder(){return this->_irb;}
    //inline llvm::ExecutionEngine* jit() {return this->_jit;}
    //inline llvm::FunctionPassManager* optimizer() {return *this->_optimizer;}
    inline std::map<std::string, llvm::Value*>& localVars()      {return this->_localVars;     }
    inline std::map<std::string, llvm::Value*>& globalVars()     {return this->_globalVars;    }
    inline std::map<std::string, llvm::Value*>& persistentVars() {return this->_persistentVars;}
    void optimize(llvm::Function*);
    void declareBuiltins();
  private:
    llvm::Module* _mod; // delete at destruction
    llvm::IRBuilder<> _irb;
    llvm::LLVMContext& _ctx;
    llvm::BasicBlock* _currentBlock;
    llvm::ExecutionEngine* _jit; // delete at destruction
    llvm::FunctionPassManager* _optimizer; // delete at destruction
    std::map<std::string, llvm::Value*> _localVars;
    std::map<std::string, llvm::Value*> _globalVars;
    std::map<std::string, llvm::Value*> _persistentVars;
};

#endif // BUILDER_H
