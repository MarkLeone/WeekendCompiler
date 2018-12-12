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

#ifndef OPT_LEVEL
/// Optimization level, which defaults to -O2.
#define OPT_LEVEL 2
#endif

namespace {
    
// Forward declarations.
void optimize( Module* module, int optLevel );
int  readFile( const char* filename, std::vector<char>* buffer );
void dumpSyntax( const Program& program, const char* srcFilename );
void dumpIR( llvm::Module& module, const char* srcFilename, const char* what );

// Parse and typecheck the given source code, adding definitions to the given Program.
// This is used to process both builtin definitions and user code.
int parseAndTypecheck( const char* source, Program* program )
{
    // Construct token stream, which encapsulates the lexer.  \see TokenStream.
    TokenStream tokens( source );

    // Parse the token stream into a program.
    int status = ParseProgram( tokens, program );

    // If the parser succeeded, typecheck the program.
    if( status == 0 )
        status = Typecheck( *program );
    return status;
}

} // anonymous namespace


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

    // Read source file.  TODO: use an input stream, rather than reading the entire file.
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
    optimize( module.get(), OPT_LEVEL );
    dumpIR( *module, filename, "optimized" );

    // Use the JIT engine to generate native code.
    VModuleKey key = jit.addModule( std::move(module) );

    // Get the main function pointer.
    JITSymbol mainSymbol = jit.findSymbol( key, "main" );
    typedef int ( *MainFunc )( int );
    MainFunc mainFunc = reinterpret_cast<MainFunc>( cantFail( mainSymbol.getAddress() ) );

    // Call the main function using the input value from the command line.
    int result = mainFunc(inputValue);
    std::cout << result << std::endl;
    
    return 0;
}

namespace {

// Optimize the module using the given optimization level (0 - 3).
void optimize( Module* module, int optLevel )
{
    // Construct the function and module pass managers, which are populated
    // with standard optimizations (e.g. constant propagation, inlining, etc.)
    legacy::FunctionPassManager functionPasses( module );
    legacy::PassManager         modulePasses;

    // Populate the pass managers based on the optimization level.
    PassManagerBuilder builder;
    builder.OptLevel = optLevel;
    builder.populateFunctionPassManager( functionPasses );
    builder.populateModulePassManager( modulePasses );

    // Run the function passes, then the module passes.
    for( Function& function : *module )
        functionPasses.run( function );
    modulePasses.run( *module );
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

// Dump syntax for debugging if the "ENABLE_DUMP" environment variable is set.
void dumpSyntax( const Program& program, const char* srcFilename )
{
    if ( !getenv("ENABLE_DUMP") )
        return;
    std::string   filename( std::string( srcFilename ) + ".syn" );
    std::ofstream out( filename );
    out << program << std::endl;
}

// Dump LLVM IR for debugging if the "ENABLE_DUMP" environment variable is set.
void dumpIR( llvm::Module& module, const char* srcFilename, const char* what )
{
    if ( !getenv("ENABLE_DUMP") )
        return;
    std::string   filename( std::string( srcFilename ) + "." + what + ".ll" );
    std::ofstream stream( filename );
    llvm::raw_os_ostream out( stream );
    out << module;
}

} // anonymous namespace
