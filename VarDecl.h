#pragma once

#include "Type.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <string>

/// Variable declaration syntax.  VarDecl is used to represent function
/// parameters (\see FuncDef) and local variable declarations (\see DeclStmt).
class VarDecl
{
  public:
    /// The declaration kind is needed by the code generator, since it
    /// determines how the variable value is stored.
    enum Kind
    {
        kLocal,
        kParam
    };

    /// Construct a variable declaration of the specified kind with the given
    /// type and name.  Note that the initializer for a local variable is not
    /// part of the declaration; it is stored in the DeclStmt.
    VarDecl( Kind kind, Type type, const std::string& name )
        : m_kind( kind )
        , m_type( type )
        , m_name( name )
    {
    }

    /// Get the kind of this variable declaration (kLocal vs. kParam).
    Kind GetKind() const { return m_kind; }

    /// Get the variable's type.
    const Type& GetType() const { return m_type; }

    /// Get the variable name.
    const std::string& GetName() const { return m_name; }

  private:
    Kind         m_kind;
    Type         m_type;
    std::string  m_name;
};

/// Unique pointer to variable declaration.
using VarDeclPtr = std::unique_ptr<VarDecl>;

/// Output a variable declaration.
inline std::ostream& operator<<( std::ostream& out, const VarDecl& varDecl )
{
    return out << ToString( varDecl.GetType() ) << ' ' << varDecl.GetName();
}


