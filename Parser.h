#pragma once

class Program;
class TokenStream;

/// Parse the given tokens, adding function definitions to the given program.
/// Returns zero for success (otherwise an error message is reported).
int ParseProgram( TokenStream& tokens, Program* program );


