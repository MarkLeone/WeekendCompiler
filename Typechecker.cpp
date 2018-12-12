#include "Typechecker.h"
#include "Exp.h"
#include "FuncDef.h"
#include "Program.h"
#include "Scope.h"
#include "Stmt.h"
#include "VarDecl.h"

#include <iostream>
#include <string>
#include <map>

namespace {
    
// The function table is a multimap, mapping function names to overloaded definitions.
using FuncTable = std::multimap<std::string, const FuncDef*>;

// Exceptions are used internally by the typechecker, but they do not
// propagate beyond the top-level typechecking routine.
class TypeError : public std::runtime_error
{
  public:
    explicit TypeError( const std::string& msg )
        : std::runtime_error( msg )
    {
    }
};

// The expression typechecker is a visitor.  It holds a Scope, which maps
// variable names to their declarations, and a function table, which maps
// function names to definitions.  The typechecker decorates each expression
// with its type, and it resolves lexical scoping, linking variable references
// and function calls to the corresponding definitions.  This allows
// subsequent passes (e.g. Codegen) to operate without any knowledge of
// scoping rules.
class ExpTypechecker : public ExpVisitor
{
  public:
    // Construct typecheck from scope and function table.
    ExpTypechecker( const Scope& scope, const FuncTable& funcTable )
        : m_scope( scope )
        , m_funcTable( funcTable )
    {
    }

    // Helper routine to typecheck a subexpression.  The visitor operates on
    // non-const expressions, so we must const_cast when dispatching.
    void Check( const Exp& exp ) { const_cast<Exp&>( exp ).Dispatch( *this ); }

    // Typecheck a boolean constant.
    void* Visit( BoolExp& exp ) override
    {
        assert( exp.GetType() == kTypeBool );
        return nullptr;
    }

    // Typecheck an integer constant.
    void* Visit( IntExp& exp ) override
    {
        assert( exp.GetType() == kTypeInt );
        return nullptr;
    }

    // Typecheck a variable reference.
    void* Visit( VarExp& exp ) override
    {
        // Look up the variable name in the current scope.
        const VarDecl* decl = m_scope.Find( exp.GetName() );
        if( decl )
        {
            // Set the expression type to the type specified in the declaration.
            exp.SetType( decl->GetType() );

            // Link the variable use to its declaration.  This allows subsequent passes
            // to operate without knowledge of scoping.
            exp.SetVarDecl( decl );
        }
        else
            throw TypeError( std::string( "Undefined variable: " ) + exp.GetName() );
        return nullptr;
    }

    // Typecheck a function call.
    void* Visit( CallExp& exp ) override
    {
        // Typecheck the arguments.
        const std::vector<ExpPtr>& args = exp.GetArgs();
        for( const ExpPtr& arg : args )
            Check( *arg );

        // Look up the function definition, which might be overloaded.
        const std::string& funcName = exp.GetFuncName();
        const FuncDef* funcDef  = findFunc( funcName, args );
        if( !funcDef )
            // TODO: better error message, including candidates.
            throw TypeError( std::string( "No match for function: " ) + funcName );

        // Set expression type and link it to the function definition.
        exp.SetType( funcDef->GetReturnType() );
        exp.SetFuncDef( funcDef );
        return nullptr;
    }

  private:
    const Scope&     m_scope;
    const FuncTable& m_funcTable;

    // Find a (possibly overloaded) function definition with the specified
    // name whose parameters match the types of the given arguments.
    // TODO: generalize this and use it to check for duplicate definitions.
    const FuncDef* findFunc( std::string name, const std::vector<ExpPtr>& args ) const
    {
        auto range = m_funcTable.equal_range( name );
        for( auto it = range.first; it != range.second; ++it )
        {
            const FuncDef* funcDef = it->second;
            if (argsMatch(funcDef->GetParams(), args))
                return funcDef;
        }
        return nullptr;
    }

    // Check whether the given function parameters match the given argument types.
    static bool argsMatch( const std::vector<VarDeclPtr>& params, const std::vector<ExpPtr>& args )
    {
        if( params.size() != args.size() )
            return false;
        for( size_t i = 0; i < params.size(); ++i )
        {
            if( params[i]->GetType() != args[i]->GetType() )
                return false;
        }
        return true;
    }
};


// The statement typechecker holds a scope and a function table, along with a pointer to the current function
// (for typechecking return statements). The scope is extended as nested lexical scopes are encountered.
class StmtTypechecker : public StmtVisitor
{
  public:
    StmtTypechecker( Scope* scope, const FuncTable& funcTable, const FuncDef& enclosingFunction )
        : m_scope( scope )
        , m_funcTable( funcTable )
        , m_enclosingFunction( enclosingFunction )
    {
    }

    
    // Helper routine to typecheck a sub-statement.  The visitor operates on
    // non-const expressions, so we must const_cast when dispatching.
    void CheckStmt( const Stmt& stmt ) { const_cast<Stmt&>( stmt ).Dispatch( *this ); }

