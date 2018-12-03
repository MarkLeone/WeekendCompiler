#ifndef DECL_H
#define DECL_H

#include "Type.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <string>

class VarDecl
{
  public:
    enum Kind
    {
        kLocal,
        kParam
    };

    VarDecl( Kind kind, Type type, const std::string& name )
        : m_kind( kind )
        , m_type( type )
        , m_name( name )
    {
    }

    Kind GetKind() const { return m_kind; }

    const Type& GetType() const { return m_type; }

    const std::string& GetName() const { return m_name; }

  private:
    Kind         m_kind;
    Type         m_type;
    std::string  m_name;
};

using VarDeclPtr = std::unique_ptr<VarDecl>;

inline std::ostream& operator<<( std::ostream& out, const VarDecl& varDecl )
{
    return out << ToString( varDecl.GetType() ) << ' ' << varDecl.GetName();
}

#endif
