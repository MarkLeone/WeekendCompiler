#ifndef FUNC_DEF_H
#define FUNC_DEF_H

#include "Fwd.h"
#include "Type.h"
#include "Stmt.h"
#include <memory>
#include <string>
#include <vector>

class FuncDef
{
public:
    FuncDef(const Type& returnType,
	    const std::string& name,
	    std::vector<VarDeclPtr>&& params,
	    SeqStmtPtr body) :
	m_returnType(returnType),
	m_name(name),
	m_params(std::move(params)),
	m_body(std::move(body))
    {
    }

    const Type& GetReturnType() const { return m_returnType; }
    
    const std::string& GetName() const { return m_name; }

    const std::vector<VarDeclPtr>& GetParams() const { return m_params; }

    bool HasBody() const { return bool(m_body); }
    
    const SeqStmt& GetBody() const
    {
	assert(HasBody() && "Expected function body");
	return *m_body;
    }

private:
    Type m_returnType;
    std::string m_name;
    std::vector<VarDeclPtr> m_params;
    SeqStmtPtr m_body;
};

using FuncDefPtr = std::unique_ptr<FuncDef>;

#endif
