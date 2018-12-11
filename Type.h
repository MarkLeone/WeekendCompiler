#pragma once

#include <cassert>

enum Type
{
    kTypeUnknown,
    kTypeBool,
    kTypeInt
};

inline const char* ToString( Type type )
{
    switch( type )
    {
        case kTypeUnknown:
            return "<unknown>";
        case kTypeBool:
            return "bool";
        case kTypeInt:
            return "int";
    }
    assert( false && "Unhandled type" );
    return "";
}


