#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <iostream>
#include <initializer_list>
#include "llvm-dependencies.h"
#include "vartype.h"

class Builder;

class AST
{
  public:
    virtual ~AST();

    std::string toString(const std::string& firstPrefix, const std::string& prefix) const;
    inline bool isVar() const {return this->_isVar();}
    inline void defineType() {return this->_defType();}

    template<class T = AST>
    static T* Error(const std::string& msg);
    friend std::ostream& operator<<(std::ostream& out, const AST& ast);
  protected:
    virtual bool _isVar() const;
  private:
    virtual void _defType() = 0;
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
    virtual llvm::BasicBlock* Codegen(Builder&, llvm::Function* = nullptr);
    virtual llvm::BasicBlock* Codegen(Builder&, const std::string&, llvm::Function* = nullptr);
  protected:
  private:
    std::vector<StatementAST*> _statements; // delete at destruction
    
    virtual void _defType();

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class StatementAST : public AST
{
  public:
    virtual ~StatementAST();
    virtual llvm::Value* Codegen(Builder&) = 0;
  protected:
  private:
    virtual void _defType()=0;
  
    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const = 0;
};

class ExprAST;
class StatementExprAST : public StatementAST
{
  public:
    StatementExprAST(ExprAST*);
    virtual ~StatementExprAST();
    virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    ExprAST* _expr; // delete at destruction
    
    virtual void _defType();

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};
class VariableAST;
class AffectationAST : public StatementAST
{
  public:
    AffectationAST(VariableAST * variableAST, ExprAST* expr);
    virtual ~AffectationAST();
    virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    VariableAST * _variableAST;
    ExprAST* _expr; // delete at destruction
    
    virtual void _defType();

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class IfAST : public StatementAST
{
  public:
    IfAST(ExprAST* condAST, BlocAST* thenAST);
    IfAST(ExprAST* condAST, BlocAST* thenAST, BlocAST* elseAST);
    virtual ~IfAST();
    virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    ExprAST *_condAST; // delete at destruction
    BlocAST *_thenAST, *_elseAST; // delete at destruction
    
    virtual void _defType();

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};


class ForAST : public StatementAST
{
  public:
    ForAST(VariableAST * variableAST, ExprAST * beginAST, ExprAST * endAST, ExprAST * incrementAST, BlocAST* loopAST);
    virtual ~ForAST();
    virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    VariableAST * _variableAST;
    ExprAST *_beginAST;
    ExprAST *_endAST;
    ExprAST *_incrementAST;
    BlocAST *_loopAST;
    
    virtual void _defType();

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class WhileAST : public StatementAST
{
  public:
    WhileAST(ExprAST* condAST, BlocAST* loopAST);
    virtual ~WhileAST();
    virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    ExprAST *_condAST;
    BlocAST *_loopAST;
    
    virtual void _defType();

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class RepeatAST : public StatementAST
{
  public:
    RepeatAST(ExprAST* condAST, BlocAST* loopAST);
    virtual ~RepeatAST();
    virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    ExprAST *_condAST;
    BlocAST *_loopAST;
    
    virtual void _defType();

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class ExprAST : public AST
{
  public:
    virtual ~ExprAST();
    virtual llvm::Value* Codegen(Builder&) = 0;
    void setType(VarType vtype);
    VarType getType() const;
  protected:
    VarType _vtype;
  private:
    virtual void _defType()=0;
    
    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const = 0;
};

class LiteralAST : public ExprAST
{
  public:
    LiteralAST(const std::string& val, VarType vtype);
    virtual ~LiteralAST();
    virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    std::string _val;
    
    virtual void _defType();

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class VariableAST : public ExprAST
{
  public:
    VariableAST();
    virtual ~VariableAST();
    virtual llvm::Value* Codegen(Builder&) = 0;
  protected:
    virtual bool _isVar() const;
  private:
    virtual void _defType();
    
    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const = 0;
};

class LocalVariableAST : public VariableAST
{
  public:
    LocalVariableAST(const std::string &);
    virtual ~LocalVariableAST();
    virtual llvm::Value* Codegen(Builder&);
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
    virtual llvm::Value* Codegen(Builder&);
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
    virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    std::string _name;

    
    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class UniOpAST : public ExprAST
{
  public:
    UniOpAST(const std::string& op, ExprAST* expr);
    virtual ~UniOpAST();
    virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    std::string _str;
    ExprAST *_expr; // delete at destruction

    virtual void _defType();
    
    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class BinOpAST : public ExprAST
{
  public:
    BinOpAST(const std::string& op, ExprAST* lhs, ExprAST* rhs);
    virtual ~BinOpAST();
    virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    std::string _str;
    ExprAST *_lhs, *_rhs; // delete at destruction

    virtual void _defType();
    
    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class CallAST : public ExprAST
{
  public:
    CallAST(const std::string&, const std::vector<ExprAST*>&);
    virtual ~CallAST();
    virtual llvm::Value* Codegen(Builder&);
  protected:
  private:
    std::string _name;
    std::vector<ExprAST*> _args; // delete at destruction
    
    virtual void _defType();

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};


class DefinitionAST : public AST
{
  public:
    virtual ~DefinitionAST() = default;
    virtual llvm::Function* Codegen(Builder&) = 0;
  protected:
  private:
    virtual void _defType();
  
    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const = 0;
};

class PrototypeAST : public DefinitionAST
{
  public:
    PrototypeAST(const std::string&, const std::vector<std::string>&);
    virtual ~PrototypeAST();
    virtual llvm::Function* Codegen(Builder&);
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
    virtual llvm::Function* Codegen(Builder&);
  protected:
  private:
    PrototypeAST* _proto; // keep at destruction
    BlocAST* _body; // delete at destruction
    

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};


#endif // AST_H
