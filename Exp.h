#pragma once

#include "Type.h"
#include "Visitor.h"
#include <iostream>
#include <memory>
#include <vector>

/// Base class for an expression, which holds its type.
class Exp
{
  public:
    /// Construct expression.  Most expression types are unknown until typechecking,
    /// except for constants.
    explicit Exp( Type type = kTypeUnknown )
        : m_type( type )
    {
    }

    /// The destructor is virtual, ensuring that the destructor for a derived
    /// class will be properly invoked.
    virtual ~Exp() {}

    /// Get the expression type (usually kTypeUnknown if not yet typechecked).
    Type GetType() const { return m_type; }

    /// Set the expression type.
    void SetType( Type type ) { m_type = type; }

    /// Dispatch to a visitor.  \see ExpVisitor
    virtual void* Dispatch( ExpVisitor& visitor ) = 0;

  private:
    Type m_type;
};

/// Unique pointer to expression.
using ExpPtr = std::unique_ptr<Exp>;

/// Boolean constant expression.
class BoolExp : public Exp
{
  public:
    /// Construct boolean constant expression.
    BoolExp( bool value )
        : Exp( kTypeBool )
        , m_value( value )
    {
    }

    /// Get the value of this constant.
    bool GetValue() const { return m_value; }

    /// Dispatch to visitor.
    void* Dispatch( ExpVisitor& visitor ) override { return visitor.Visit( *this ); }

  private:
    bool m_value;
};


/// Integer constant expression
class IntExp : public Exp
{
  public:
    /// Construct integer constant expression.
    IntExp( int value )
        : Exp( kTypeInt )
        , m_value( value )
    {
    }

    /// Get the value of this constant.
    int GetValue() const { return m_value; }

    /// Dispatch to visitor.
    void* Dispatch( ExpVisitor& visitor ) override { return visitor.Visit( *this ); }

  private:
    int m_value;
};


/// Variable expression.
class VarExp : public Exp
{
  public:
    /// Construct variable expression.
    VarExp( const std::string& name )
        : m_name( name )
    {
    }

    /// Get the variable name.
    const std::string& GetName() const { return m_name; }

    /// Get the variable's declaration (null if not yet typechecked).
    const VarDecl* GetVarDecl() const { return m_varDecl; }

    /// Link this variable expression to the variable's declaration.  Called from the typechecker.
    void SetVarDecl( const VarDecl* varDecl ) { m_varDecl = varDecl; }

    /// Dispatch to a visitor.
    void* Dispatch( ExpVisitor& visitor ) override { return visitor.Visit( *this ); }

  private:
    std::string    m_name;
    const VarDecl* m_varDecl;  // assigned by the typechecker.
};


/// Function call expression.
class CallExp : public Exp
{
  public:
    /// Construct function call expression with arbitrary arguments.
    CallExp( const std::string& funcName, std::vector<ExpPtr>&& args )
        : m_funcName( funcName )
        , m_args( std::move( args ) )
        , m_funcDef( nullptr )
    {
    }

    /// Construct a unary function call (for convenience).
    CallExp( const std::string& funcName, ExpPtr exp )
        : m_funcName( funcName )
        , m_args( 1 )
        , m_funcDef( nullptr )
    {
        m_args[0] = std::move( exp );
    }

    /// Construct a binary function call (for convenience).
    CallExp( const std::string& funcName, ExpPtr leftExp, ExpPtr rightExp )
        : m_funcName( funcName )
        , m_args( 2 )
        , m_funcDef( nullptr )
    {
        m_args[0] = std::move( leftExp );
        m_args[1] = std::move( rightExp ); 
    }

    /// Get the function name.
    const std::string& GetFuncName() const { return m_funcName; }

    /// Get the argument expressions.
    const std::vector<ExpPtr>& GetArgs() const { return m_args; }

    /// Get the function definition (null until typechecked).
    const FuncDef* GetFuncDef() const { return m_funcDef; }

    /// Link this function call to the function definition.  Called from typechecker.
    void SetFuncDef( const FuncDef* funcDef ) { m_funcDef = funcDef; }

    /// Dispatch to visitor.
    void* Dispatch( ExpVisitor& visitor ) override { return visitor.Visit( *this ); }

  private:
    std::string         m_funcName;
    std::vector<ExpPtr> m_args;
    const FuncDef*      m_funcDef;  // set by typechecker.
};

/// Unique pointer to function call expression
using CallExpPtr = std::unique_ptr<CallExp>;

