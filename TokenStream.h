#pragma once

#include "Lexer.h"

/// The Lexer returns a single token.  This class wraps the Lexer to provide a
/// stream-like interface to the Parser.  A single token of lookahead is
/// provided (via operator*), and the token stream can be advanced using the
/// increment operator.  For example, a token is typically consumed via
/// "Token token(*tokens++);"  (Note that the dereference operator has higher
/// precedence than the increment operator.)
class TokenStream
{
  public:
    /// Construct token stream for the given source code, which must be null
    /// terminated.
    TokenStream( const char* source )
        : m_source( source )
        , m_token( kTokenEOF )
    {
        ++*this;  // Lex the first token
    }

    /// Inspect the next token, without advancing the token stream.
    Token operator*() { return m_token; }

    /// Advance the token stream, calling the Lexer to obtain the next token.
    TokenStream& operator++()
    {
        m_token = Lexer( m_source );
        return *this;
    }

    /// Postfix increment operator. 
    TokenStream operator++( int )
    {
        TokenStream before( *this );
        this->operator++();
        return before;
    }

  private:
    const char* m_source;
    Token       m_token;
};


