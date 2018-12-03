#include "Codegen.h"
#include "FuncDef.h"
#include "InputBuffer.h"
#include "Parser.h"
#include "Printer.h"
#include "Program.h"
#include "TokenStream.h"
#include <iostream>

int main( int argc, const char* const* argv )
{
    if( argc != 2 )
    {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return -1;
    }

    InputBuffer in( argv[1] );
    if( !in.IsValid() )
    {
        std::cerr << "Unable to open input file: " << argv[1] << std::endl;
        return -1;
    }

    TokenStream tokens( in.Get() );
    ProgramPtr  program( ParseProgram( tokens ) );
    if( !program )
        return -1;  // Error already reported

    std::cout << *program << std::endl;
    return 0;
}
