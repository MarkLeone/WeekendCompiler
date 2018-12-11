#include "Typechecker.h"
#include "Exp.h"
#include "FuncTable.h"
#include "Program.h"
#include "Scope.h"
#include "Stmt.h"
#include "VarDecl.h"
#include <string>
#include <unordered_map>

class TypeError : public std::runtime_error
{
  public:
    explicit TypeError( const std::string& msg )
        : std::runtime_error( msg )
    {
    }
};

class ExpTypechecker : public ExpVisitor
{
  public:
    ExpTypechecker( const Scope& scope, const FuncTable& funcTable )
        : m_scope( scope )
        , m_funcTable( funcTable )
    {
    }

    void Check( const Exp& exp ) { const_cast<Exp&>( exp ).Dispatch( *this ); }

    void* Visit( BoolExp& exp ) override
    {
        assert( exp.GetType() == kTypeBool );
        return nullptr;
    }

    void* Visit( IntExp& exp ) override
    {
        assert( exp.GetType() == kTypeInt );
        return nullptr;
    }
    
    void* Visit( VarExp& exp ) override
    {
        // Link the variable use to its declaration.
        const VarDecl* decl = m_scope.Find( exp.GetName() );
        if( decl )
        {
            exp.SetType( decl->GetType() );
            exp.SetVarDecl( decl );
        }
        else
            throw TypeError( std::string( "Undefined variable: " ) + exp.GetName() );
        return nullptr;
    }

    void* Visit( CallExp& exp ) override
    {
        const std::vector<ExpPtr>& args = exp.GetArgs();
        for( const ExpPtr& arg : args )
            Check( *arg );

        // Special case: equality/inequality.
        const std::string& funcName = exp.GetFuncName();
        if( funcName == "==" || funcName == "!=" )
        {
            if( args.size() != 2 || args[0]->GetType() != args[1]->GetType() )
                throw TypeError( std::string( "Equality operator expects two arguments of the same type" ) );
            exp.SetType( kTypeBool );
            return nullptr;
        }

        // General case: look up function definition.
        const FuncDef*     funcDef  = m_funcTable.Find( funcName );
        if( !funcDef )
            throw TypeError( std::string( "Undefined function: " ) + funcName );

        // Check argument expression types vs. function parameter types.
        const std::vector<VarDeclPtr>& params = funcDef->GetParams();
        if( args.size() != params.size() )
            throw TypeError( std::string( "Argument count mismatch in call to " + funcName ) );
        for( size_t i = 0; i < args.size(); ++i )
        {
            if( args[i]->GetType() != params[i]->GetType() )
                throw TypeError( std::string( "Argument type mismatch in call to " + funcName ) );
        }

        // Set expression type and link it to the function definition.
        exp.SetType( funcDef->GetReturnType() );
        exp.SetFuncDef( funcDef );
        return nullptr;
    }

  private:
    const Scope&     m_scope;
    const FuncTable& m_funcTable;
};

class StmtTypechecker : public StmtVisitor
{
  public:
    StmtTypechecker( Scope* scope, const FuncTable& funcTable, const FuncDef& enclosingFunction )
        : m_scope( scope )
        , m_funcTable( funcTable )
        , m_enclosingFunction( enclosingFunction )
    {
    }

    void CheckStmt( const Stmt& stmt ) { const_cast<Stmt&>( stmt ).Dispatch( *this ); }

    void CheckExp( const Exp& exp ) const { ExpTypechecker( *m_scope, m_funcTable ).Check( exp ); }

    void Visit( CallStmt& stmt ) override { CheckExp( stmt.GetCallExp() ); }

    void Visit( AssignStmt& stmt ) override
    {
        CheckExp( stmt.GetRvalue() );
        const std::string& varName = stmt.GetVarName();
        const VarDecl*     varDecl = m_scope->Find( varName );
        if( !varDecl )
            throw TypeError( std::string( "Undefined variable in assignment: " ) + varName );
        if( varDecl->GetType() != stmt.GetRvalue().GetType() )
            throw TypeError( std::string( "Type mismatch in assignment to " ) + varName );
        if( varDecl->GetKind() != VarDecl::kLocal )
            throw TypeError( std::string( "Expected local variable in assignment to " ) + varName );

        // Link the assignment to the variable declaration.
        stmt.SetVarDecl( varDecl );
    }

    void Visit( DeclStmt& stmt ) override
    {
        const VarDecl*     varDecl = stmt.GetVarDecl();
        const std::string& varName = varDecl->GetName();
        if( stmt.HasInitExp() )
        {
            CheckExp( stmt.GetInitExp() );
            if( stmt.GetInitExp().GetType() != varDecl->GetType() )
                throw TypeError( std::string( "Type mismatch in initialization of " ) + varName );
        }
        if( !m_scope->Insert( varDecl ) )
            throw TypeError( std::string( "Variable already defined in this scope: " ) + varName );
    }

    void Visit( ReturnStmt& stmt ) override
    {
        CheckExp( stmt.GetExp() );
        if( stmt.GetExp().GetType() != m_enclosingFunction.GetReturnType() )
            throw TypeError( "Type mismatch in return statement" );
    }

    void Visit( SeqStmt& seq ) override
    {
        // Create a nested scope for any local variable declarations, saving and restoring
        // the parent scope.
        Scope* parentScope = m_scope;
        Scope  localScope( parentScope );
        m_scope = &localScope;
        for( const StmtPtr& stmt : seq.Get() )
        {
            CheckStmt( *stmt );
        }
        m_scope = parentScope;
    }

    void Visit( IfStmt& stmt ) override
    {
        CheckCondExp( stmt.GetCondExp() );
        CheckStmt( stmt.GetThenStmt() );
        if( stmt.HasElseStmt() )
            CheckStmt( stmt.GetElseStmt() );
    }

    void Visit( WhileStmt& stmt ) override
    {
        CheckCondExp( stmt.GetCondExp() );
        CheckStmt( stmt.GetBodyStmt() );
    }

    void CheckCondExp( const Exp& exp)
    {
        CheckExp( exp );
        switch (exp.GetType())
        {
            case kTypeBool:
            case kTypeInt:
                return;
            default:
                throw TypeError( "Expected integer condition expression" );
        }
    }

  private:
    Scope*           m_scope;
    const FuncTable& m_funcTable;
    const FuncDef&   m_enclosingFunction;
};

class Typechecker
{
  public:
    Typechecker()
    {
        // TODO: load builtin function definitions.
    }

    int Check( Program& program )
    {
        for( const FuncDefPtr& funcDef : program.GetFunctions() )
        {
            try
            {
                checkFunction( funcDef.get() );
            }
            catch( const TypeError& e )
            {
                fprintf(stderr, "Error: %s\n", e.what());
                return -1;
            }
        }
        return 0;
    }

  private:
    FuncTable m_funcTable;

    void checkFunction( FuncDef* funcDef )
    {
        // To permit recursion, we add the definition to the function table
        // before typechecking the body.
        if( !m_funcTable.Insert( funcDef ) )
            throw TypeError( std::string( "Function already defined: " ) + funcDef->GetName() );

        // Construct a scope and add the function parameters.
        Scope scope;
        for( const VarDeclPtr& param : funcDef->GetParams() )
        {
            if( !scope.Insert( param.get() ) )
                throw TypeError( "Parameter already defined: " + param->GetName() );
        }

        // Typecheck the function body.
        if( funcDef->HasBody() )
            StmtTypechecker( &scope, m_funcTable, *funcDef ).CheckStmt( funcDef->GetBody() );
    }
};

int Typecheck( Program& program )
{
    return Typechecker().Check( program );
}
