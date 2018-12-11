#pragma once

#include "Token.h"

/// Scan the given string for the next token (discarding whitespace).
/// The string pointer is passed by reference; it is advanced to the character
/// following the token.  Discards invalid characters (with a warning).
/// Returns kTokenEOF if the string contains no token.
Token Lexer( const char*& source );


