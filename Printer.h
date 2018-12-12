#pragma once

#include <iosfwd>

class Exp;
class Stmt;
class FuncDef;
class Program;

/// Output an expression.
std::ostream& operator<<( std::ostream& out, const Exp& exp );

/// Output a statement.
std::ostream& operator<<( std::ostream& out, const Stmt& stmt );

/// Output a function definition.
std::ostream& operator<<( std::ostream& out, const FuncDef& def );

/// Output a program.
std::ostream& operator<<( std::ostream& out, const Program& program );


