#pragma once

#include <memory>

class Program;
namespace llvm { class LLVMContext; class Module; }

// Generate LLVM IR for the given program.
std::unique_ptr<llvm::Module> Codegen( llvm::LLVMContext* context, const Program& program );
