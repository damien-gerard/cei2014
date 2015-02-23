#ifndef BUILTINS_H
#define BUILTINS_H

#include <string>
#include <vector>
#include <map>

class Builtin;

int BUILTINalert(int);

class Builtin {
  public:
    Builtin(const std::string&, int, void*);
    ~Builtin();
    
    std::string getName() const;
    int getArgNumber() const;
    std::vector<std::string> getArgsName() const;
    void* getPtr() const;
    
    static std::map<std::string, Builtin*> _list;
    static std::map<std::string, Builtin*> getList();
    
  protected:
  private:
    std::string _name;
    int _argNumber;
    void* _ptr;
};

#endif // BUILTINS_H
