#ifndef STMT_H
#define STMT_H

#include "Exp.h"
#include "VarDecl.h"

class Stmt
{
public:
    virtual ~Stmt() { }

    virtual void Dispatch(StmtVisitor& visitor) = 0;
};

using StmtPtr = std::unique_ptr<Stmt>;    

class CallStmt : public Stmt
{
public:
    CallStmt(CallExpPtr&& callExp) :
	m_callExp(std::move(callExp))
    {
    }

    const CallExp& GetCallExp() const { return *m_callExp; }

    void Dispatch(StmtVisitor& visitor) override { visitor.Visit(*this); }

private:
    CallExpPtr m_callExp;
};

class AssignStmt : public Stmt
{
public:
    AssignStmt(const std::string& varName, ExpPtr&& rvalue) :
	m_varName(varName),
	m_rvalue(std::move(rvalue))
    {
    }

    const std::string& GetVarName() const { return m_varName; }

    const Exp& GetRvalue() const { return *m_rvalue; }

    const VarDecl* GetVarDecl() const { return m_varDecl; }

    void SetVarDecl(const VarDecl* varDecl) { m_varDecl = varDecl; }

    void Dispatch(StmtVisitor& visitor) override { visitor.Visit(*this); }

private:
    std::string m_varName;
    ExpPtr m_rvalue;
    const VarDecl* m_varDecl;
};

class DeclStmt : public Stmt
{
public:
    DeclStmt(VarDeclPtr&& varDecl, ExpPtr&& initExp = ExpPtr()) :
	m_varDecl(std::move(varDecl)),
	m_initExp(std::move(initExp))
    {
    }

    // Get pointer to variable declaration, which is stored at use sites by the typechecker.
    const VarDecl* GetVarDecl() const { return m_varDecl.get(); }

    /// Check whether this declaration has an initializer expression.
    bool HasInitExp() const { return bool(m_initExp); }

    /// Get the initializer expression.  Check HasInitExp() before calling.
    const Exp& GetInitExp() const
    {
	assert(HasInitExp() && "Expected initializer expression in variable declaration");
	return *m_initExp;
    }

    void Dispatch(StmtVisitor& visitor) override { visitor.Visit(*this); }

private:
    VarDeclPtr m_varDecl;
    ExpPtr m_initExp;
};

class ReturnStmt : public Stmt
{
public:
    ReturnStmt(ExpPtr&& exp) :
	m_exp(std::move(exp))
    {
    }

    const Exp& GetExp() const { return *m_exp; }

    void Dispatch(StmtVisitor& visitor) override { visitor.Visit(*this); }

private:
    ExpPtr m_exp;
};

class SeqStmt : public Stmt
{
public:
    SeqStmt(std::vector<StmtPtr>&& stmts) :
	m_stmts(std::move(stmts))
    {
    }

    const std::vector<StmtPtr>& Get() const { return m_stmts; }

    void Dispatch(StmtVisitor& visitor) override { visitor.Visit(*this); }

private:
    std::vector<StmtPtr> m_stmts;
};

using SeqStmtPtr = std::unique_ptr<SeqStmt>;

class IfStmt : public Stmt
{
public:
    IfStmt(ExpPtr condExp, StmtPtr thenStmt, StmtPtr elseStmt = StmtPtr()) :
	m_condExp(std::move(condExp)),
	m_thenStmt(std::move(thenStmt)),
	m_elseStmt(std::move(elseStmt))
    {
    }

    const Exp& GetCondExp() const { return *m_condExp; }

    const Stmt& GetThenStmt() const { return *m_thenStmt; }

    bool HasElseStmt() const { return bool(m_elseStmt); }

    const Stmt& GetElseStmt() const
    {
	assert(HasElseStmt() && "Expected else statement");
	return *m_elseStmt;
    }

    void Dispatch(StmtVisitor& visitor) override { visitor.Visit(*this); }

private:
    ExpPtr m_condExp;
    StmtPtr m_thenStmt;
    StmtPtr m_elseStmt;
};

class WhileStmt : public Stmt
{
public:
    WhileStmt(ExpPtr condExp, StmtPtr bodyStmt) :
	m_condExp(std::move(condExp)),
	m_bodyStmt(std::move(bodyStmt))
    {
    }

    const Exp& GetCondExp() const { return *m_condExp; }

    const Stmt& GetBodyStmt() const { return *m_bodyStmt; }

    void Dispatch(StmtVisitor& visitor) override { visitor.Visit(*this); }
private:
    ExpPtr m_condExp;
    StmtPtr m_bodyStmt;
};

#endif
