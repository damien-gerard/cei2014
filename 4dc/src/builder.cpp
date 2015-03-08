#include "../include/builder.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/func.h"
#include "../include/functionsignature.h"
#include "../include/util/logger.h"
#include "../include/builtins.h"

using namespace std;
using namespace llvm;

Builder::Builder()
  : Builder("Builder")
{}
Builder::Builder(const string& name)
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

void Builder::buildAll(const vector<pair<string,File>>& files)
{
  // Création de l'objet Builder qui va permettre de construire le programme
  Builder builder;
  builder.createJIT();
  builder.setOptimizer(builder.getStandardOptimizer());
  
  // listes des variables globales et persistantes
  map<string, VarType> globalVars;
  map<string, VarType> persistentVars;
  
  // Liste des fonctions à compiler
  vector<Func*> functionsDef(files.size());
  vector<Function*> functions(files.size()); // LLVM
  
  const string* name;
  const File* file;
  
  // Parcours de la liste des fichhiers pour parser les fonctions correspondantes
  for (unsigned int i = 0; i < files.size(); ++i) {
    name = &files[i].first;
    file = &files[i].second;
    // Si le "fichier" correspond à l'entrée standard, on choisit le flux d'entrée standard
    if (file->isStdin()) {
      functionsDef[i] = builder.parse(globalVars, persistentVars, *name, cin);
      
    // Sinon, on tente d'ouvrir le fichier
    } else {
      ifstream in(file->filename());
      if (!in) {
        Logger::error << "Cannot read file \"" << file->filename() << "\"" << endl;
        exit(EXIT_FAILURE);
      }
      Logger::debug << "\"" << file->filename() << "\":" << endl;
      functionsDef[i] = builder.parse(globalVars, persistentVars, *name, in);
    }
    Logger::debug << *functionsDef[i];
  }
  
  // Déclaration de toutes les BUILTINs existants
  Logger::debug << "Declaration des BUILTINs" << endl;
  builder.declareBuiltins();
  Logger::debug << "Fin de la declaration des BUILTINs" << endl << endl;
  
  Logger::debug << "Declaration des variables globales" << endl
                << "  globales     : ";
  builder.createGlobals(globalVars, builder.globalVars()); // Declare global variables
  
  Logger::debug << endl 
                << "  persistentes : ";
  builder.createGlobals(persistentVars, builder.persistentVars()); // Declare persistent variables
  
  Logger::debug << endl << "Fin de la declaration des variables globales" << endl << endl;
  
  for (unsigned int i = 0; i < files.size(); ++i) {
    functions[i] = builder.build(functionsDef[i]);
  }
  Function* main = builder.createMain(functionsDef.back()->signature(), functions.back());
  
  Logger::debug << endl << "Toutes les fonctions ont ete compilees avec succes !" << endl << endl;
  
  for (unsigned int i = 0; i < files.size(); ++i) {
    delete functionsDef[i];
  }
  
  builder._mod->dump();
  
  cout << endl;
  
  builder.callFunctionLLVM(main);
}


Func* Builder::parse(
    map<string, VarType>& globalVars,
    map<string, VarType>& persistentVars,
    const string& name,
    istream& in
)
{
  // Parse le fichier
  if (&in == &cin) {
    Logger::debug << "Entree clavier en attente :" << endl;
  } else {
    Logger::debug << "Parse la fonction " << name << endl;
    Logger::debug << "  parse... ";
  }
  Lexer lexer(in);
  Parser parser(lexer);
  BlocAST* ast = parser.parse();
  
  if (!ast) {
    exit(EXIT_FAILURE);
  }
  
  if (&in == &cin) {
    Logger::debug << "Parse la fonction " << name << endl;
    Logger::debug << "  parse clavier OK" << endl;
  } else {
    Logger::debug << "OK" << endl;
  }
  
  Func* Fdef = new Func(name, ast);
  if (Fdef) {
    Fdef->taggingPass(globalVars, persistentVars);
  } else {
    Logger::error << "Error: Couldn't parse function \"" << name << "\"" << endl;
    exit(EXIT_FAILURE);
  }
  Logger::debug << "Fin du parse" << endl << endl;
  return Fdef;
}

llvm::Function* Builder::build(Func* Fdef)
{
  assert(Fdef != nullptr);
  Function *F = Fdef->Codegen(*this);
  if (F) {
    Logger::debug << endl << "Dump: ";
    dumpDebug(F);
  } else {
    Logger::error << "Build Error: Couldn't build function \"" << Fdef->name() << "\"" << endl;
    exit(EXIT_FAILURE);
  }
  return F;
}

