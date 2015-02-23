#include "../include/builtins.h"
#include <algorithm>
#include <cstdio>
using namespace std;


int BUILTINalert(int i)
{
  printf("%i\n", i);
  return 0;
}

std::map<std::string, Builtin*> Builtin::_list{
  {"alert", new Builtin("BUILTINalert", 1, (void*) &::BUILTINalert)}
};



Builtin::Builtin(const string& name, int argNumber, void* ptr)
  : _name(name), _argNumber(argNumber), _ptr(ptr)
{}
Builtin::~Builtin() = default;

string Builtin::getName() const
{
  return _name;
}
int Builtin::getArgNumber() const
{
  return _argNumber;
}
vector<string> Builtin::getArgsName() const
{
  int i = 0;
  vector<string> vec(_argNumber);
  generate(vec.begin(), vec.end(), [&] () {return "arg" + i++;});
  return vec;
}
void* Builtin::getPtr() const
{
  return _ptr;
}

std::map<std::string, Builtin*> Builtin::getList()
{
  return _list;
}

