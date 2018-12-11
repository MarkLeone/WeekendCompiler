#pragma once

inline const char* GetBuiltins()
{
    // Equality and unary minus are not included because they require overloading.
    return "int  operator+  ( int x, int y ); "
           "int  operator-  ( int x, int y ); "
           "int  operator*  ( int x, int y ); "
           "int  operator/  ( int x, int y ); "
           "int  operator%  ( int x, int y ); "
           "bool operator<  ( int x, int y ); "
           "bool operator<= ( int x, int y ); "
           "bool operator>  ( int x, int y ); "
           "bool operator>= ( int x, int y ); "
           "bool operator!  ( bool x ); "
           "bool operator&& ( bool x, bool y ); "
           "bool operator|| ( bool x, bool y ); "
        ;
}
