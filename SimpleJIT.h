#pragma once

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>

#include <memory>
#include <string>

using namespace llvm;
using namespace llvm::orc;

/// A simple JIT engine that encapsulates the LLVM ORC JIT API.
/// (JIT = Just In Time, ORC = On Request Compilation)
class SimpleJIT {
public:
    /// Construct JIT engine, initializing the execution session and layers.
    SimpleJIT() : m_initialized(false), m_jit(nullptr) {
        m_initialized = init();
        if (m_initialized) {
            llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
        }
    }

    /// Initialize LLVM target infrastructure. This is safe to call multiple times.
    /// Returns true if initialization was successful or already done.
    static bool initializeLLVM() {
        static bool initialized = false;
        if (initialized) {
            return true;
        }
        
        InitializeNativeTarget();
        InitializeNativeTargetAsmPrinter();
        InitializeNativeTargetAsmParser();
        
        initialized = true;
        return true;
    }


    /// Add the given module to the JIT engine.
    Error addModule(std::unique_ptr<Module> module) {
        if (!m_initialized) {
            return make_error<StringError>("JIT not initialized", inconvertibleErrorCode());
        }

        // Create a new context for this module
        auto context = std::make_unique<LLVMContext>();
        ThreadSafeModule tsm(std::move(module), std::move(context));
        return m_jit->addIRModule(std::move(tsm));
    }

    /// Find the specified symbol in the JIT.
    Expected<ExecutorAddr> findSymbol(const std::string& name) {
        if (!m_initialized) {
            return make_error<StringError>("JIT not initialized", inconvertibleErrorCode());
        }

        return m_jit->lookup(name);
    }

private:
    bool m_initialized;
    std::unique_ptr<LLJIT> m_jit;

    // Perform prerequisite initialization.
    bool init() {
        // Ensure LLVM target infrastructure is initialized
        if (!initializeLLVM()) {
            return false;
        }

        auto jitOrError = LLJITBuilder().create();
        if (!jitOrError) {
            return false;
        }

        m_jit = std::move(*jitOrError);

        return true;
    }
};
