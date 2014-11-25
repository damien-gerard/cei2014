#include "../include/builder.h"
#include "../include/parser.h"

using namespace llvm;

Builder::Builder(Parser& p)
: Builder("Builder", p, nullptr)
{}
Builder::Builder(Parser& p, ExecutionEngine* jit)
: Builder("Builder", p, jit)
{}
Builder::Builder(const std::string& name, Parser& p)
: Builder(name, p, nullptr)
{}
Builder::Builder(const std::string& name, Parser& p, ExecutionEngine* jit)
: _parser(p), _mod(new Module(name, getGlobalContext())), _irb(getGlobalContext()), _jit(jit)
{}

Builder::~Builder()
{
    delete this->_mod;
    this->_namedValues.clear();
}

void Builder::build() {
  Function* F;
  AST* ast = nullptr;
  DefinitionAST* def = nullptr;
  ExprAST* expr = nullptr;
  do {
    ast = this->_parser.parseLine();
    def = nullptr;
    expr = nullptr;
    if (!ast) {
        std::cerr << "Build Finished" << std::endl;
    } else if (typeid(*ast) == typeid(PrototypeAST)) {
        std::cerr << "Read extern:" << std::endl;
        def = static_cast<DefinitionAST*>(ast);
    } else if (typeid(*ast) == typeid(FunctionAST)) {
        std::cerr << "Read Function definition:" << std::endl;
        def = static_cast<DefinitionAST*>(ast);
    } else if (typeid(*ast) == typeid(DefinitionAST)) {
        std::cerr << "Definition error" << std::endl;
    } else {
        std::cerr << "Read expression:" << std::endl;
        expr = static_cast<ExprAST*>(ast);
    }

    if (expr) {
        PrototypeAST* proto = new PrototypeAST("", std::vector<std::string>());
        def = new FunctionAST(proto, expr);
    }
    if (def) {
        std::cerr << "Dump: ";
        F = def->Codegen(*this);
        if (F) {
          F->dump();
        } else {
          std::cerr << "empty" << std::endl;
        }
    }

    if (expr && this->_jit) {
      std::cerr << "Execution:" << std::endl;
      void* fptr = this->_jit->getPointerToFunction(F);

      // cast to native function
      double (*f)() = (double (*)())(intptr_t)fptr;
      std::cerr << "Result: " << f() << std::endl;
    }

  } while (ast);

  this->_mod->dump();
}

void Builder::createJIT() {
  InitializeNativeTarget();
  std::string err;
  EngineBuilder eb = EngineBuilder(this->_mod);
  eb.setErrorStr(&err);
  auto jit = eb.create();
  this->_jit = jit;
  if (!jit) {
    std::cerr << "Impossible de créer le moteur JIT : " << err << std::endl;
  }
}

