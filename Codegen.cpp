#include "Codegen.h"
#include "CodegenUtils.h"
#include "Exp.h"
#include "Stmt.h"
#include "Visitor.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

using namespace llvm;

class CodegenBase
{
  public:
    CodegenBase( LLVMContext* context, Module* module )
        : m_context( context )
        , m_module( module )
        , m_builder( *context )
        , m_utils( context, module )
    {
    }

    LLVMContext* GetContext() { return m_context; }

    Module* GetModule() { return m_module; }

    IRBuilder<>* GetBuilder() { return &m_builder; }

    CodegenUtils* GetUtils() { return &m_utils; }

  protected:
    LLVMContext* m_context;
    Module*      m_module;
    IRBuilder<>  m_builder;
    CodegenUtils m_utils;
};

class CodegenExp : public ExpVisitor, CodegenBase
{
  public:
    CodegenExp( LLVMContext* context, Module* module )
        : CodegenBase( context, module )
    {
    }

    Value* Codegen( const Exp& exp ) { return reinterpret_cast<Value*>( const_cast<Exp&>( exp ).Dispatch( *this ) ); }

    void* Visit( ConstantExp& exp ) override { return m_utils.MakeInt32( exp.GetValue() ); }

    void* Visit( VarExp& exp ) override
    {
        return nullptr;  // XXX
    }

    void* Visit( CallExp& exp ) override
    {
        return nullptr;  // XXX
    }
};


class CodegenStmt : public StmtVisitor, CodegenBase
{
  public:
    CodegenStmt( LLVMContext* context, Module* module )
        : CodegenBase( context, module )
    {
    }

    void Visit( CallStmt& stmt ) override {}

    void Visit( AssignStmt& stmt ) override {}

    void Visit( DeclStmt& stmt ) override {}

    void Visit( ReturnStmt& stmt ) override {}

    void Visit( SeqStmt& stmt ) override {}

    void Visit( IfStmt& stmt ) override {}

    void Visit( WhileStmt& stmt ) override {}
};

class CodegenFunc : public CodegenBase
{
  public:
    CodegenFunc( LLVMContext* context, Module* module )
        : CodegenBase( context, module )
    {
    }
};

class Codegen
{
  public:
    Codegen()
        : m_context()
        , m_module()
    {
    }

  private:
    LLVMContext             m_context;
    std::unique_ptr<Module> m_module;
};
