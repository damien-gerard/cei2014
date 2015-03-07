#include "../include/builtins.h"
#include "../include/functionsignature.h"
#include <cstdio>
#include <cstdlib>
using namespace std;


int BUILTINalert(int i)
{
  printf("%i\n", i);
  return 0;
}

int BUILTINabort()
{
  abort();
  return 0;
}

std::map<std::string, Builtin*> Builtin::_list{
  {"alert", new Builtin(new FunctionSignature(
      "BUILTINalert", VarType::INT, {VarType::INT}),
      (void*) &::BUILTINalert)
  },
  {"abort", new Builtin(new FunctionSignature(
      "BUILTINabort", VarType::INT, {}),
      (void*) &::BUILTINabort)
  }
};



Builtin::Builtin(FunctionSignature* signature, void* ptr)
  : _signature(signature), _ptr(ptr)
{}
Builtin::~Builtin() = default;


std::map<std::string, Builtin*> Builtin::getList()
{
  return _list;
}

