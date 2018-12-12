#pragma once

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JITSymbol.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/LambdaResolver.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>

#include <memory>
#include <string>

using namespace llvm;
using namespace llvm::orc;

/// A simple JIT engine that encapsulates the LLVM ORC JIT API.  Adapted from
/// the LLVM KaleidoscopeJIT example.
class SimpleJIT {
public:
    /// Construct JIT engine, initializing the resolver, object layer, and compile layer.
    SimpleJIT() :
        m_initialized( init() ),
        m_resolver
        (createLegacyLookupResolver
           ( m_session,
             [this](const std::string& name) {
                 return m_objectLayer.findSymbol(name, true);
             },
             [](Error Err) { cantFail(std::move(Err), "lookupFlags failed"); })),
          m_target(EngineBuilder().selectTarget()), m_dataLayout(m_target->createDataLayout()),
          m_objectLayer(m_session,
                      [this](VModuleKey)
                      { return ObjLayerT::Resources
                              { std::make_shared<SectionMemoryManager>(), m_resolver};
                      }),
          m_compileLayer(m_objectLayer, SimpleCompiler(*m_target))
    {
        llvm::sys::DynamicLibrary::LoadLibraryPermanently( nullptr );
    }

    /// Get the TargetMachine, which can be used for target-specific optimizations.
    TargetMachine& getTargetMachine() { return *m_target; }

    /// Add the given module to the JIT engine, yielding a key that can be
    /// used for subsequent symbol lookups.
    VModuleKey addModule( std::unique_ptr<Module> module )
    {
        VModuleKey key = m_session.allocateVModule();
        cantFail( m_compileLayer.addModule( key, std::move( module ) ) );
        return key;
    }

    /// Remove the module with the specified key from the JIT engine.
    void removeModule( VModuleKey key )
    {
        cantFail( m_compileLayer.removeModule( key ) );
    }

    /// Find the specified symbol in the module with the given key.
    JITSymbol findSymbol( VModuleKey key, const std::string name )
    {
        return m_compileLayer.findSymbolIn( key, name, false /*ExportedSymbolsOnly*/ );
    }

private:
    using ObjLayerT     = RTDyldObjectLinkingLayer;
    using CompileLayerT = IRCompileLayer<ObjLayerT, SimpleCompiler>;

    bool                            m_initialized;
    ExecutionSession                m_session;
    std::shared_ptr<SymbolResolver> m_resolver;
    std::unique_ptr<TargetMachine>  m_target;
    const DataLayout                m_dataLayout;
    ObjLayerT                       m_objectLayer;
    CompileLayerT                   m_compileLayer;

    // Perform prerequisite initialization.
    bool init()
    {
        InitializeNativeTarget();
        InitializeNativeTargetAsmPrinter();
        InitializeNativeTargetAsmParser();
        return true;
    }
    
};
