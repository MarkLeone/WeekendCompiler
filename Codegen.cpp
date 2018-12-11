#include "Codegen.h"
#include "Exp.h"
#include "FuncDef.h"
#include "Program.h"
#include "Stmt.h"
#include "Visitor.h"

#include <llvm/IR/Argument.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

using SymbolTable = std::map<const VarDecl*, Value*>;
using FunctionTable = std::map<const FuncDef*, Function*>;

class CodegenBase
{
  public:
    CodegenBase( LLVMContext* context, Module* module, IRBuilder<>* builder )
        : m_context( context )
        , m_module( module )
        , m_builder( builder )
        , m_boolType( IntegerType::get( *m_context, 1 ) )
        , m_intType( IntegerType::get( *m_context, 32 ) )
    {
    }

    LLVMContext* GetContext() { return m_context; }

    Module* GetModule() { return m_module; }

    IRBuilder<>* GetBuilder() { return m_builder; }


    llvm::Type* ConvertType(::Type type)
    {
        switch (type)
        {
            case kTypeBool:
                return m_boolType;
            case kTypeInt:
                return m_intType;
            case kTypeUnknown:
                assert( false && "Invalid type" );
                return m_intType;
        }
        assert( false && "Invalid type" );
        return m_intType;
    }

    llvm::Type* GetBoolType() const { return m_boolType; }

    llvm::Type* GetIntType() const { return m_intType; }

    Constant* GetBool( bool b ) const { return ConstantInt::get( GetBoolType(), int(b), false /*isSigned*/ ); }

    Constant* GetInt( int i ) const { return ConstantInt::get( GetIntType(), i, true /*isSigned*/ ); }

  protected:
    LLVMContext* m_context;
    Module*      m_module;
    IRBuilder<>* m_builder;
    llvm::Type*  m_boolType;
    llvm::Type*  m_intType;
};

class CodegenExp : public ExpVisitor, CodegenBase
{
  public:
    CodegenExp( LLVMContext* context, Module* module, IRBuilder<>* builder,
                SymbolTable* symbols, FunctionTable* functions )
        : CodegenBase( context, module, builder )
        , m_symbols( symbols )
        , m_functions( functions )
    {
    }

    Value* Codegen( const Exp& exp ) { return reinterpret_cast<Value*>( const_cast<Exp&>( exp ).Dispatch( *this ) ); }

    void* Visit( BoolExp& exp ) override { return GetBool( exp.GetValue() ); }

    void* Visit( IntExp& exp ) override { return GetInt( exp.GetValue() ); }

    void* Visit( VarExp& exp ) override
    {
        // The typechecker links variable references to their declarations.
        const VarDecl* varDecl = exp.GetVarDecl();
        assert( varDecl );

        // An llvm::Value was associated with the variable when its declaration was processed.
        SymbolTable::const_iterator it = m_symbols->find( varDecl );
        assert( it != m_symbols->end() );
        Value* value = it->second;

        // The value is either a function parameter or a pointer to storage for a local variable.
        switch( varDecl->GetKind() )
        {
            case VarDecl::kParam:
                return value;
            case VarDecl::kLocal:
                return GetBuilder()->CreateLoad( value, varDecl->GetName() );
        }
        assert(false && "unreachable");
        return nullptr;
    }

    void* Visit( CallExp& exp ) override
    {
        // TODO: short-circuiting for special forms (&&, ||)
        
        // Convert the arguments to LLVM values.
        std::vector<Value*> args;
        args.reserve( exp.GetArgs().size() );
        for( const ExpPtr& arg : exp.GetArgs() )
        {
            args.push_back( Codegen( *arg ) );
        }

        // Builtin definition?
        const std::string& funcName = exp.GetFuncName();
        if( funcName == "+" )
            return GetBuilder()->CreateAdd( args.at( 0 ), args.at( 1 ) );
        else if( funcName == "-" )
            return GetBuilder()->CreateSub( args.at( 0 ), args.at( 1 ) );
        else if( funcName == "*" )
            return GetBuilder()->CreateMul( args.at( 0 ), args.at( 1 ) );
        else if( funcName == "/" )
            return GetBuilder()->CreateSDiv( args.at( 0 ), args.at( 1 ) );
        else if( funcName == "%" )
            return GetBuilder()->CreateSRem( args.at( 0 ), args.at( 1 ) );
        else if( funcName == "==" )
            return GetBuilder()->CreateICmpEQ( args.at( 0 ), args.at( 1 ) );
        else if( funcName == "!=" )
            return GetBuilder()->CreateICmpNE( args.at( 0 ), args.at( 1 ) );
        else if( funcName == "<" )
            return GetBuilder()->CreateICmpSLT( args.at( 0 ), args.at( 1 ) );
        else if( funcName == "<=" )
            return GetBuilder()->CreateICmpSLE( args.at( 0 ), args.at( 1 ) );
        else if( funcName == ">" )
            return GetBuilder()->CreateICmpSGT( args.at( 0 ), args.at( 1 ) );
        else if( funcName == ">=" )
            return GetBuilder()->CreateICmpSGE( args.at( 0 ), args.at( 1 ) );

        // The typechecker linked function call sites to their definitions.
        const FuncDef* funcDef = exp.GetFuncDef();
        assert( funcDef );

        // An llvm::Function was associated with the function when its definition was processed.
        FunctionTable::const_iterator it = m_functions->find( funcDef );
        assert( it != m_functions->end() );
        Function* function = it->second;

        // Generate LLVM function call.
        return GetBuilder()->CreateCall( function, args, funcDef->GetName() );
    }

