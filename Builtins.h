#pragma once

inline const char* GetBuiltins()
{
    return "int  operator+  ( int x, int y ); "
           "int  operator-  ( int x, int y ); "
           "int  operator*  ( int x, int y ); "
           "int  operator/  ( int x, int y ); "
           "bool operator<  ( int x, int y ); "
           "bool operator<= ( int x, int y ); "
           "bool operator>  ( int x, int y ); "
           "bool operator>= ( int x, int y ); ";
}
