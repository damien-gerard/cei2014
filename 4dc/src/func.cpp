#include "../include/func.h"
#include "../include/builder.h"
#include "../include/functionsignature.h"
#include "../include/ast.h"
#include "../include/util/util.h"
#include "../include/util/logger.h"

using namespace std;
using namespace llvm;

Func::Func(const string& name, BlocAST* body)
  : _name(name), _signature(nullptr), _body(body)
{}

Func::~Func()
{
  delete _body;
  if (_signature) delete _signature;
}

Function* Func::create(FunctionSignature* signature, Builder& b)
{
  assert(signature != nullptr);
  Logger::debug << "  Declaration de la fonction " << *signature << endl;
  
  Logger::debug << "    Creation de la signature... ";
  const string& name = signature->name();
  vector<Type*> argsType(signature->argsNumber());
  for (int i = 0; i < signature->argsNumber(); ++i) {
    argsType[i] = Type::getInt32Ty(b.context()); //should be return type.getType();
  }
  
  Type* retType = Type::getInt32Ty(b.context()); // should be signature->returnType().getType();
  
  
  FunctionType *FT = FunctionType::get(retType, argsType, false);
  assert(FT != nullptr);
  Logger::debug << "OK" << endl;
  
  Logger::debug << "    Creation de la fonction... ";
  Function *F = Function::Create(FT, Function::ExternalLinkage, name, &b.module());
  assert(F != nullptr);


  // If F conflicted, there was already something named 'Name'.  If it has a
  // body, don't allow redefinition or reextern.
  if (F->getName() != name) {
    // Delete the one we just made and get the existing one.
    F->eraseFromParent();
    F = b.module().getFunction(name);

    // If F already has a body, reject this.
    if (!F->empty()) {
      return AST::Error<Function>("redefinition of function");
    }

    // If F took a different number of args, reject.
    // TODO: check argsType sameness
    if (F->arg_size() != argsType.size()) {
      return AST::Error<Function>("redefinition of function with different # args");
    }
  }
  Logger::debug << "OK" << endl << "  Fin de la declaration" << endl;
  return F;
}

void Func::taggingPass(
    map<string, VarType>& globaleVars,
    map<string, VarType>& persistentVars
)
{
  Logger::debug << "  Passe de tag de la fonction " << _name << endl;
  map<int, VarType> argVars;
  
  Logger::debug << "    Parcours du corps de la fonction... ";
  this->_body->taggingPass(argVars, _localVars, globaleVars, persistentVars);
  Logger::debug << "OK" << endl;
  
  Logger::debug << "    Analyse des variables... ";
  int nbArgs = 0;
  for (auto& pairVar : argVars) {
    int num = pairVar.first;
    if (nbArgs < num) {
      nbArgs = num;
    }
  }
  
  //VarType type;
  VarType returnType = VarType::INT; // should be VOID
  VarType variadicArgsType = VarType::VOID;
  vector<VarType> argsType(nbArgs, VarType::INT); // should be VOID
  
  /*
  for (auto& pairVar : argVars) {
    int num = pairVar.first;
    type = pairVar.second;
    if (num == 0) {
      returnType = type;
    } else {
      argsType[num] = type;
    }
  }
  */
  Logger::debug << "OK" << endl;
  this->_signature = new FunctionSignature(this->_name, returnType, argsType, variadicArgsType);
  
  Logger::debug << "  Fin de la passe de tag" << endl;
}


Function* Func::Codegen(Builder& b)
{
  Logger::debug << "Compilation de la fonction " << *_signature << endl;
  assert(this->_signature != nullptr);
  Function *F = create(this->_signature, b);
  
  
  Logger::debug << "  Initialisation du bloc... ";
  // Create a new basic block to start insertion into.
  BasicBlock *block = BasicBlock::Create(b.context(), "entry", F);
  b.irbuilder().SetInsertPoint(block);
  b.currentBlock() = block;
  
  assert(F != nullptr);
  assert(block != nullptr);
  Logger::debug << "OK" << endl;
  
  
  Logger::debug << "  Initialisation des variables... ";
  int i = 1;
  AllocaInst *Alloca;
  
  b.localVars().clear();
  b.createAllocas(_localVars, b.localVars(), F);
  
  // Charge les arguments en mémoires
  for (auto& arg : F->args()) {
    if (_localVars.count(Util::toS(i))) {
      string s = Util::toS(i);
      Alloca = b.localVars()[s];
      assert(Alloca != nullptr);
      b.irbuilder().CreateStore(&arg, Alloca);
    }
    ++i;
  }
  Logger::debug << "OK" << endl;
  
  
  Logger::debug << "  Compilation du corps de la fonction... ";
  block = this->_body->Codegen(b, F);
  if (block) {
    Logger::debug << "OK" << endl;
    Logger::debug << "  Finalisation... ";
    b.irbuilder().SetInsertPoint(block);
    // Finish off the function.
    Value* retVal = ConstantInt::get(Type::getInt32Ty(b.context()), 0);
    if (b.localVars().count("0")) {
      Alloca = b.localVars()["0"];
      retVal = b.irbuilder().CreateLoad(Alloca);
    }
    b.irbuilder().CreateRet(retVal);
    Logger::debug << "OK" << endl;
    
    Logger::debug << "  Verification... ";
    // Validate the generated code, checking for consistency.
    verifyFunction(*F);
    Logger::debug << "OK" << endl;
    
    //b.dumpDebug(F);

    Logger::debug << "  Optimisation... ";
    b.optimize(F);
    Logger::debug << "OK" << endl;
    Logger::debug << "Fonction Compilee" << endl;

    return F;
  }

  return nullptr;
}


std::ostream& operator<<(std::ostream& out, const Func& f)
{
  if (f._signature) {
    return out << *f._signature << endl << *f._body << endl;
  }
  return out << f._name << endl << *f._body << endl;
}

