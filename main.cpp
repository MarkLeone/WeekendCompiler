#include "Codegen.h"
#include "FuncDef.h"
#include "InputBuffer.h"
#include "Parser.h"
#include "Printer.h"
#include "Program.h"
#include "SimpleJIT.h"
#include "TokenStream.h"
#include "Typechecker.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/raw_ostream.h>

#include <iostream>

void dumpSyntax( const Program& program, const char* srcFilename )
{
    if ( !getenv("ENABLE_DUMP") )
        return;
    std::string   filename( std::string( srcFilename ) + ".syn" );
    std::ofstream out( filename );
    out << program << std::endl;
}

void dumpIR( llvm::Module& module, const char* srcFilename, const char* what )
{
    if ( !getenv("ENABLE_DUMP") )
        return;
    std::string   filename( std::string( srcFilename ) + what + ".ll" );
    std::ofstream stream( filename );
    llvm::raw_os_ostream out( stream );
    out << module;
}

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
    dumpSyntax( *program, filename );

    int status = Typecheck( *program );
    if( status )
        return status;

    // Generate LLVM IR.
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module( Codegen( &context, *program ) );
    dumpIR( *module, filename, "initial" );

    // Construct JIT engine and use data layout for target-specific optimizations.
    SimpleJIT jit;
    module->setDataLayout( jit.getTargetMachine().createDataLayout() );

    // Optimize the module.
    legacy::FunctionPassManager functionPasses( module.get() );
    legacy::PassManager         modulePasses;
    PassManagerBuilder          builder;
    builder.OptLevel = 2;
    builder.populateFunctionPassManager( functionPasses );
    builder.populateModulePassManager( modulePasses );
    for( Function& function : *module )
        functionPasses.run( function );
    modulePasses.run( *module );
    dumpIR( *module, filename, "optimized" );

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
