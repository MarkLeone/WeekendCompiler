#include "Printer.h"
#include "Exp.h"
#include "FuncDef.h"
#include "Program.h"
#include "Stmt.h"
#include "Visitor.h"

class ExpPrinter : public ExpVisitor
{
  public:
    ExpPrinter( std::ostream& out )
        : m_out( out )
    {
    }

    void Print( const Exp& exp ) { const_cast<Exp&>( exp ).Dispatch( *this ); }

    void* Visit( BoolExp& exp ) override
    {
        m_out << (exp.GetValue() ? "true" : "false");
        return nullptr;
    }

    void* Visit( IntExp& exp ) override
    {
        m_out << exp.GetValue();
        return nullptr;
    }
    
    void* Visit( VarExp& exp ) override
    {
        m_out << exp.GetName();
        return nullptr;
    }

    void* Visit( CallExp& exp ) override
    {
        m_out << exp.GetFuncName() << '(';
        for( size_t i = 0; i < exp.GetArgs().size(); ++i )
        {
            if( i > 0 )
                m_out << ", ";
            exp.GetArgs()[i]->Dispatch( *this );
        }
        m_out << ')';
        return nullptr;
    }

    void* Visit( CondExp& exp ) override
    {
        m_out << "(" << exp.GetCondExp() << ")"
              << "? (" << exp.GetThenExp()
              << ") : (" << exp.GetElseExp() << ")";
        return nullptr;
    }
    
  private:
    std::ostream& m_out;
};


class StmtPrinter : public StmtVisitor
{
  public:
    StmtPrinter( std::ostream& out )
        : m_out( out )
    {
    }

    void Print( const Stmt& stmt ) { const_cast<Stmt&>( stmt ).Dispatch( *this ); }

    void Visit( CallStmt& stmt ) override { m_out << stmt.GetCallExp() << ';'; }

    void Visit( AssignStmt& stmt ) override { m_out << stmt.GetVarName() << " = " << stmt.GetRvalue() << ';'; }

    void Visit( DeclStmt& stmt ) override
    {
        m_out << stmt.GetVarDecl();
        if( stmt.HasInitExp() )
            m_out << " = " << stmt.GetInitExp();
        m_out << ';';
    }

    void Visit( ReturnStmt& stmt ) override { m_out << "return " << stmt.GetExp() << ';'; }

    void Visit( SeqStmt& seq ) override
    {
        m_out << "{" << std::endl;
        for( const StmtPtr& stmt : seq.Get() )
        {
            Print( *stmt );
            m_out << std::endl;
        }
        m_out << "}";
    }

    void Visit( IfStmt& stmt ) override
    {
        m_out << "if (" << stmt.GetCondExp() << ")" << std::endl;
        Print( stmt.GetThenStmt() );
        if( stmt.HasElseStmt() )
        {
            m_out << std::endl << "else" << std::endl;
            Print( stmt.GetElseStmt() );
        }
    }

    void Visit( WhileStmt& stmt ) override
    {
        m_out << "while (" << stmt.GetCondExp() << ")" << std::endl;
        Print( stmt.GetBodyStmt() );
    }

  private:
    std::ostream& m_out;
};

std::ostream& operator<<( std::ostream& out, const Exp& exp )
{
    ExpPrinter( out ).Print( exp );
    return out;
}

std::ostream& operator<<( std::ostream& out, const Stmt& stmt )
{
    StmtPrinter( out ).Print( stmt );
    return out;
}

std::ostream& operator<<( std::ostream& out, const FuncDef& def )
{
    out << ToString( def.GetReturnType() ) << ' ' << def.GetName() << '(';
    for( size_t i = 0; i < def.GetParams().size(); ++i )
    {
        if( i > 0 )
            out << ", ";
        out << *def.GetParams()[i];
    }
    out << ')' << std::endl;
    if( def.HasBody() )
        out << def.GetBody();
    return out;
}

std::ostream& operator<<( std::ostream& out, const Program& program )
{
    for( const FuncDefPtr& funcDef : program.GetFunctions() )
    {
        if( funcDef->HasBody() )
            out << *funcDef << std::endl;
    }
    return out;
}
