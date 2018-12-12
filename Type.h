#pragma once

#include <cassert>

/// Only bool and int types are supported, so a Type is simply an enum value.
/// Supporting arrays and structs would require a structured representation.
enum Type
{
    kTypeUnknown,
    kTypeBool,
    kTypeInt
};

/// Convert the given type to a string.
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


