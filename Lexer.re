#include "Lexer.h"
#include <iostream>

Token Lexer(const char*& source)
{
 start:
    const char* begin = source;
    const char* marker;
    /*!re2c
        re2c:define:YYCTYPE  = char;
        re2c:define:YYCURSOR = source;
        re2c:define:YYMARKER = marker;
        re2c:yyfill:enable   = 0;

        integer     = "-"?[0-9]+;
        id          = [a-zA-Z_][a-zA-Z_0-9]*;
        space       = [ \t\r\n]*;
        eof         = "\x00";
        op          = "+" | "-" | "*" | "/" | "%" | "==" | "!=" | "<" | "<=" | ">" | ">=" | "&&" | "||" | "!";

        integer  { return Token( atoi( begin ) ); }
        "bool"   { return kTokenBool; }
        "true"   { return kTokenTrue; }
        "false"  { return kTokenFalse; }
        "int"    { return kTokenInt; }
        "if"     { return kTokenIf; }
        "else"   { return kTokenElse; }
        "operator" op { return Token( std::string( begin, source ) ); }
        "return" { return kTokenReturn; }
        "while"  { return kTokenWhile; }
        id       { return Token( std::string( begin, source ) ); }
        "+"      { return kTokenPlus; }
        "-"      { return kTokenMinus; }
        "*"      { return kTokenTimes; }
        "/"      { return kTokenDiv; }
        "%"      { return kTokenMod; }
        "=="     { return kTokenEQ; }
        "!="     { return kTokenNE; }
        "<"      { return kTokenLT; }
        "<="     { return kTokenLE; }
        ">"      { return kTokenGT; }
        ">="     { return kTokenGE; }
        "&&"     { return kTokenAnd; }
        "||"     { return kTokenOr; }
        "!"      { return kTokenNot; }
        "("      { return kTokenLparen; }
        ")"      { return kTokenRparen; }
        "{"      { return kTokenLbrace; }
        "}"      { return kTokenRbrace; }
        ","      { return kTokenComma; }
        "="      { return kTokenAssign; }
        ";"      { return kTokenSemicolon; }
        space    { goto start; }
        eof      { return Token( kTokenEOF ); }
        .        { std::cerr << "Discarding unexpected character '" 
                                << *begin << "'" << std::endl; }
    */
}    
