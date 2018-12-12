#pragma once

#include "VarDecl.h"
#include <string>
#include <unordered_map>

/// The typechecker uses a Scope to resolve lexical scoping.  A Scope maps
/// variable names to variable declarations.  A Scope has a parent pointer
/// that links to the enclosing scope.  This allows local variables to shadow
/// function parameters, etc.
class Scope
{
  public:
    /// Construct a scope with an optional parent scope.
    explicit Scope( const Scope* parent = nullptr )
        : m_map()
        , m_parent( parent )
    {
    }

    /// Look up the variable with the specified name, delegating to the parent
    /// scope if not found.  Returns null if the variable is not defined.
    const VarDecl* Find( const std::string& name ) const
    {
        MapType::const_iterator it = m_map.find( name );
        if( it != m_map.end() )
            return it->second;
        else
            return m_parent ? m_parent->Find( name ) : nullptr;
    }

    /// Add the given variable declaration to this scope.  The variable
    /// declaration might be a function parameter or a local variable.
    /// Returns true for success, or false if the variable is already defined
    /// in this scope.  (Note that a variable can be shadowed in an enclosing
    /// scope, but it cannot be declared twice in the same scope.)
    bool Insert( const VarDecl* varDecl )
    {
        return m_map.insert( MapType::value_type( varDecl->GetName(), varDecl ) ).second;
    }

  private:
    using MapType = std::unordered_map<std::string, const VarDecl*>;
    MapType      m_map;
    const Scope* m_parent;
};


