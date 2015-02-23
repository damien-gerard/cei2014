#include "../include/builder.h"
#include "../include/parser.h"
#include "../include/util/logger.h"
#include "../include/builtins.h"

using namespace llvm;

Builder::Builder()
  : Builder("Builder")
{}
Builder::Builder(const std::string& name)
  : _mod(new Module(name, getGlobalContext())),
    _irb(getGlobalContext()), _ctx(_mod->getContext()),
    _currentBlock(nullptr),
    _jit(nullptr), _optimizer(nullptr)
{}

Builder::~Builder()
{
  // Si le moteur JIT a été créé, il a pris le contrôle du module
  // et se charge donc de sa destruction
  if (this->_jit) {
    delete this->_jit;
  } else {
    delete this->_mod;
  }
  if (this->_optimizer) {
    delete this->_optimizer;
  }
}

void Builder::build(AST* ast)
{
  Function* F;
  DefinitionAST* def = nullptr;
  //BasicBlock* bloc = nullptr;
  
  declareBuiltins();
  PrototypeAST* proto = new PrototypeAST("", std::vector<std::string>());
  def = new FunctionAST(proto, dynamic_cast<BlocAST*>(ast));
  if (def) {
    std::cerr << "Dump: ";
    F = def->Codegen(*this);
    if (F) {
      F->dump();
    } else {
      std::cerr << "empty" << std::endl;
    }
    if (F && this->_jit) {
      std::cerr << "Execution:" << std::endl;
      void* fptr = this->_jit->getPointerToFunction(F);

      // cast to native function
      int (*f)() = (int (*)())(intptr_t)fptr;
      std::cerr << "Result: " << f() << std::endl;
    }
  }
  
  this->_mod->dump();
}

void Builder::createJIT()
{
  InitializeNativeTarget();
  std::string err;
  EngineBuilder eb = EngineBuilder(this->_mod);
  eb.setErrorStr(&err);
  auto jit = eb.create();
  this->_jit = jit;
  if (!jit) {
    Logger::critical << "Critical Error: Impossible de créer le moteur JIT : " << err << std::endl;
  }
}

void Builder::setOptimizer(FunctionPassManager* optimizer)
{
  this->_optimizer = optimizer;
}

FunctionPassManager* Builder::getStandardOptimizer()
{
  FunctionPassManager* optimizer = new FunctionPassManager(this->_mod);

  // Set up the optimizer pipeline.  Start with registering info about how the
  // target lays out data structures.
  if (this->_jit) {
    optimizer->add(new DataLayoutPass(*this->_jit->getDataLayout()));
  }
  // Provide basic AliasAnalysis support for GVN.
  optimizer->add(createBasicAliasAnalysisPass());
  // Do simple "peephole" optimizations and bit-twiddling optzns.
  optimizer->add(createInstructionCombiningPass());
  // Reassociate expressions.
  optimizer->add(createReassociatePass());
  // Eliminate Common SubExpressions.
  optimizer->add(createGVNPass());
  // Simplify the control flow graph (deleting unreachable blocks, etc).
  optimizer->add(createCFGSimplificationPass());

  optimizer->doInitialization();
  return optimizer;
}


void Builder::optimize(Function* f)
{
  if (this->_optimizer) {
    this->_optimizer->run(*f);
  }
}

void Builder::declareBuiltins()
{
  PrototypeAST* proto;
  Function* F;
  Builtin* builtin;
  for (auto& pBuiltin : Builtin::getList()) {
    builtin = &*pBuiltin.second;
    assert(builtin != nullptr);
    proto = new PrototypeAST(*builtin);
    F = proto->Codegen(*this);
    assert(F != nullptr);
    if (_jit) {
      _jit->addGlobalMapping(F, builtin->getPtr());
    }
  }
}
