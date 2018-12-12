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
        case kTokenOperator:  return "operator";
        case kTokenPlus:      return "+";
        case kTokenMinus:     return "-";
        case kTokenTimes:     return "*";
        case kTokenDiv:       return "/";
        case kTokenMod:       return "%";
        case kTokenEQ:        return "==";
        case kTokenNE:        return "!=";
        case kTokenLT:        return "<";
        case kTokenLE:        return "<=";
        case kTokenGT:        return ">";
        case kTokenGE:        return ">=";
        case kTokenAnd:       return "&&";
        case kTokenOr:        return "||";
        case kTokenNot:       return "!";
        case kTokenLbrace:    return "{";
        case kTokenRbrace:    return "}";
        case kTokenLparen:    return "(";
        case kTokenRparen:    return ")";
        case kTokenComma:     return ",";
        case kTokenAssign:    return "=";
        case kTokenSemicolon: return ";";
        case kTokenColon:     return ":";
        case kTokenQuestion:  return "?";
        case kTokenEOF:       return "<EOF>";
    }
    assert(false && "Unhandled token kind");
    return "";
}
