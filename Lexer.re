#include "Lexer.h"
#include <iostream>

// This file is processed by re2c (http://re2c.org) to generate a finite state
// machine that matches various regular expressions.

// Scan the given string for the next token (discarding whitespace).
// The string pointer is passed by reference; it is advanced to the character
// following the token.  Discards invalid characters (with a warning).
// Returns kTokenEOF if the string contains no token.
Token Lexer(const char*& source)
{
 start:
    const char* begin = source;
    /*!re2c
        re2c:define:YYCTYPE  = char;
        re2c:define:YYCURSOR = source;
        re2c:yyfill:enable   = 0;

        integer     = "-"?[0-9]+;
        id          = [a-zA-Z_][a-zA-Z_0-9]*;
        space       = [ \t\r\n]+;
        eof         = "\x00";

        integer    { return Token( atoi( begin ) ); }
        "bool"     { return kTokenBool; }
        "true"     { return kTokenTrue; }
        "false"    { return kTokenFalse; }
        "int"      { return kTokenInt; }
        "if"       { return kTokenIf; }
        "else"     { return kTokenElse; }
        "operator" { return kTokenOperator; }
        "return"   { return kTokenReturn; }
        "while"    { return kTokenWhile; }
        id         { return Token( std::string( begin, source ) ); }
        "+"        { return kTokenPlus; }
        "-"        { return kTokenMinus; }
        "*"        { return kTokenTimes; }
        "/"        { return kTokenDiv; }
        "%"        { return kTokenMod; }
        "=="       { return kTokenEQ; }
        "!="       { return kTokenNE; }
        "<"        { return kTokenLT; }
        "<="       { return kTokenLE; }
        ">"        { return kTokenGT; }
        ">="       { return kTokenGE; }
        "&&"       { return kTokenAnd; }
        "||"       { return kTokenOr; }
        "!"        { return kTokenNot; }
        "("        { return kTokenLparen; }
        ")"        { return kTokenRparen; }
        "{"        { return kTokenLbrace; }
        "}"        { return kTokenRbrace; }
        ","        { return kTokenComma; }
        "="        { return kTokenAssign; }
        ";"        { return kTokenSemicolon; }
        space      { goto start; }
        eof        { return Token( kTokenEOF ); }
        .          { std::cerr << "Discarding unexpected character '" 
                                << *begin << "'" << std::endl; }
    */
}    
