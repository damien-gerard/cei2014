#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <iostream>
#include <initializer_list>
//#include "llvm-dependencies.h"

class Builder;

class AST
{
  public:
    virtual ~AST();

    std::string toString(const std::string& firstPrefix, const std::string& prefix) const;

    template<class T = AST>
    static T* Error(const std::string& msg);
    friend std::ostream& operator<<(std::ostream& out, const AST& ast);
  protected:
  private:
    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const = 0;
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
    BlocAST(std::initializer_list<StatementAST*>);
    BlocAST(const std::vector<StatementAST*>&);
    virtual ~BlocAST();
    //virtual llvm::???* Codegen(Builder&) = 0;
  protected:
  private:
    std::vector<StatementAST*> _statements; // delete at destruction

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class StatementAST : public AST
{
  public:
    virtual ~StatementAST();
    //virtual llvm::???* Codegen(Builder&) = 0;
  protected:
  private:
    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const = 0;
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

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
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

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class VariableAST;
class ForAST : public StatementAST
{
  public:
    ForAST(VariableAST * variableAST, ExprAST * beginAST, ExprAST * endAST, ExprAST * incrementAST, BlocAST* loopAST);
    virtual ~ForAST();
    //virtual llvm::???* Codegen(Builder&);
  protected:
  private:
    VariableAST * _variableAST;
    ExprAST *_beginAST;
    ExprAST *_endAST;
    ExprAST *_incrementAST;
    BlocAST *_loopAST;

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
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

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class ExprAST : public AST
{
  public:
    virtual ~ExprAST();
    //virtual llvm::Value* Codegen(Builder&) = 0;
  protected:
  private:
    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const = 0;
};

class LiteralAST : public ExprAST
{
  public:
    LiteralAST(const std::string& val);
    virtual ~LiteralAST();
    //virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    std::string _val;

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class VariableAST : public ExprAST
{
  public:
    //VariableAST(const std::string &) = 0;
    virtual ~VariableAST();
    //virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const = 0;
};

class LocalVariableAST : public VariableAST
{
  public:
    LocalVariableAST(const std::string &);
    virtual ~LocalVariableAST();
    //virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    std::string _name;

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class GlobaleVariableAST : public VariableAST
{
  public:
    GlobaleVariableAST(const std::string &);
    virtual ~GlobaleVariableAST();
    //virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    std::string _name;

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class PersistentVariableAST : public VariableAST
{
  public:
    PersistentVariableAST(const std::string &);
    virtual ~PersistentVariableAST();
    //virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    std::string _name;

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};


class OpAST : public ExprAST
{
  public:
    OpAST(const std::string& op, ExprAST* lhs, ExprAST* rhs);
    virtual ~OpAST();
    //virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    std::string _str;
    ExprAST *_lhs, *_rhs; // delete at destruction

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
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

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};


class DefinitionAST : public AST
{
  public:
    virtual ~DefinitionAST() = default;
    //virtual llvm::Function* Codegen(Builder&) = 0;
  protected:
  private:
    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const = 0;
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

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
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

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};


#endif // AST_H
