#ifndef BUILDER_H
#define BUILDER_H

#include "ast.h"
#include "llvm-dependencies.h"
#include "util/file.h"
#include <map>
#include <string>
#include <vector>
#include <iostream>

class Parser;
class Func;
class FunctionSignature;
class Builder
{
  public:
    Builder();
    Builder(const std::string&);
    ~Builder();

    static void buildAll(const std::vector<std::pair<std::string,File>>&);
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
    inline std::map<std::string, llvm::GlobalVariable*>& globalVars()     {return this->_globalVars;    }
    inline std::map<std::string, llvm::GlobalVariable*>& persistentVars() {return this->_persistentVars;}
    void optimize(llvm::Function*);
    void declareBuiltins();
    void createAllocas(
                    std::map<std::string, VarType>& types,
                    std::map<std::string, llvm::AllocaInst*>& vals,
                    llvm::Function* F
    );
    void createGlobals(
                    std::map<std::string, VarType>& types,
                    std::map<std::string, llvm::GlobalVariable*>& vals
    );
    void dumpDebug();
    void dumpDebug(llvm::Function*);
  protected:
  private:
    llvm::Module* _mod; // delete at destruction
    llvm::IRBuilder<> _irb;
    llvm::LLVMContext& _ctx;
    llvm::BasicBlock* _currentBlock;
    llvm::ExecutionEngine* _jit; // delete at destruction
    llvm::FunctionPassManager* _optimizer; // delete at destruction
    std::map<std::string, llvm::AllocaInst*> _localVars;
    std::map<std::string, llvm::GlobalVariable*> _globalVars;
    std::map<std::string, llvm::GlobalVariable*> _persistentVars;
    
    Func* parse(
        std::map<std::string, VarType>& globalVars,
        std::map<std::string, VarType>& persistentVars,
        const std::string& name,
        std::istream& in
    );
    llvm::Function* build(Func* Fdef);
    void callFunctionLLVM(llvm::Function *F);
    llvm::Function* createMain(FunctionSignature* signature, llvm::Function *F);
    static llvm::AllocaInst *createEntryBlockAlloca(llvm::Function *F, const std::string& name, llvm::Type* type);
};

#endif // BUILDER_H