void Builder::callFunctionLLVM(Function* F)
{
  assert(F != nullptr);
  if (F && this->_jit) {
    Logger::info << "Execution :" << endl;
    void* fptr = this->_jit->getPointerToFunction(F);

    // cast to native function
    int (*f)() = (int (*)())(intptr_t)fptr;
    Logger::info << "=> " << f() << endl;
  }
}

Function* Builder::createMain(FunctionSignature* signature, Function* F)
{
  Logger::debug << "Creation de la fonction main" << endl;
  assert(signature != nullptr);
  assert(F != nullptr);
  
  if (signature->argsNumber() != 0) {
    Logger::error << "Build Error: " << *signature << " must take 0 argument" << endl;
    exit(EXIT_FAILURE);
  }
  
  string name = module().getFunction("main") ? "/main" : "main";
  
  FunctionSignature* mainSig = new FunctionSignature(name, VarType::INT);
  assert(mainSig != nullptr);
  
  Function* main = Func::create(mainSig, *this);
  assert(main != nullptr);
  
  BasicBlock* block = BasicBlock::Create(context(), "main.entry", main);
  assert(block != nullptr);
  
  irbuilder().SetInsertPoint(block);
  currentBlock() = block;
  
  Value* retVal = irbuilder().CreateCall(F, std::vector<Value*>{}, "calltmp");
  assert(retVal != nullptr);
  
  irbuilder().CreateRet(retVal);
  Logger::debug << "  Verification... ";
  verifyFunction(*main);
  Logger::debug << "OK" << endl;
  Logger::debug << "  Optimisation... ";
  optimize(main);
  Logger::debug << "OK" << endl;
  Logger::debug << "Fonction main OK" << endl;
  return main;
}

void Builder::createJIT()
{
  InitializeNativeTarget();
  string err;
  EngineBuilder eb = EngineBuilder(this->_mod);
  eb.setErrorStr(&err);
  auto jit = eb.create();
  this->_jit = jit;
  if (!jit) {
    Logger::critical << "Critical Error: Impossible de creer le moteur JIT : " << err << endl;
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
  // Promote allocas to registers.
  optimizer->add(createPromoteMemoryToRegisterPass());
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
  Function* F;
  Builtin* builtin;
  for (auto& pBuiltin : Builtin::getList()) {
    builtin = &*pBuiltin.second;
    assert(builtin != nullptr);
    F = Func::create(builtin->signature(), *this);
    assert(F != nullptr);
    if (_jit) {
      _jit->addGlobalMapping(F, builtin->getPtr());
    }
  }
}


AllocaInst* Builder::createEntryBlockAlloca(Function *F, const string& name, Type* type)
{
  IRBuilder<> TmpB(&F->getEntryBlock(),
                 F->getEntryBlock().begin());
  return TmpB.CreateAlloca(type, 0, name.c_str());
}


void Builder::createAllocas(
          map<string, VarType>& types,
          map<string, AllocaInst*>& vals,
          Function* F
)
{
  AllocaInst *Alloca ;
  string varName;
  Type* varType = Type::getInt32Ty(context());
  for (auto& varPair : types) {
    varName = varPair.first;
    //varType = varPair.second.getType();
    if (vals.count(varName) == 0) {
      Alloca = createEntryBlockAlloca(F, varName, varType);
      vals[varName] = Alloca;
    }
  }
}


void Builder::createGlobals(
          map<string, VarType>& types,
          map<string, GlobalVariable*>& vals
)
{
  bool comma = false;
  string name;
  Type* type;
  for (auto& var : types) {
    name = var.first;
    type = Type::getInt32Ty(context()); //should be return var.second.getType();
    if (comma) {
      Logger::debug << ", ";
    } else {
      comma = true;
    }
    Logger::debug << name;
    if (vals.count(name) == 0) {
       vals[name] = new GlobalVariable(
                        this->module(),
                        type,
                        false,
                        GlobalValue::ExternalLinkage,
                        ConstantInt::get(Type::getInt32Ty(context()), 0),
                        name
      );
    } // else -> type verification
  }
}


void Builder::dumpDebug()
{
  if (&Logger::debug == &cout) {
    _mod->dump();
  }
}
void Builder::dumpDebug(llvm::Function* F)
{
  if (&Logger::debug == &cout) {
    F->dump();
  }
}

