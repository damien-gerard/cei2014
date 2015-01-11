#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <iostream>
//#include "llvm-dependencies.h"

class Builder;

class AST
{
  public:
    virtual ~AST();
    template<class T = AST>
    static T* Error(const std::string& msg);
  protected:
  private:
};


template<class T>
T* AST::Error(const std::string& msg)
{
  std::cerr << msg << std::endl;
  return nullptr;
}
class StatementAST;
class BlocAST : public AST
{
  public:
    BlocAST(StatementAST*);
    BlocAST(const std::vector<StatementAST*>&);
    virtual ~BlocAST();
    //virtual llvm::???* Codegen(Builder&) = 0;
  protected:
  private:
    std::vector<StatementAST*> _statements; // delete at destruction
};

class StatementAST : public AST
{
  public:
    virtual ~StatementAST();
    //virtual llvm::???* Codegen(Builder&) = 0;
  protected:
  private:
};

class ExprAST;
class StatementExprAST : public StatementAST
{
  public:
    StatementExprAST(ExprAST*);
    virtual ~StatementExprAST();
    //virtual llvm::???* Codegen(Builder&);
  protected:
  private:
    ExprAST* _expr; // delete at destruction
};

class IfAST : public StatementAST
{
  public:
    IfAST(ExprAST* condAST, BlocAST* thenAST);
    IfAST(ExprAST* condAST, BlocAST* thenAST, BlocAST* elseAST);
    virtual ~IfAST();
    //virtual llvm::???* Codegen(Builder&);
  protected:
  private:
    ExprAST *_condAST; // delete at destruction
    BlocAST *_thenAST, *_elseAST; // delete at destruction
};

class forAST : public StatementAST
{
  public:
    forAST(StatementAST* beginAST, ExprAST* condAST, StatementAST* endAST, BlocAST* loopAST);
    virtual ~forAST();
    //virtual llvm::???* Codegen(Builder&);
  protected:
  private:
    StatementAST *_beginAST, *_endAST;
    ExprAST *_condAST;
    BlocAST *_loopAST;
};

class whileAST : public StatementAST
{
  public:
    whileAST(ExprAST* condAST, BlocAST* loopAST);
    virtual ~whileAST();
    //virtual llvm::???* Codegen(Builder&);
  protected:
  private:
    ExprAST *_condAST;
    BlocAST *_loopAST;
};

class ExprAST : public AST
{
  public:
    virtual ~ExprAST();
    //virtual llvm::Value* Codegen(Builder&) = 0;
  protected:
  private:
};

class LitteralAST : public ExprAST
{
  public:
    LitteralAST(const std::string& val);
    virtual ~LitteralAST();
    //virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    std::string _val;
};

class VariableAST : public ExprAST
{
  public:
    //VariableAST(const std::string &) = 0;
    virtual ~VariableAST();
    //virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
};

class LocalVariableAST : public ExprAST
{
  public:
    LocalVariableAST(const std::string &);
    virtual ~LocalVariableAST();
    //virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    std::string _name;
};

class GlobaleVariableAST : public ExprAST
{
  public:
    GlobaleVariableAST(const std::string &);
    virtual ~GlobaleVariableAST();
    //virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    std::string _name;
};

class PersistentVariableAST : public ExprAST
{
  public:
    PersistentVariableAST(const std::string &);
    virtual ~PersistentVariableAST();
    //virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    std::string _name;
};


class OpAST : public ExprAST
{
  public:
    OpAST(char op, ExprAST* lhs, ExprAST* rhs);
    virtual ~OpAST();
    //virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    char _chr;
    ExprAST *_lhs, *_rhs; // delete at destruction
};

class CallAST : public ExprAST
{
  public:
    CallAST(const std::string&, const std::vector<ExprAST*>&);
    virtual ~CallAST();
    //virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    std::string _name;
    std::vector<ExprAST*> _args; // delete at destruction
};


class DefinitionAST : public AST
{
  public:
    virtual ~DefinitionAST() = default;
    //virtual llvm::Function* Codegen(Builder&) = 0;
  protected:
  private:
};

class PrototypeAST : public DefinitionAST
{
  public:
    PrototypeAST(const std::string&, const std::vector<std::string>&);
    virtual ~PrototypeAST();
    //virtual llvm::Function* Codegen(Builder&);
  protected:
  private:
    std::string _name;
    std::vector<std::string> _args;
};

class FunctionAST : public DefinitionAST
{
  public:
    FunctionAST(PrototypeAST*, BlocAST*);
    virtual ~FunctionAST();
    //virtual llvm::Function* Codegen(Builder&);
  protected:
  private:
    PrototypeAST* _proto; // keep at destruction
    BlocAST* _body; // delete at destruction
};


#endif // AST_H
