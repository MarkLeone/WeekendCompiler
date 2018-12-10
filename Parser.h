#ifndef PARSER_H
#define PARSER_H

#include "Fwd.h"

class Program;
class TokenStream;

int ParseProgram( TokenStream& tokens, Program* program );

#endif
