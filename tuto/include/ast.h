#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <iostream>

class AST
{
    public:
        virtual ~AST() = default;
        template<class T = AST>
        static T* Error(const std::string& msg);
    protected:
    private:
};


template<class T>
T* AST::Error(const std::string& msg) {
    std::cerr << msg << std::endl;
    return nullptr;
}
/*
class BlocExprAST : public AST
{
    public:
        BlocExprAST(ExprAST*);
        BlocExprAST(const std::vector<ExprAST*>&);
        virtual ~BlocExprAST();
    protected:
    private:
        std::vector<ExprAST*> _exprs;
}
*/
class ExprAST : public AST
{
    public:
        virtual ~ExprAST() = default;
    protected:
    private:
};

class NumberAST : public ExprAST
{
    public:
        NumberAST(double);
        virtual ~NumberAST();
    protected:
    private:
        double _val;
};

class VariableAST : public ExprAST
{
    public:
        VariableAST(const std::string &);
        virtual ~VariableAST();
    protected:
    private:
        std::string _name;
};

class OpAST : public ExprAST
{
    public:
        OpAST(char op, ExprAST* lhs, ExprAST* rhs);
        virtual ~OpAST();
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
    protected:
    private:
        std::string _name;
        std::vector<ExprAST*> _args; // delete at destruction
};


class DefinitionAST : public AST
{
    public:
        virtual ~DefinitionAST() = default;
    protected:
    private:
};

class PrototypeAST : public DefinitionAST
{
    public:
        PrototypeAST(const std::string&, const std::vector<std::string>&);
        virtual ~PrototypeAST();
    protected:
    private:
        std::string _name;
        std::vector<std::string> _args;
};

class FunctionAST : public DefinitionAST
{
    public:
        FunctionAST(PrototypeAST*, ExprAST*);
        //FunctionAST(PrototypeAST*, BlocExprAST*);
        virtual ~FunctionAST();
    protected:
    private:
        PrototypeAST* _proto; // keep at destruction
        ExprAST* _body; // delete at destruction
};


#endif // AST_H
