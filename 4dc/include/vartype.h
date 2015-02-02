#ifndef VARTYPE_H
#define VARTYPE_H


#include <string>


enum class VarType
{
  NOTDEFINE, // variable non encore typé.

  NUMBER,   // int, double, float... we dont know yet
  INT,      // int
  
  
  STRING,   // string
  BOOLEAN,  // true or false
};

std::ostream& operator<<(std::ostream& out, VarType vtype);

#endif // VARTYPE_H