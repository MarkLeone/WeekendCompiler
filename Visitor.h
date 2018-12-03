#ifndef VISITOR_H
#define VISITOR_H

#include "Fwd.h"

class ExpVisitor
{
public:
    virtual void* Visit(ConstantExp& exp) = 0;
    virtual void* Visit(VarExp& exp) = 0;
    virtual void* Visit(CallExp& exp) = 0;
};

class StmtVisitor
{
public:
    virtual void Visit(CallStmt& exp) = 0;
    virtual void Visit(AssignStmt& exp) = 0;
    virtual void Visit(DeclStmt& exp) = 0;
    virtual void Visit(ReturnStmt& exp) = 0;
    virtual void Visit(SeqStmt& exp) = 0;
    virtual void Visit(IfStmt& exp) = 0;
    virtual void Visit(WhileStmt& exp) = 0;
};

#endif
