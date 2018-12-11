#include "Codegen.h"
#include "FuncDef.h"
#include "InputBuffer.h"
#include "Parser.h"
#include "Printer.h"
#include "Program.h"
#include "SimpleJIT.h"
#include "TokenStream.h"
#include "Typechecker.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>

#include <iostream>


int main( int argc, const char* const* argv )
{
    // Get command-line arguments.
    if( argc != 3 )
    {
        std::cerr << "Usage: " << argv[0] << " <filename> <inputValue>" << std::endl;
        return -1;
    }
    const char* filename = argv[1];
    int inputValue = atoi( argv[2] );

    InputBuffer in( filename );
    if( !in.IsValid() )
    {
        std::cerr << "Unable to open input file: " << argv[1] << std::endl;
        return -1;
    }

    TokenStream tokens( in.Get() );
    ProgramPtr  program( ParseProgram( tokens ) );
    if( !program )
        return -1;  // Error already reported

    int status = Typecheck( *program );
    if( status )
        return status;
    // std::cout << *program << std::endl;

    // Generate LLVM IR.
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module( Codegen( &context, *program ) );
    // llvm::outs() << *module;

    // Construct the JIT engine.
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();
    SimpleJIT jit;

    // Optimize the module.  Using the data layout from the JIT engine allows
    // target-specific optimizations.
    module->setDataLayout(jit.getTargetMachine().createDataLayout());
    
    // Use the JIT engine to generate native code.
    VModuleKey key = jit.addModule( std::move(module) );

    // Get the main function pointer.
    JITSymbol mainSymbol = jit.findSymbol( key, "main" );
    typedef int ( *MainFunc )( int );
    MainFunc mainFunc = reinterpret_cast<MainFunc>( cantFail( mainSymbol.getAddress() ) );

    // Call the main function using the input value from the command line.
    int result = mainFunc(inputValue);
    printf("%i\n", result);
    
    return 0;
}