  private:
    SymbolTable* m_symbols;
    FunctionTable* m_functions;
};


class CodegenStmt : public StmtVisitor, CodegenBase
{
  public:
    CodegenStmt( LLVMContext* context, Module* module, IRBuilder<>* builder,
                 SymbolTable* symbols, FunctionTable* functions, Function* currentFunction )
        : CodegenBase( context, module, builder )
        , m_symbols( symbols )
        , m_functions( functions )
        , m_currentFunction( currentFunction )
        , m_codegenExp( context, module, builder, symbols, functions )
    {
    }

    void Codegen( const Stmt& stmt )
    {
        const_cast<Stmt&>( stmt ).Dispatch( *this );
    }

    void Visit( CallStmt& stmt ) override
    {
        m_codegenExp.Codegen( stmt.GetCallExp() );
    }

    void Visit( AssignStmt& stmt ) override
    {
        // The typechecker links assignments to variable declarations.  Assignments to function
        // parameters are prohibited by the typechecker.
        const VarDecl* varDecl = stmt.GetVarDecl();
        assert( varDecl && varDecl->GetKind() == VarDecl::kLocal );

        // The symbol table maps local variables to stack-allocated storage.
        SymbolTable::const_iterator it = m_symbols->find( varDecl );
        assert( it != m_symbols->end() );
        Value* location = it->second;

        // Generate code for the rvalue and store it.
        Value* rvalue = m_codegenExp.Codegen( stmt.GetRvalue() );
        GetBuilder()->CreateStore(rvalue, location);
    }

    void Visit( DeclStmt& stmt ) override
    {
        const VarDecl* varDecl = stmt.GetVarDecl();
        llvm::Type* type = ConvertType(varDecl->GetType());
        
        // Generate an "alloca" instruction, which goes in entry block of the current function.
        IRBuilder<> allocaBuilder( &m_currentFunction->getEntryBlock() );
        Value* location = allocaBuilder.CreateAlloca( type, nullptr /*arraySize*/, varDecl->GetName() );

        // Store the variable location in the symbol table.
        m_symbols->insert( SymbolTable::value_type( varDecl, location ) );

        // Generate code for the initializer (if any) and store it.
        if (stmt.HasInitExp())
        {
            Value* rvalue = m_codegenExp.Codegen( stmt.GetInitExp() );
            GetBuilder()->CreateStore(rvalue, location);
        }
    }

    void Visit( ReturnStmt& stmt ) override
    {
        Value* result = m_codegenExp.Codegen( stmt.GetExp() );
        GetBuilder()->CreateRet( result );
    }

    void Visit( SeqStmt& seq ) override
    {
        for( const StmtPtr& stmt : seq.Get() )
        {
            Codegen( *stmt );
        }
    }

    void Visit( IfStmt& stmt ) override
    {
        // Generate code for the conditional expression.
        Value* condition = codegenCondExp( stmt.GetCondExp() );

        // Create basic blocks for "then" branch, "else" branch (if any), and the join point.
        BasicBlock* thenBlock = BasicBlock::Create( *GetContext(), "then", m_currentFunction );
        BasicBlock* elseBlock = stmt.HasElseStmt() ? BasicBlock::Create( *GetContext(), "else", m_currentFunction ) : nullptr;
        BasicBlock* joinBlock = BasicBlock::Create( *GetContext(), "join", m_currentFunction );

        // Create a conditional branch.
        GetBuilder()->CreateCondBr( condition, thenBlock, elseBlock ? elseBlock : joinBlock );

        // Generate code for "then" branch
        GetBuilder()->SetInsertPoint( thenBlock );
        Codegen( stmt.GetThenStmt() );

        // Create an unconditional branch to the "join" block, unless the block already ends
        // in a return instruction.
        if( !GetBuilder()->GetInsertBlock()->getTerminator() )
            GetBuilder()->CreateBr( joinBlock );

        // If present, generate code for "else" branch.
        if( stmt.HasElseStmt() )
        {
            GetBuilder()->SetInsertPoint( elseBlock );
            Codegen( stmt.GetElseStmt() );

            // Create an unconditional branch to the "join" block, unless the block already ends
            // in a return instruction.
            if( !GetBuilder()->GetInsertBlock()->getTerminator() )
                GetBuilder()->CreateBr( joinBlock );
        }

        // Set the builder insertion point in the join block.
        GetBuilder()->SetInsertPoint( joinBlock );
    }

