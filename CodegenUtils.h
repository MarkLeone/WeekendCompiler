#ifndef CODEGEN_UTILS_H
#define CODEGEN_UTILS_H

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>

class CodegenUtils
{
public:
    CodegenUtils(llvm::LLVMContext* context,
		 llvm::Module* module) :
	m_context(context),
	m_module(module),
	m_builder(*context)
    {
    }

    llvm::Type* GetInt32Type() const
    {
	return llvm::IntegerType::get(*m_context, 32);
    }
    
    llvm::Constant* MakeInt32(int value)
    {
	return llvm::ConstantInt::get(GetInt32Type(), value, true);
    }
    
private:
    llvm::LLVMContext* m_context;
    llvm::Module* m_module;
    llvm::IRBuilder<> m_builder;
};

#endif
