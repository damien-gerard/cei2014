
#include "../include/vartype.h"
#include <iostream>

std::ostream& operator<<(std::ostream& out, VarType vtype){
  switch (vtype) {
  case VarType::NOTDEFINE:
    out << "not defined";
    break;
  case VarType::NUMBER:
    out << "number";
    break;
  case VarType::INT:
    out << "int";
    break;
  case VarType::STRING:
    out << "string";
    break;
  case VarType::BOOLEAN:
    out << "boolean";
    break;
  case VarType::VOID:
    out << "void";
    break;
  }
  return out;
}
