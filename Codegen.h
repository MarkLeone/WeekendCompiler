#ifndef CODEGEN_H
#define CODEGEN_H

#include <memory>

class Program;
namespace llvm { class LLVMContext; class Module; }

std::unique_ptr<llvm::Module> Codegen( llvm::LLVMContext* context, const Program& program );

#endif
