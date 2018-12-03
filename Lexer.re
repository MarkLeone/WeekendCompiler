#include "Lexer.h"
#include <iostream>

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
        keyword     = "if" | "int" | "return" | "while";
        punctuation = "(" | ")" | "{" | "}" | "=" | "," | ";";
        space       = [ \t\r\n]*;
        eof         = "\x00";

        integer  { return Token(atoi(begin)); }
        "int"    { return kTokenInt; }
        "if"     { return kTokenIf; }
        "else"   { return kTokenElse; }
        "return" { return kTokenReturn; }
        "while"  { return kTokenWhile; }
        id       { return Token(std::string(begin, source)); }
        "("      { return kTokenLparen; }
        ")"      { return kTokenRparen; }
        "{"      { return kTokenLbrace; }
        "}"      { return kTokenRbrace; }
        ","      { return kTokenComma; }
        "="      { return kTokenEq; }
        ";"      { return kTokenSemicolon; }
        space    { goto start; }
        eof      { return Token(kTokenEOF); }
        .        { std::cerr << "Discarding unexpected character '" 
                                << *begin << "'" << std::endl; }
    */
}    
