#include "Builtins.h"
#include "Codegen.h"
#include "FuncDef.h"
#include "Parser.h"
#include "Printer.h"
#include "Program.h"
#include "SimpleJIT.h"
#include "TokenStream.h"
#include "Typechecker.h"

#include <llvm/IR/PassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/TargetParser/Host.h>

#include <fstream>
#include <iostream>
#include <string>

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
    // Initialize LLVM target infrastructure early
    SimpleJIT::initializeLLVM();
    
    // Get command-line arguments.
    if( argc < 3 || argc > 4 )
    {
        std::cerr << "Usage: " << argv[0] << " [-O0|-O1|-O2|-O3] <filename> <inputValue>" << std::endl;
        std::cerr << "  -O0: no optimization, -O1: basic, -O2: default, -O3: aggressive" << std::endl;
        return -1;
    }
    
    // Parse optimization level and arguments
    int optLevel = OPT_LEVEL; // default
    const char* filename;
    int inputValue;
    
    if( argc == 3 ) {
        // No optimization flag provided, use default
        filename = argv[1];
        inputValue = atoi( argv[2] );
    } else {
        // Optimization flag provided
        std::string optArg = argv[1];
        if( optArg == "-O0" ) optLevel = 0;
        else if( optArg == "-O1" ) optLevel = 1;
        else if( optArg == "-O2" ) optLevel = 2;
        else if( optArg == "-O3" ) optLevel = 3;
        else {
            std::cerr << "Invalid optimization level: " << optArg << std::endl;
            std::cerr << "Use -O0, -O1, -O2, or -O3" << std::endl;
            return -1;
        }
        filename = argv[2];
        inputValue = atoi( argv[3] );
    }

    // Read source file.  TODO: use an input stream, rather than reading the entire file.
    std::vector<char> source;
    int status = readFile( filename, &source );
    if( status != 0 )
    {
        std::cerr << "Unable to open input file: " << filename << std::endl;
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

    // Verify the module, which catches malformed instructions and type errors.
    assert(!verifyModule(*module, &llvm::errs()));

    // Construct JIT engine.
    SimpleJIT jit;
    // Note: Data layout is automatically handled by LLJIT in LLVM 19

    // Optimize the module.
    optimize( module.get(), optLevel );
    dumpIR( *module, filename, "optimized" );

    // Use the JIT engine to generate native code.
    auto addResult = jit.addModule( std::move(module) );
    if (addResult) {
        std::cerr << "Failed to add module to JIT: " << toString(std::move(addResult)) << std::endl;
        return -1;
    }

    // Get the main function pointer.
    auto mainSymbolResult = jit.findSymbol( "main" );
    if (!mainSymbolResult) {
        std::cerr << "Failed to find main symbol: " << toString(mainSymbolResult.takeError()) << std::endl;
        return -1;
    }
    
    typedef int ( *MainFunc )( int );
    MainFunc mainFunc = reinterpret_cast<MainFunc>( mainSymbolResult->getValue() );

    // Call the main function using the input value from the command line.
    int result = mainFunc(inputValue);
    std::cout << result << std::endl;
    
    return 0;
}

namespace {

// Optimize the module using the given optimization level (0 - 3).
void optimize( Module* module, int optLevel )
{
    // Ensure LLVM target infrastructure is initialized
    SimpleJIT::initializeLLVM();
    
    // Skip optimization for O0
    if (optLevel == 0) {
        return;
    }
    
    // Create a simple target machine for optimization
    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    if (!target) {
        llvm::errs() << "Warning: Could not find target for optimization: " << error << "\n";
        return;
    }
    
    auto targetMachine = target->createTargetMachine(
        targetTriple, "generic", "", llvm::TargetOptions{}, std::nullopt);
    if (!targetMachine) {
        llvm::errs() << "Warning: Could not create target machine for optimization\n";
        return;
    }
    
    // Create analysis managers
    llvm::LoopAnalysisManager LAM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager CGAM;
    llvm::ModuleAnalysisManager MAM;
    
    // Create pass builder with target machine
    llvm::PipelineTuningOptions PTO;
    llvm::PassBuilder PB(targetMachine, PTO);
    
    // Register analysis managers in the correct order (from LLVM opt tool)
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);
    
    // Configure optimization level
    llvm::OptimizationLevel level;
    switch(optLevel) {
        case 1: level = llvm::OptimizationLevel::O1; break;
        case 2: level = llvm::OptimizationLevel::O2; break;
        case 3: level = llvm::OptimizationLevel::O3; break;
        default: level = llvm::OptimizationLevel::O2; break;
    }
    
    // Build and run the optimization pipeline
    llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(level);
    MPM.run(*module, MAM);
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
