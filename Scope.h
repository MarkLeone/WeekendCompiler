#ifndef SCOPE_H
#define SCOPE_H

#include "VarDecl.h"
#include <string>
#include <unordered_map>

class Scope
{
public:
    explicit Scope(const Scope* parent = nullptr) :
	m_map(),
	m_parent(parent)
    {
    }

    const VarDecl* Find(const std::string& name) const
    {
	MapType::const_iterator it = m_map.find(name);
	if (it != m_map.end())
	    return it->second;
	else
	    return m_parent ? m_parent->Find(name) : nullptr;
    }

    bool Insert(const VarDecl* varDecl)
    {
	return m_map.insert(MapType::value_type(varDecl->GetName(), varDecl)).second;
    }

private:
    using MapType = std::unordered_map<std::string, const VarDecl*>;
    MapType m_map;
    const Scope* m_parent;
};

#endif
