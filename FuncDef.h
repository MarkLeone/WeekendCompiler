#pragma once

#include "Stmt.h"
#include "Syntax.h"
#include "Type.h"

#include <memory>
#include <string>
#include <vector>

/// Syntax for function definition.
class FuncDef
{
  public:
    /// Construct function definition syntax.
    FuncDef( const Type& returnType, const std::string& name, std::vector<VarDeclPtr>&& params, SeqStmtPtr body )
        : m_returnType( returnType )
        , m_name( name )
        , m_params( std::move( params ) )
        , m_body( std::move( body ) )
    {
    }

    /// Get the function return type.
    const Type& GetReturnType() const { return m_returnType; }

    /// Get the function name.
    const std::string& GetName() const { return m_name; }

    /// Get the parameter declarations.
    const std::vector<VarDeclPtr>& GetParams() const { return m_params; }

    /// Check whether the function definition has a body.  (Builtin function declarations do not.)
    bool HasBody() const { return bool( m_body ); }

    /// Get the function body, which is a sequence of statements.
    const SeqStmt& GetBody() const
    {
        assert( HasBody() && "Expected function body" );
        return *m_body;
    }

  private:
    Type                    m_returnType;
    std::string             m_name;
    std::vector<VarDeclPtr> m_params;
    SeqStmtPtr              m_body;
};

/// Unique pointer to a function definition.
using FuncDefPtr = std::unique_ptr<FuncDef>;

