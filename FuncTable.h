#ifndef FUNC_TABLE_H
#define FUNC_TABLE_H

#include "FuncDef.h"
#include <string>
#include <unordered_map>

class FuncTable
{
public:
    const FuncDef* Find(std::string name) const
    {
	MapType::const_iterator it = m_map.find(name);
	return it == m_map.end() ? nullptr : it->second;
    }

    bool Insert(const FuncDef* funcDef)
    {
	return m_map.insert(MapType::value_type(funcDef->GetName(), funcDef)).second;
    }

private:
    using MapType = std::unordered_map<std::string, const FuncDef*>;
    MapType m_map;
};

#endif
