#ifndef BUILDER_H
#define BUILDER_H

#include "ast.h"
#include <map>
#include <string>

/*
namespace llvm {
    class Module;
    template<>
    class IRBuilder;
}

*/
class Builder
{
    public:
        Builder();
        virtual ~Builder();
    protected:
    private:
        llvm::Module* _mod;
        llvm::IRBuilder _irb;
        std::map<std::string, llvm::Value*> _namedValues;
};

#endif // BUILDER_H
