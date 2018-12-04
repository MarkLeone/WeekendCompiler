#include "Token.h"
#include <string>

std::string Token::ToString() const
{
    switch( GetTag() )
    {
        case kTokenNum:
        {
            std::stringstream stream;
            stream << GetNum();
            return stream.str();
        }
        case kTokenId:        return GetId();
        case kTokenBool:      return "bool";
        case kTokenTrue:      return "true";
        case kTokenFalse:     return "false";
        case kTokenInt:       return "int";
        case kTokenIf:        return "if";
        case kTokenElse:      return "else";
        case kTokenReturn:    return "return";
        case kTokenWhile:     return "while";
        case kTokenLbrace:    return "{";
        case kTokenRbrace:    return "}";
        case kTokenLparen:    return "(";
        case kTokenRparen:    return ")";
        case kTokenComma:     return ",";
        case kTokenEq:        return "=";
        case kTokenSemicolon: return ";";
        case kTokenEOF:       return "<EOF>";
    }
    assert(false && "Unhandled token kind");
    return "";
}
