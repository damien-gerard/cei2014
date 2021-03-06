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
    inline void taggingPass(
                  std::map<int, VarType>& argVars,
                  std::map<std::string, VarType>& localVars,
                  std::map<std::string, VarType>& globaleVars,
                  std::map<std::string, VarType>& persistentVars
                ) {return this->_taggingPass(argVars, localVars, globaleVars, persistentVars);};

    template<class T = AST>
    static T* Error(const std::string& msg);
    friend std::ostream& operator<<(std::ostream& out, const AST& ast);
  protected:
    virtual bool _isVar() const;
  private:
    virtual void _taggingPass(
                  std::map<int, VarType>& argVars,
                  std::map<std::string, VarType>& localVars,
                  std::map<std::string, VarType>& globaleVars,
                  std::map<std::string, VarType>& persistentVars
                ) = 0;
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
    
    virtual void _taggingPass(
                  std::map<int, VarType>& argVars,
                  std::map<std::string, VarType>& localVars,
                  std::map<std::string, VarType>& globaleVars,
                  std::map<std::string, VarType>& persistentVars
                );

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class StatementAST : public AST
{
  public:
    virtual ~StatementAST();
    virtual bool Codegen(Builder&) = 0;
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
    virtual bool Codegen(Builder&);
  protected:
  private:
    ExprAST* _expr; // delete at destruction
    
    virtual void _taggingPass(
                  std::map<int, VarType>& argVars,
                  std::map<std::string, VarType>& localVars,
                  std::map<std::string, VarType>& globaleVars,
                  std::map<std::string, VarType>& persistentVars
                );

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};
class VariableAST;
class AffectationAST : public StatementAST
{
  public:
    AffectationAST(VariableAST * variableAST, ExprAST* expr);
    virtual ~AffectationAST();
    virtual bool Codegen(Builder&);
  protected:
  private:
    VariableAST * _variableAST;
    ExprAST* _expr; // delete at destruction
    
    virtual void _taggingPass(
                  std::map<int, VarType>& argVars,
                  std::map<std::string, VarType>& localVars,
                  std::map<std::string, VarType>& globaleVars,
                  std::map<std::string, VarType>& persistentVars
                );

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class IfAST : public StatementAST
{
  public:
    IfAST(ExprAST* condAST, BlocAST* thenAST);
    IfAST(ExprAST* condAST, BlocAST* thenAST, BlocAST* elseAST);
    virtual ~IfAST();
    virtual bool Codegen(Builder&);
  protected:
  private:
    ExprAST *_condAST; // delete at destruction
    BlocAST *_thenAST, *_elseAST; // delete at destruction
    
    virtual void _taggingPass(
                  std::map<int, VarType>& argVars,
                  std::map<std::string, VarType>& localVars,
                  std::map<std::string, VarType>& globaleVars,
                  std::map<std::string, VarType>& persistentVars
                );

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};
class ForAST : public StatementAST
{
  public:
    ForAST(VariableAST * variableAST, ExprAST * beginAST, ExprAST * endAST, ExprAST * incrementAST, BlocAST* loopAST);
    virtual ~ForAST();
    virtual bool Codegen(Builder&);
  protected:
  private:
    VariableAST * _variableAST;
    ExprAST *_beginAST;
    ExprAST *_endAST;
    ExprAST *_incrementAST;
    BlocAST *_loopAST;
    
