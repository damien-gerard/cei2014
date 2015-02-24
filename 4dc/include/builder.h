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

    inline llvm::Module& module() {return *this->_mod;}
    inline llvm::LLVMContext& context(){return this->_ctx;}
    inline llvm::BasicBlock*& currentBlock(){return this->_currentBlock;}
    inline llvm::IRBuilder<>& irbuilder(){return this->_irb;}
    //inline llvm::ExecutionEngine* jit() {return this->_jit;}
    //inline llvm::FunctionPassManager* optimizer() {return *this->_optimizer;}
    inline std::map<std::string, llvm::AllocaInst*>& localVars()      {return this->_localVars;     }
    inline std::map<std::string, llvm::AllocaInst*>& globalVars()     {return this->_globalVars;    }
    inline std::map<std::string, llvm::AllocaInst*>& persistentVars() {return this->_persistentVars;}
    void optimize(llvm::Function*);
    void declareBuiltins();
  protected:
  private:
    llvm::Module* _mod; // delete at destruction
    llvm::IRBuilder<> _irb;
    llvm::LLVMContext& _ctx;
    llvm::BasicBlock* _currentBlock;
    llvm::ExecutionEngine* _jit; // delete at destruction
    llvm::FunctionPassManager* _optimizer; // delete at destruction
    std::map<std::string, llvm::AllocaInst*> _localVars;
    std::map<std::string, llvm::AllocaInst*> _globalVars;
    std::map<std::string, llvm::AllocaInst*> _persistentVars;
};

#endif // BUILDER_H
