#ifndef FUNC_H
#define FUNC_H

#include "vartype.h"
#include "llvm-dependencies.h"
#include <string>
#include <map>

class Builder;
class FunctionSignature;
class BlocAST;
class Func
{
  public:
    Func(const std::string& name, BlocAST* body);
    ~Func();
    
    inline std::string name() {return _name;}
    inline FunctionSignature* signature() {return _signature;}
    
    static llvm::Function* create(FunctionSignature*, Builder&);
    
    
    void taggingPass(
        std::map<std::string, VarType>& globaleVars,
        std::map<std::string, VarType>& persistentVars
    );
    llvm::Function* Codegen(Builder&);
    
    friend std::ostream& operator<<(std::ostream&, const Func&);
  protected:
  private:
    std::string _name;
    FunctionSignature* _signature; // delete at destruction
    std::map<std::string, VarType> _localVars;
    BlocAST* _body; // delete at destruction
};

#endif // FUNC_H
