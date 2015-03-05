#ifndef FUNCTIONSIGNATURE_H
#define FUNCTIONSIGNATURE_H

#include <string>
#include <vector>
#include <iostream>
#include <initializer_list>
#include "vartype.h"

class FunctionSignature
{
  public:
    /* Constructors */
    // void name()
    FunctionSignature(const std::string& name);
    //
    // returnType name()
    FunctionSignature(const std::string& name,
                      VarType returnType);
    //
    // void name(argsType args)
    FunctionSignature(const std::string& name,
                      std::initializer_list<VarType> argsType);
    //
    // returnType name(argsType args)
    FunctionSignature(const std::string& name,
                      VarType returnType,
                      std::initializer_list<VarType> argsType);
    //
    // void name(argsType args, variadicArgsType ...)
    FunctionSignature(const std::string& name,
                      std::initializer_list<VarType> argsType,
                      VarType variadicArgsType);
    //
    // returnType name(argsType args, variadicArgsType ...)
    FunctionSignature(const std::string& name,
                      VarType returnType,
                      std::initializer_list<VarType> argsType,
                      VarType variadicArgsType);
    //
    // void name(argsType args)
    FunctionSignature(const std::string& name,
                      const std::vector<VarType>& argsType);
    //
    // returnType name(argsType args)
    FunctionSignature(const std::string& name,
                      VarType returnType,
                      const std::vector<VarType>& argsType);
    //
    // void name(argsType args, variadicArgsType ...)
    FunctionSignature(const std::string& name,
                      const std::vector<VarType>& argsType,
                      VarType variadicArgsType);
    //
    // returnType name(argsType args, variadicArgsType ...)
    FunctionSignature(const std::string& name,
                      VarType returnType,
                      const std::vector<VarType>& argsType,
                      VarType variadicArgsType);
    //
    /* Destructor */
    ~FunctionSignature();
    
    /* getters */
    inline std::string name() const {return _name;}
    inline VarType returnType() const {return _returnType;}
    inline std::vector<VarType> argsType() const {return _argsType;}
    inline VarType variadicArgsType() const {return _variadicArgsType;}
    
    /* helpers */
    inline bool returnVoid() const {return returnType() == VarType::VOID;}
    inline bool returnAnything() const {return !returnVoid();}
    inline int argsNumber() const {return _argsType.size();}
    inline bool takeVariadicArgs() const {return variadicArgsType() != VarType::VOID;}
  protected:
  private:
    std::string _name;
    VarType _returnType;
    std::vector<VarType> _argsType;
    VarType _variadicArgsType;
};
/* operators */
inline bool operator==(FunctionSignature f1, FunctionSignature f2)
{
  return  f1.name() == f2.name() &&
          f1.returnType() == f2.returnType() &&
          f1.argsType() == f2.argsType() &&
          f1.variadicArgsType() == f2.variadicArgsType();
}

std::ostream& operator<<(std::ostream& out, FunctionSignature f);

#endif // FUNCTIONSIGNATURE_H
