#ifndef TYPE_H
#define TYPE_H

#include <cassert>

enum Type
{
   kTypeUnknown,
   kTypeInt
};

inline const char* ToString(Type type)
{
    switch (type)
    {
      case kTypeUnknown: return "<unknown>";
      case kTypeInt: return "int";
    }
    assert(false && "Unhandled type");
    return "";
}

#endif
