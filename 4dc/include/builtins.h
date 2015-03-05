#ifndef BUILTINS_H
#define BUILTINS_H

#include <string>
#include <vector>
#include <map>


class Builtin;
class FunctionSignature;
int BUILTINalert(int);

class Builtin {
  public:
    Builtin(FunctionSignature*, void*);
    ~Builtin();
    
    inline FunctionSignature* signature() const {return _signature;}
    inline void* getPtr() const {return _ptr;}
    
    static std::map<std::string, Builtin*> _list;
    static std::map<std::string, Builtin*> getList();
    
  protected:
  private:
    FunctionSignature* _signature;
    void* _ptr;
};

#endif // BUILTINS_H
