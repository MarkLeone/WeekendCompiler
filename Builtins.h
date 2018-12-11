#pragma once

// Get builtin function declarations (for typechecking purposes).
inline const char* GetBuiltins()
{
    return
        // Arithmetic
        "int  operator+  ( int x, int y ); "
        "int  operator-  ( int x, int y ); "
        "int  operator*  ( int x, int y ); "
        "int  operator/  ( int x, int y ); "
        "int  operator%  ( int x, int y ); "
        // Equality
        "bool operator== ( int x, int y ); "
        "bool operator!= ( int x, int y ); "
        "bool operator== ( bool x, bool y ); "
        "bool operator!= ( bool x, bool y ); "
        // Comparisons
        "bool operator<  ( int x, int y ); "
        "bool operator<= ( int x, int y ); "
        "bool operator>  ( int x, int y ); "
        "bool operator>= ( int x, int y ); "
        // Unary operations.
        "bool operator!  ( bool x ); "
        "int  operator-  ( int x ); "
        // Logical operations
        "bool operator&& ( bool x, bool y ); "
        "bool operator|| ( bool x, bool y ); "
        // Type conversions
        "bool operator bool ( int x ); "
        "int  operator int  ( bool x ); "
        ;
}
