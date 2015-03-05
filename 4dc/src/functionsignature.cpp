#include "../include/functionsignature.h"

// void name()
FunctionSignature::FunctionSignature
    (const std::string& name)
: FunctionSignature(name, VarType::VOID, {}, VarType::VOID)
{}

// returnType name()
FunctionSignature::FunctionSignature
    (const std::string& name, VarType returnType)
: FunctionSignature(name, returnType, {}, VarType::VOID)
{}

// void name(argsType args)
FunctionSignature::FunctionSignature
    (const std::string& name, std::initializer_list<VarType> argsType)
: FunctionSignature(name, VarType::VOID, argsType, VarType::VOID)
{}

// returnType name(argsType args)
FunctionSignature::FunctionSignature
    (const std::string& name, VarType returnType, std::initializer_list<VarType> argsType)
: FunctionSignature(name, returnType, argsType, VarType::VOID)
{}

// void name(argsType args, variadicArgsType ...)
FunctionSignature::FunctionSignature
    (const std::string& name, std::initializer_list<VarType> argsType, VarType variadicArgsType)
: FunctionSignature(name, VarType::VOID, argsType, variadicArgsType)
{}

// returnType name(argsType args, variadicArgsType ...)
FunctionSignature::FunctionSignature
    (const std::string& name, VarType returnType, std::initializer_list<VarType> argsType, VarType variadicArgsType)
: _name(name),
  _returnType(returnType),
  _argsType(argsType.begin(), argsType.end()),
  _variadicArgsType(variadicArgsType)
{}

// void name(argsType args)
FunctionSignature::FunctionSignature
    (const std::string& name, const std::vector<VarType>& argsType)
: FunctionSignature(name, VarType::VOID, argsType, VarType::VOID)
{}

// returnType name(argsType args)
FunctionSignature::FunctionSignature
    (const std::string& name, VarType returnType, const std::vector<VarType>& argsType)
: FunctionSignature(name, returnType, argsType, VarType::VOID)
{}

// void name(argsType args, variadicArgsType ...)
FunctionSignature::FunctionSignature
    (const std::string& name, const std::vector<VarType>& argsType, VarType variadicArgsType)
: FunctionSignature(name, VarType::VOID, argsType, variadicArgsType)
{}

// returnType name(argsType args, variadicArgsType ...)
FunctionSignature::FunctionSignature
    (const std::string& name, VarType returnType, const std::vector<VarType>& argsType, VarType variadicArgsType)
: _name(name),
  _returnType(returnType),
  _argsType(argsType),
  _variadicArgsType(variadicArgsType)
{}
FunctionSignature::~FunctionSignature()
{}

std::ostream& operator<<(std::ostream& out, FunctionSignature f)
{
  bool comma = false;
  out << f.returnType() << " " << f.name() << "(";
  for (auto& argType : f.argsType()) {
    if (comma) {
      out << ", ";
    } else {
      comma = true;
    }
    out << argType;
  }
  if (f.takeVariadicArgs()) {
    if (comma) out << ", ";
    out << f.variadicArgsType() << " ...";
  }
  return out << ")";
}