    // Helper routine to typecheck an expression.  We construct an expression
    // typechecker on the fly (which is cheap) that contains the current scope
    // and function table.
    void CheckExp( const Exp& exp ) const { ExpTypechecker( *m_scope, m_funcTable ).Check( exp ); }

    // Typecheck a function call statement.
    void Visit( CallStmt& stmt ) override { CheckExp( stmt.GetCallExp() ); }

    // Typecheck an assignment statement.
    void Visit( AssignStmt& stmt ) override
    {
        // Check the rvalue (the right hand side).
        CheckExp( stmt.GetRvalue() );

        // Look up the declaration of the variable on the left hand side of the assignment.
        const std::string& varName = stmt.GetVarName();
        const VarDecl*     varDecl = m_scope->Find( varName );
        if( !varDecl )
            throw TypeError( std::string( "Undefined variable in assignment: " ) + varName );

        // Check that the type of the rvalue matches the lvalue.
        if( varDecl->GetType() != stmt.GetRvalue().GetType() )
            throw TypeError( std::string( "Type mismatch in assignment to " ) + varName );

        // Prohibit assignment to function parameters.
        if( varDecl->GetKind() != VarDecl::kLocal )
            throw TypeError( std::string( "Expected local variable in assignment to " ) + varName );

        // Link the assignment to the variable declaration.
        stmt.SetVarDecl( varDecl );
    }

    // Typecheck a declaration statement (e.g. "int x = 1;")
    void Visit( DeclStmt& stmt ) override
    {
        // Add the variable declaration to the current scope.  Declaring the same variable twice in
        // a given scope is prohibited.
        const VarDecl*     varDecl = stmt.GetVarDecl();
        const std::string& varName = varDecl->GetName();
        if( !m_scope->Insert( varDecl ) )
            throw TypeError( std::string( "Variable already defined in this scope: " ) + varName );

        // Typecheck the initializer expression (if any) and verify that its type matches the declaration.
        if( stmt.HasInitExp() )
        {
            CheckExp( stmt.GetInitExp() );
            if( stmt.GetInitExp().GetType() != varDecl->GetType() )
                throw TypeError( std::string( "Type mismatch in initialization of " ) + varName );
        }
        
    }

    // Typecheck a return statement, ensuring that the type of the return value matches the current
    // function definition.
    void Visit( ReturnStmt& stmt ) override
    {
        CheckExp( stmt.GetExp() );
        if( stmt.GetExp().GetType() != m_enclosingFunction.GetReturnType() )
            throw TypeError( "Type mismatch in return statement" );
    }

    // Typecheck a sequence of statements in a nested lexical scope.
    void Visit( SeqStmt& seq ) override
    {
        // Create a nested scope for any local variable declarations, saving the parent scope.
        Scope* parentScope = m_scope;
        Scope  localScope( parentScope );
        m_scope = &localScope;

        // Typecheck each statement in the sequence
        for( const StmtPtr& stmt : seq.Get() )
        {
            CheckStmt( *stmt );
        }

        // Restore the parent scope.
        m_scope = parentScope;
    }

    // Typecheck an "if" statement.
    void Visit( IfStmt& stmt ) override
    {
        CheckCondExp( stmt.GetCondExp() );
        CheckStmt( stmt.GetThenStmt() );
        if( stmt.HasElseStmt() )
            CheckStmt( stmt.GetElseStmt() );
    }

    // Typechek a while loop.
    void Visit( WhileStmt& stmt ) override
    {
        CheckCondExp( stmt.GetCondExp() );
        CheckStmt( stmt.GetBodyStmt() );
    }

    // Typecheck the conditional expression from an "if" statement or while
    // loop, ensuring that it has type bool or int.
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


// Typecheck a function definition, adding it to the given function table.
void checkFunction( FuncDef* funcDef, FuncTable* funcTable )
{
    // To permit recursion, we add the definition to the function table
    // before typechecking the body.  TODO: check for duplicate definitions.
    funcTable->insert( FuncTable::value_type( funcDef->GetName(), funcDef ) );

    // Construct a scope and add the function parameters.
    Scope scope;
    for( const VarDeclPtr& param : funcDef->GetParams() )
    {
        if( !scope.Insert( param.get() ) )
            throw TypeError( "Parameter already defined: " + param->GetName() );
    }

    // Typecheck the function body.
    if( funcDef->HasBody() )
        StmtTypechecker( &scope, *funcTable, *funcDef ).CheckStmt( funcDef->GetBody() );
}

} // anonymous namespace


// Typecheck a program, returning zero for success.  If a TypeError exception
// is caught, an error message is reported and a non-zero value is returned.
int Typecheck( Program& program )
{
    FuncTable funcTable;

    for( const FuncDefPtr& funcDef : program.GetFunctions() )
    {
        try
        {
            checkFunction( funcDef.get(), &funcTable );
        }
        catch( const TypeError& e )
        {
            std::cerr << "Error: " << e.what() << std::endl;
            return -1;
        }
    }
    return 0;
}
