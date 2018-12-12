#pragma once

#include "Exp.h"
#include "VarDecl.h"

/// Base class for statement syntax.
class Stmt
{
  public:
    /// The destructor is virtual, ensuring that the destructor for a derived
    /// class will be properly invoked.
    virtual ~Stmt() {}

    /// Dispatch to a visitor.  \see StmtVisitor
    virtual void Dispatch( StmtVisitor& visitor ) = 0;
};

/// Unique pointer to statement.
using StmtPtr = std::unique_ptr<Stmt>;


/// Function call statement, which simply holds a CallExp.
class CallStmt : public Stmt
{
  public:
    /// Construct from the given function call expression.
    CallStmt( CallExpPtr&& callExp )
        : m_callExp( std::move( callExp ) )
    {
    }

    /// Get the function call expression.
    const CallExp& GetCallExp() const { return *m_callExp; }

    /// Dispatch to a visitor.
    void Dispatch( StmtVisitor& visitor ) override { visitor.Visit( *this ); }

  private:
    CallExpPtr m_callExp;
};


// Assignment statement.
class AssignStmt : public Stmt
{
  public:
    /// Construct assignment statement.  The lvalue is a variable, and the
    /// rvalue is an arbitrary expression.
    AssignStmt( const std::string& varName, ExpPtr&& rvalue )
        : m_varName( varName )
        , m_rvalue( std::move( rvalue ) )
    {
    }

    /// Get the variable name (lvalue).
    const std::string& GetVarName() const { return m_varName; }

    /// Get the rvalue (the right-hand side of the assignment).
    const Exp& GetRvalue() const { return *m_rvalue; }

    /// Get the declaration of the assigned variable (null until typechecked).
    const VarDecl* GetVarDecl() const { return m_varDecl; }

    /// Link the assignment to the declaration of the assigned variable
    /// (called by the typechecker).
    void SetVarDecl( const VarDecl* varDecl ) { m_varDecl = varDecl; }

    /// Dispatch to a visitor.
    void Dispatch( StmtVisitor& visitor ) override { visitor.Visit( *this ); }

  private:
    std::string    m_varName;
    ExpPtr         m_rvalue;
    const VarDecl* m_varDecl;
};


/// A declaration statement (e.g. "int x = 0;") declares a local variable with
/// an optional initializer.
class DeclStmt : public Stmt
{
  public:
    /// Construct a declaration statement from the specified variable declaration
    /// and optional initializer expression.
    DeclStmt( VarDeclPtr&& varDecl, ExpPtr&& initExp = ExpPtr() )
        : m_varDecl( std::move( varDecl ) )
        , m_initExp( std::move( initExp ) )
    {
    }

    /// Get pointer to variable declaration, which is stored at use sites by the typechecker.
    const VarDecl* GetVarDecl() const { return m_varDecl.get(); }

    /// Check whether this declaration has an initializer expression.
    bool HasInitExp() const { return bool( m_initExp ); }

    /// Get the initializer expression.  Check HasInitExp() before calling.
    const Exp& GetInitExp() const
    {
        assert( HasInitExp() && "Expected initializer expression in variable declaration" );
        return *m_initExp;
    }

    /// Dispatch to a visitor.
    void Dispatch( StmtVisitor& visitor ) override { visitor.Visit( *this ); }

  private:
    VarDeclPtr m_varDecl;
    ExpPtr     m_initExp;
};


/// Return statement.
class ReturnStmt : public Stmt
{
  public:
    /// Construct return statement with the given return value expression.
    /// (Note that void functions are not permitted, so the return value is required.)
    ReturnStmt( ExpPtr&& exp )
        : m_exp( std::move( exp ) )
    {
    }

    /// Get the return value expression.
    const Exp& GetExp() const { return *m_exp; }

    /// Dispatch to a visitor.
    void Dispatch( StmtVisitor& visitor ) override { visitor.Visit( *this ); }

  private:
    ExpPtr m_exp;
};


/// A sequence of statements.
class SeqStmt : public Stmt
{
  public:
    /// Construct sequence of statements from a vector of unique pointers.
    SeqStmt( std::vector<StmtPtr>&& stmts )
        : m_stmts( std::move( stmts ) )
    {
    }

    /// Get the sequence of statements.
    const std::vector<StmtPtr>& Get() const { return m_stmts; }

    /// Dispatch to a visitor.
    void Dispatch( StmtVisitor& visitor ) override { visitor.Visit( *this ); }

  private:
    std::vector<StmtPtr> m_stmts;
};

/// Unique pointer to a sequence of statements.
using SeqStmtPtr = std::unique_ptr<SeqStmt>;


/// If statement syntax.
class IfStmt : public Stmt
{
  public:
    /// Construct "if" statement with conditional expression, "then" statement
    /// (which might be a sequence), and an optional "else" statement.
    IfStmt( ExpPtr condExp, StmtPtr thenStmt, StmtPtr elseStmt = StmtPtr() )
        : m_condExp( std::move( condExp ) )
        , m_thenStmt( std::move( thenStmt ) )
        , m_elseStmt( std::move( elseStmt ) )
    {
    }

    /// Get the conditional expression.
    const Exp& GetCondExp() const { return *m_condExp; }

    /// Get the "then" statement, which might be a sequence.
    const Stmt& GetThenStmt() const { return *m_thenStmt; }

    /// Check whether this "if" statement has an "else" statement.
    bool HasElseStmt() const { return bool( m_elseStmt ); }

    /// Get the "else" statement.
    const Stmt& GetElseStmt() const
    {
        assert( HasElseStmt() && "Expected else statement" );
        return *m_elseStmt;
    }

    /// Dispatch to a visitor.
    void Dispatch( StmtVisitor& visitor ) override { visitor.Visit( *this ); }

  private:
    ExpPtr  m_condExp;
    StmtPtr m_thenStmt;
    StmtPtr m_elseStmt;
};


/// While statement.
class WhileStmt : public Stmt
{
  public:
    /// Construct while statement from a conditional expression and the loop body
    /// statement (which might be a sequence).
    WhileStmt( ExpPtr condExp, StmtPtr bodyStmt )
        : m_condExp( std::move( condExp ) )
        , m_bodyStmt( std::move( bodyStmt ) )
    {
    }

    /// Get the conditional expression.
    const Exp& GetCondExp() const { return *m_condExp; }

    /// Get the loop body statement (which might be a sequence).
    const Stmt& GetBodyStmt() const { return *m_bodyStmt; }

    /// Dispatch to a visitor.
    void Dispatch( StmtVisitor& visitor ) override { visitor.Visit( *this ); }
    
  private:
    ExpPtr  m_condExp;
    StmtPtr m_bodyStmt;
};


