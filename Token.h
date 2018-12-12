#pragma once

#include <cassert>
#include <iosfwd>
#include <sstream>
#include <string>

/// Tag of lexical token, e.g. integer, id, keyword, or punctuation.
enum TokenTag
{
    // Value-carrying tokens:
    kTokenNum,
    kTokenId,

    // Keywords:
    kTokenBool,
    kTokenTrue,
    kTokenFalse,
    kTokenInt,
    kTokenIf,
    kTokenElse,
    kTokenReturn,
    kTokenWhile,
    kTokenOperator,

    // Operators:
    kTokenPlus,
    kTokenMinus,
    kTokenTimes,
    kTokenDiv,
    kTokenMod,
    kTokenEQ,
    kTokenNE,
    kTokenLT,
    kTokenLE,
    kTokenGT,
    kTokenGE,
    kTokenAnd,
    kTokenOr,
    kTokenNot,

    // Punctuation:
    kTokenLbrace,
    kTokenRbrace,
    kTokenLparen,
    kTokenRparen,
    kTokenComma,
    kTokenAssign,
    kTokenSemicolon,
    kTokenEOF
};

/// The lexer converts sequences of characters into tokens.  A token has a tag
/// (e.g. integer vs. id) and a value (e.g integer value or identifier name).
class Token
{
  public:
    /// Construct an integer token.
    explicit Token( int value )
        : m_tag( kTokenNum )
        , m_int( value )
    {
    }

    /// Construct an identifier token.
    explicit Token( const std::string& id )
        : m_tag( kTokenId )
        , m_id( id )
    {
    }

    /// Construct a non-value-carrying token.  Implicit conversion is allowed.
    Token( TokenTag tag )
        : m_tag( tag )
    {
        assert( tag != kTokenNum && tag != kTokenId && "Value required for integer and id tokens" );
    }

    /// Get the token's tag.
    TokenTag GetTag() const { return m_tag; }

    /// Get the value of a numeric token.
    int GetNum() const
    {
        assert( GetTag() == kTokenNum && "Expected numeric token" );
        return m_int;
    }

    /// Get identifier.
    const std::string& GetId() const
    {
        assert( GetTag() == kTokenId && "Expected identifier token" );
        return m_id;
    }

    /// Get token text, e.g. operator name.
    std::string ToString() const;

    /// Equality considers token value for numeric and identifier tokens.
    bool operator==( const Token& other )
    {
        if( GetTag() != other.GetTag() )
            return false;
        if( GetTag() == kTokenNum )
            return GetNum() == other.GetNum();
        if( GetTag() == kTokenId )
            return GetId() == other.GetId();
        return true;
    }

    /// Inequality operator.
    bool operator!=( const Token& other ) { return !( *this == other ); }

    /// Check whether this token can be preceded by an "operator" keyword
    /// (e.g. operator+).  This allows operators to be defined by ordinary
    /// function definitions (see Builtins.h)
    bool IsOperator() const
    {
        switch( GetTag() )
        {
            case kTokenPlus:
            case kTokenMinus:
            case kTokenTimes:
            case kTokenDiv:
            case kTokenMod:
            case kTokenEQ:
            case kTokenNE:
            case kTokenLT:
            case kTokenLE:
            case kTokenGT:
            case kTokenGE:
            case kTokenAnd:
            case kTokenOr:
            case kTokenNot:
            case kTokenBool:
            case kTokenInt:
                return true;
            default:
                return false;
        }
    }

  private:
    TokenTag    m_tag;  // Tag of token, e.g. int, id, keyword.
    int         m_int;  // Integer value, if tag is kTokenNum.
    std::string m_id;   // Identifier value, if tag is kTokenId.
};



