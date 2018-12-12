#pragma once

#include "Syntax.h"

/// Expression visitor base class.  The typechecker and code generator are
/// implemented as visitors, which allows all of the logic for a pass to be
/// defined in a single class, rather than being scattered throughout separate
/// methods in the the various syntax classes.
/// 
/// A visitor can also retain state in member variables.  For example, the
/// typechecker visitor contains a symbol table (\see Scope) that is extended
/// as variable declarations are processed.
///
/// TODO: generalize visitors with templated argument and result types.
/// TODO: can we use the "curiously recurring template pattern" (CRTP) to
///       reduce virtual method overhead?
class ExpVisitor
{
  public:
    virtual void* Visit( BoolExp& exp ) = 0;
    virtual void* Visit( IntExp& exp )  = 0;
    virtual void* Visit( VarExp& exp )  = 0;
    virtual void* Visit( CallExp& exp ) = 0;
    virtual void* Visit( CondExp& exp ) = 0;
};


/// Statement visitor base class.  None of the derived classes need to return
/// values from the visit methods, so unlike the expression visitor they are
/// declared with a void return type.
class StmtVisitor
{
  public:
    virtual void Visit( CallStmt& exp )   = 0;
    virtual void Visit( AssignStmt& exp ) = 0;
    virtual void Visit( DeclStmt& exp )   = 0;
    virtual void Visit( ReturnStmt& exp ) = 0;
    virtual void Visit( SeqStmt& exp )    = 0;
    virtual void Visit( IfStmt& exp )     = 0;
    virtual void Visit( WhileStmt& exp )  = 0;
};


