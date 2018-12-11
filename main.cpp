#include "Builtins.h"
#include "Codegen.h"
#include "FuncDef.h"
#include "Parser.h"
#include "Printer.h"
#include "Program.h"
#include "SimpleJIT.h"
#include "TokenStream.h"
#include "Typechecker.h"

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/raw_ostream.h>

#include <fstream>
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
    std::string   filename( std::string( srcFilename ) + "." + what + ".ll" );
    std::ofstream stream( filename );
    llvm::raw_os_ostream out( stream );
    out << module;
}

// Read file into the given buffer.  Returns zero for success.
int readFile( const char* filename, std::vector<char>* buffer )
{
    // Open the stream at the end, get file size, and allocate data.
    std::ifstream in( filename, std::ifstream::ate | std::ifstream::binary );
    if( in.fail() )
        return -1;
    size_t length = static_cast<size_t>( in.tellg() );

    buffer->resize( length + 1 );

    // Rewind and read entire file
    in.clear();  // clear EOF
    in.seekg( 0, std::ios::beg );
    in.read( buffer->data(), length );

    // The buffer is null-terminated (for the benefit of the Lexer).
    (*buffer)[length] = '\0';
    return 0;
}

int parseAndTypecheck( const char* buffer, Program* program )
{
    TokenStream tokens( buffer );
    int status = ParseProgram( tokens, program );
    if( status == 0 )
        status = Typecheck( *program );
    return status;
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

    // Read source file
    std::vector<char> source;
    int status = readFile( argv[1], &source );
    if( status != 0 )
    {
        std::cerr << "Unable to open input file: " << argv[1] << std::endl;
        return status;
    }

    // Parse and typecheck builtin functions.
    ProgramPtr  program( new Program );
    status = parseAndTypecheck( GetBuiltins(), program.get() );
    assert(status == 0);

    // Parse and typecheck user source code.
    status = parseAndTypecheck( source.data(), program.get());
    if( status )
        return status;
    dumpSyntax( *program, filename );

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