    void Visit( WhileStmt& stmt ) override
    {
        // Create a basic block for the start of the loop.
        BasicBlock* loopBlock = BasicBlock::Create( *GetContext(), "loop", m_currentFunction );
        GetBuilder()->CreateBr( loopBlock );
        GetBuilder()->SetInsertPoint( loopBlock );

        // Generate code for the loop condition.
        Value* condition = codegenCondExp( stmt.GetCondExp() );

        // Create basic blocks for the loop body and the join point.
        BasicBlock* bodyBlock = BasicBlock::Create( *GetContext(), "body", m_currentFunction );
        BasicBlock* joinBlock = BasicBlock::Create( *GetContext(), "join", m_currentFunction );

        // Create a conditional branch.
        GetBuilder()->CreateCondBr( condition, bodyBlock, joinBlock );

        // Generate code for the loop body, followed by an unconditional branch to the loop head.
        GetBuilder()->SetInsertPoint( bodyBlock );
        Codegen( stmt.GetBodyStmt() );
        GetBuilder()->CreateBr( loopBlock );

        // Set the builder insertion point in the join block.
        GetBuilder()->SetInsertPoint( joinBlock );
    }

  private:
    SymbolTable*   m_symbols;
    FunctionTable* m_functions;
    Function*      m_currentFunction;
    CodegenExp     m_codegenExp;

    Value* codegenCondExp( const Exp& exp )
    {
        Value* condition = m_codegenExp.Codegen( exp );
        if ( exp.GetType() == kTypeBool )
            return condition;

        // Convert the integer conditional expresison to a boolean (i1) using a comparison.
        assert( exp.GetType() == kTypeInt );
        return GetBuilder()->CreateICmpNE( condition, GetInt( 0 ) );
    }
};

class CodegenFunc : public CodegenBase
{
  public:
    CodegenFunc( LLVMContext* context, Module* module, FunctionTable* functions )
        : CodegenBase( context, module, &m_builder )
        , m_builder( *context )
        , m_functions( functions )
    {
    }

    void Codegen( const FuncDef* funcDef )
    {
        // Don't generate code for builtin function declarations.
        if( !funcDef->HasBody() )
            return;

        // Convert parameter types to LLVM types.
        const std::vector<VarDeclPtr>& params = funcDef->GetParams();
        std::vector<llvm::Type*> paramTypes;
        paramTypes.reserve( params.size() );
        for( const VarDeclPtr& param : params )
        {
            paramTypes.push_back( ConvertType( param->GetType() ) );
        }

        // Construct LLVM function type and function definition.
        llvm::Type*   returnType = ConvertType( funcDef->GetReturnType() );
        FunctionType* funcType   = FunctionType::get( returnType, paramTypes, false /*isVarArg*/ );
        Function* function = Function::Create( funcType, Function::ExternalLinkage, funcDef->GetName(), GetModule() );

        // Update the function table.
        m_functions->insert( FunctionTable::value_type( funcDef, function ) );

        // Construct a symbol table that maps the parameter declarations to the LLVM function parameters.
        SymbolTable symbols;
        size_t i = 0;
        for( Argument& arg : function->args() )
        {
            symbols.insert( SymbolTable::value_type( params[i].get(), &arg ) );
            ++i;
        }

        // Create entry block and use it as the builder's insertion point.
        BasicBlock* block = BasicBlock::Create(*GetContext(), "entry", function);
        GetBuilder()->SetInsertPoint(block);

        // Generate code for the body of the function.
        CodegenStmt codegen( GetContext(), GetModule(), GetBuilder(), &symbols, m_functions, function );
        codegen.Codegen( funcDef->GetBody() );

        // Add a return instruction if the user neglected to do so.
        if( !GetBuilder()->GetInsertBlock()->getTerminator() )
            GetBuilder()->CreateRet( GetInt( 0 ) );
    }

  private:
    IRBuilder<> m_builder;
    FunctionTable* m_functions;
};

std::unique_ptr<Module> Codegen(LLVMContext* context, const Program& program)
{
    std::unique_ptr<Module> module( new Module( "module", *context ) );
    FunctionTable functions;
    for( const FuncDefPtr& funcDef : program.GetFunctions() )
    {
        CodegenFunc( context, module.get(), &functions ).Codegen( funcDef.get() );
    }
    assert(!verifyModule(*module, &llvm::errs()));
    return std::move( module );
}
