#ifndef TOKEN_STREAM_H
#define TOKEN_STREAM_H

#include "Lexer.h"

class TokenStream
{
  public:
    TokenStream( const char* source )
        : m_source( source )
        , m_token( kTokenEOF )
    {
        ++*this;  // Lex the first token
    }

    Token operator*() { return m_token; }

    TokenStream& operator++()
    {
        m_token = Lexer( m_source );
        return *this;
    }

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

#endif