    virtual void _taggingPass(
                  std::map<int, VarType>& argVars,
                  std::map<std::string, VarType>& localVars,
                  std::map<std::string, VarType>& globaleVars,
                  std::map<std::string, VarType>& persistentVars
                );

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class WhileAST : public StatementAST
{
  public:
    WhileAST(ExprAST* condAST, BlocAST* loopAST);
    virtual ~WhileAST();
    virtual bool Codegen(Builder&);
  protected:
  private:
    ExprAST *_condAST;
    BlocAST *_loopAST;
    
    virtual void _taggingPass(
                  std::map<int, VarType>& argVars,
                  std::map<std::string, VarType>& localVars,
                  std::map<std::string, VarType>& globaleVars,
                  std::map<std::string, VarType>& persistentVars
                );

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class RepeatAST : public StatementAST
{
  public:
    RepeatAST(ExprAST* condAST, BlocAST* loopAST);
    virtual ~RepeatAST();
    virtual bool Codegen(Builder&);
  protected:
  private:
    ExprAST *_condAST;
    BlocAST *_loopAST;
    
    virtual void _taggingPass(
                  std::map<int, VarType>& argVars,
                  std::map<std::string, VarType>& localVars,
                  std::map<std::string, VarType>& globaleVars,
                  std::map<std::string, VarType>& persistentVars
                );

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
    
    virtual void _taggingPass(
                  std::map<int, VarType>& argVars,
                  std::map<std::string, VarType>& localVars,
                  std::map<std::string, VarType>& globaleVars,
                  std::map<std::string, VarType>& persistentVars
                );

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class VariableAST : public ExprAST
{
  public:
    VariableAST();
    virtual ~VariableAST();
    virtual llvm::Value* Codegen(Builder&) = 0;
    virtual llvm::Value* CodegenMute(Builder&, llvm::Value*) = 0;
  protected:
    virtual bool _isVar() const;
  private:
    
    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const = 0;
};

class LocalVariableAST : public VariableAST
{
  public:
    LocalVariableAST(const std::string &);
    virtual ~LocalVariableAST();
    virtual llvm::Value* Codegen(Builder&);
    virtual llvm::Value* CodegenMute(Builder&, llvm::Value*);
  protected:
  private:
    std::string _name;
    
    virtual void _taggingPass(
                  std::map<int, VarType>& argVars,
                  std::map<std::string, VarType>& localVars,
                  std::map<std::string, VarType>& globaleVars,
                  std::map<std::string, VarType>& persistentVars
                );

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class GlobaleVariableAST : public VariableAST
{
  public:
    GlobaleVariableAST(const std::string &);
    virtual ~GlobaleVariableAST();
    virtual llvm::Value* Codegen(Builder&);
    virtual llvm::Value* CodegenMute(Builder&, llvm::Value*);
  protected:
  private:
    std::string _name;
    
    virtual void _taggingPass(
                  std::map<int, VarType>& argVars,
                  std::map<std::string, VarType>& localVars,
                  std::map<std::string, VarType>& globaleVars,
                  std::map<std::string, VarType>& persistentVars
                );

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};

class PersistentVariableAST : public VariableAST
{
  public:
    PersistentVariableAST(const std::string &);
    virtual ~PersistentVariableAST();
    virtual llvm::Value* Codegen(Builder&);
    virtual llvm::Value* CodegenMute(Builder&, llvm::Value*);
  protected:
  private:
    std::string _name;

    virtual void _taggingPass(
                  std::map<int, VarType>& argVars,
                  std::map<std::string, VarType>& localVars,
                  std::map<std::string, VarType>& globaleVars,
                  std::map<std::string, VarType>& persistentVars
                );
    
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

    virtual void _taggingPass(
                  std::map<int, VarType>& argVars,
                  std::map<std::string, VarType>& localVars,
                  std::map<std::string, VarType>& globaleVars,
                  std::map<std::string, VarType>& persistentVars
                );
    
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

    virtual void _taggingPass(
                  std::map<int, VarType>& argVars,
                  std::map<std::string, VarType>& localVars,
                  std::map<std::string, VarType>& globaleVars,
                  std::map<std::string, VarType>& persistentVars
                );
    
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
    
    virtual void _taggingPass(
                  std::map<int, VarType>& argVars,
                  std::map<std::string, VarType>& localVars,
                  std::map<std::string, VarType>& globaleVars,
                  std::map<std::string, VarType>& persistentVars
                );

    virtual std::string _toString(const std::string& firstPrefix, const std::string& prefix) const;
};



#endif // AST_H
