#ifndef PARSER_H
#define PARSER_H

#include "Fwd.h"

class TokenStream;

ProgramPtr ParseProgram( TokenStream& tokens );

#endif
