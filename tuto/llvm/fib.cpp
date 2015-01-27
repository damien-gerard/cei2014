#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <cstdint>





namespace // anonymous
{

	class Program final
	{
	public:
		Program();
		int compileAndRun(std::int64_t x);

	private:
		llvm::Function* createFuncMain();
		llvm::Function* createFuncPrintf();
		llvm::ConstantInt* constantInt64(std::int64_t value);
		llvm::Function* createFuncFibonacci();

	private:
		llvm::LLVMContext& ctx;
		llvm::IRBuilder<> builder;
		llvm::Module* module;

	}; // class Program





	inline Program::Program()
		: ctx(llvm::getGlobalContext())
		, builder(ctx)
		, module(new llvm::Module("fibo_demo", ctx))
	{}


	inline llvm::Function* Program::createFuncMain()
	{
		std::vector<llvm::Type*> arg_types;
		// int32
		auto* rettype = llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), arg_types, false);
		// func name
		// const auto& twine = llvm::Twine("main");

		// main
		auto* func = llvm::Function::Create(rettype, llvm::Function::ExternalLinkage, "main", module);
		func->setCallingConv(llvm::CallingConv::C);
		return func;
	}


	inline llvm::Function* Program::createFuncPrintf()
	{
		std::vector<llvm::Type*> arg_types;
		arg_types.push_back(llvm::Type::getInt8PtrTy(ctx));

		auto* rettype = llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), arg_types, true);
		// func name
		//const auto& twine = llvm::Twine("printf");

		auto* func = llvm::Function::Create(rettype, llvm::Function::ExternalLinkage, "printf", module);
		func->setCallingConv(llvm::CallingConv::C);
		return func;
	}


	inline llvm::ConstantInt* Program::constantInt64(std::int64_t value)
	{
		return llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx), value);
	}


	inline llvm::Function* Program::createFuncFibonacci()
	{
		auto* rettype = llvm::Type::getInt64Ty(ctx);
		auto* paramXType = llvm::ArrayType::getInt64Ty(ctx);
		auto* func = llvm::cast<llvm::Function>(module->getOrInsertFunction("fib", rettype, paramXType, NULL));

		llvm::Argument* paramX = func->arg_begin();
		paramX->setName("x");


		auto* fnScope = llvm::BasicBlock::Create(ctx, "init", func);
		builder.SetInsertPoint(fnScope);


		// if-statement
		//
		//                   [ condition ]
		//                         |
		//         --------------------------------
		//        |                               |
		//       then                            else
		//        |                               |
		//        ---------------------------------
		//                         |
		//              continuation (merge - phi)
		//         code executed after 'then' or 'else'
		//

		// emit condition instructions
		// the last one is the result of the condition
		// it could be more complicated of course if 'and' / 'or' are present

		// x <= 2
		llvm::Value* condInst = new llvm::ICmpInst(*fnScope, llvm::ICmpInst::ICMP_SLE, paramX, constantInt64(2), "lt.2");


		// declaring all 3 blocks
		auto* thenBlk  = llvm::BasicBlock::Create(ctx, "fib_then", func);
		auto* elseBlk  = llvm::BasicBlock::Create(ctx, "fib_else");
		auto* mergeBlk = llvm::BasicBlock::Create(ctx, "fib_merge");
		// emit jump to 'then' or 'else' block
		builder.CreateCondBr(condInst, thenBlk, elseBlk);



		// emit 'then' instructions
		auto* thenValue = ([&]() -> llvm::Value*
		{
			// start block
			builder.SetInsertPoint(thenBlk);

			// --- start code
			// llvm::ReturnInst::Create(ctx, constantInt64(1));
			auto* contant = constantInt64(1);
			// --- end code

			// 'then' block is finished - tell to jump to the continuation block
			builder.CreateBr(mergeBlk);
			// the code emitted within the 'else' block may include other new blocks
			// (not in this exemple but this may happen in some more complex code)
			thenBlk = builder.GetInsertBlock();

			return contant;
		})();


		// emit 'else' instructions
		auto* elseValue = ([&]() -> llvm::Value*
		{
			func->getBasicBlockList().push_back(elseBlk);
			builder.SetInsertPoint(elseBlk);

			// --- start code
			// compute fib(x - 1)
			auto* exprFibMinus1 = ([&]() -> llvm::CallInst* {
				auto* sub = llvm::BinaryOperator::CreateSub(paramX, constantInt64(1), "arg", elseBlk);
				auto* expr = llvm::CallInst::Create(func, sub, "fib_expr1", elseBlk);
				expr->setTailCall(); // special case for fibo
				return expr;
			})();

			// compute fib(x - 2) - should be merged with the previous one
			auto* exprFibMinus2 = ([&]() -> llvm::CallInst* {
				auto* sub = llvm::BinaryOperator::CreateSub(paramX, constantInt64(2), "arg", elseBlk);
				auto* expr = llvm::CallInst::Create(func, sub, "fib_expr2", elseBlk);
				expr->setTailCall(); // special case for fibo
				return expr;
			})();

			// compute fib(x - 1) + fib(x - 2)
			auto* sum = llvm::BinaryOperator::CreateAdd(exprFibMinus1, exprFibMinus2, "sumres", elseBlk);

			// return the value
			// llvm::ReturnInst::Create(ctx, sum, elseBlk);
			// --- end code

			// 'else' block is finished - tell to jump to the continuation block
			builder.CreateBr(mergeBlk);
			// the code emitted within the 'else' block may include other new blocks
			// (not in this exemple but this may happen in some more complex code)
			elseBlk = builder.GetInsertBlock();

			return sum;
		})();


		// emit 'merge' block
		{
			func->getBasicBlockList().push_back(mergeBlk);
			builder.SetInsertPoint(mergeBlk);

			// select a result to return, which can be void
			auto* phi = builder.CreatePHI(rettype, 2, "iftmp");
			phi->addIncoming(thenValue, thenBlk);
			phi->addIncoming(elseValue, elseBlk);

			// return the value
			llvm::ReturnInst::Create(ctx, phi, mergeBlk);
		}

		return func;
	}




	inline int Program::compileAndRun(std::int64_t x)
	{
		auto* fibonacci  = createFuncFibonacci();

		std::string err;
		llvm::ExecutionEngine* engine = llvm::EngineBuilder(module).setErrorStr(&err)
			.setEngineKind(llvm::EngineKind::JIT).create();

		if (nullptr == engine)
		{
			std::cerr << "failed to create engine: " << err << "\n";
			return EXIT_FAILURE;
		}

		if (llvm::verifyModule(*module, &llvm::outs()))
		{
			std::cerr << "[!!] failed to compile: " << err << '\n';
			return EXIT_FAILURE;
		}


		std::vector<llvm::GenericValue> args(1);
		args[0].IntVal = llvm::APInt(64, x);
		llvm::GenericValue retvalue = engine->runFunction(fibonacci, args);

		llvm::outs() << "fib(" << x << ") = " << retvalue.IntVal << '\n';
		return EXIT_SUCCESS;
	}



} // anonymous namespace





int main(int argc, char**argv)
{
	llvm::InitializeNativeTarget();

	Program program;
	std::int64_t x = argc > 1 ? (std::int64_t) std::atoll(argv[1]) : 10;
	return program.compileAndRun(x);
}


