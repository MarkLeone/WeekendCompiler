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
    Exp() : m_type(kTypeUnknown) { }	
    
    virtual ~Exp() { }

    Type GetType() const { return m_type; }
    void SetType(Type type) { m_type = type; }

    virtual void* Dispatch(ExpVisitor& visitor) = 0;

private:
    Type m_type;  // Assigned by typechecker
};

using ExpPtr = std::unique_ptr<Exp>;

class ConstantExp : public Exp
{
public:
    ConstantExp(int value) :
	m_value(value)
    {
    }

    int GetValue() const { return m_value; }

    void* Dispatch(ExpVisitor& visitor) override { return visitor.Visit(*this); }

private:
    int m_value;
};

class VarExp : public Exp
{
public:
    VarExp(const std::string& name) :
	m_name(name)
    {
    }

    const std::string& GetName() const { return m_name; }

    void* Dispatch(ExpVisitor& visitor) override { return visitor.Visit(*this); }

    const VarDecl* GetVarDecl() const { return m_varDecl; }

    void SetVarDecl(const VarDecl* varDecl) { m_varDecl = varDecl; }

private:
    std::string m_name;
    const VarDecl* m_varDecl; // assigned by the typechecker.
};

class CallExp : public Exp
{
public:
    CallExp(const std::string& funcName,
	    std::vector<ExpPtr>&& args) :
	m_funcName(funcName),
	m_args(std::move(args)),
	m_funcDef(nullptr)
    {
    }

    const std::string& GetFuncName() const { return m_funcName; }

    const std::vector<ExpPtr>& GetArgs() const { return m_args; }

    const FuncDef* GetFuncDef() const { return m_funcDef; }

    void SetFuncDef(const FuncDef* funcDef) { m_funcDef = funcDef; }

    void* Dispatch(ExpVisitor& visitor) override { return visitor.Visit(*this); }

private:
    std::string m_funcName;
    std::vector<ExpPtr> m_args;
    const FuncDef* m_funcDef; // set by typechecker.
};

using CallExpPtr = std::unique_ptr<CallExp>;

#endif
