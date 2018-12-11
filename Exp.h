#ifndef EXP_H
#define EXP_H

#include "Type.h"
#include "Visitor.h"
#include <iostream>
#include <memory>
#include <vector>

class ExpVisitorBase;

class Exp
{
  public:
    explicit Exp( Type type = kTypeUnknown )
        : m_type( type )
    {
    }

    virtual ~Exp() {}

    Type GetType() const { return m_type; }
    void SetType( Type type ) { m_type = type; }

    virtual void* Dispatch( ExpVisitor& visitor ) = 0;

  private:
    Type m_type;  // Set by typechecker, except for constants (BoolExp and IntExp).
};

using ExpPtr = std::unique_ptr<Exp>;

class BoolExp : public Exp
{
  public:
    BoolExp( bool value )
        : Exp( kTypeBool )
        , m_value( value )
    {
    }

    bool GetValue() const { return m_value; }

    void* Dispatch( ExpVisitor& visitor ) override { return visitor.Visit( *this ); }

  private:
    bool m_value;
};

class IntExp : public Exp
{
  public:
    IntExp( int value )
        : Exp( kTypeInt )
        , m_value( value )
    {
    }

    int GetValue() const { return m_value; }

    void* Dispatch( ExpVisitor& visitor ) override { return visitor.Visit( *this ); }

  private:
    int m_value;
};

class VarExp : public Exp
{
  public:
    VarExp( const std::string& name )
        : m_name( name )
    {
    }

    const std::string& GetName() const { return m_name; }

    void* Dispatch( ExpVisitor& visitor ) override { return visitor.Visit( *this ); }

    const VarDecl* GetVarDecl() const { return m_varDecl; }

    void SetVarDecl( const VarDecl* varDecl ) { m_varDecl = varDecl; }

  private:
    std::string    m_name;
    const VarDecl* m_varDecl;  // assigned by the typechecker.
};

class CallExp : public Exp
{
  public:
    CallExp( const std::string& funcName, std::vector<ExpPtr>&& args )
        : m_funcName( funcName )
        , m_args( std::move( args ) )
        , m_funcDef( nullptr )
    {
    }

    CallExp( const std::string& funcName, ExpPtr exp )
        : m_funcName( funcName )
        , m_args( 1 )
        , m_funcDef( nullptr )
    {
        m_args[0] = std::move( exp );
    }

    CallExp( const std::string& funcName, ExpPtr leftExp, ExpPtr rightExp )
        : m_funcName( funcName )
        , m_args( 2 )
        , m_funcDef( nullptr )
    {
        m_args[0] = std::move( leftExp );
        m_args[1] = std::move( rightExp ); 
    }

    const std::string& GetFuncName() const { return m_funcName; }

    const std::vector<ExpPtr>& GetArgs() const { return m_args; }

    const FuncDef* GetFuncDef() const { return m_funcDef; }

    void SetFuncDef( const FuncDef* funcDef ) { m_funcDef = funcDef; }

    void* Dispatch( ExpVisitor& visitor ) override { return visitor.Visit( *this ); }

  private:
    std::string         m_funcName;
    std::vector<ExpPtr> m_args;
    const FuncDef*      m_funcDef;  // set by typechecker.
};

using CallExpPtr = std::unique_ptr<CallExp>;

#endif
