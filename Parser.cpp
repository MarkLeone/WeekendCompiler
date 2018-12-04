#include "Parser.h"
#include "Exp.h"
#include "FuncDef.h"
#include "Program.h"
#include "Stmt.h"
#include "TokenStream.h"

class ParseError : public std::runtime_error
{
  public:
    explicit ParseError( const std::string& msg )
        : std::runtime_error( msg )
    {
    }
};

// Forward declarations
static std::vector<ExpPtr> parseArgs( TokenStream& tokens );
static SeqStmtPtr parseSeqStmt( TokenStream& tokens );

static void skipToken( const Token& expected, TokenStream& tokens )
{
    Token token( *tokens++ );
    if( token != expected )
        throw ParseError( std::string( "Expected '" ) + expected.ToString() + "'" );
}

static ExpPtr parseExp( TokenStream& tokens )
{
    Token token( *tokens++ );
    switch( token.GetTag() )
    {
        case kTokenTrue:  return ExpPtr( new BoolExp( true ) );
        case kTokenFalse: return ExpPtr( new BoolExp( false ) );
        case kTokenNum:   return ExpPtr( new IntExp( token.GetNum() ) );
        case kTokenId:
        {
            if( *tokens == kTokenLparen )
                return ExpPtr( new CallExp( token.GetId(), parseArgs( tokens ) ) );
            else
                return ExpPtr( new VarExp( token.GetId() ) );
        }
        default:
            throw ParseError( std::string( "Unexpected token: " ) + token.ToString() );
    }
}

static std::vector<ExpPtr> parseArgs( TokenStream& tokens )
{
    skipToken( kTokenLparen, tokens );
    std::vector<ExpPtr> exps;
    if( *tokens != kTokenRparen )
    {
        exps.push_back( parseExp( tokens ) );
        while( *tokens == kTokenComma )
        {
            exps.push_back( parseExp( ++tokens ) );
        }
    }
    skipToken( kTokenRparen, tokens );

    return std::move( exps );
}

static Type parseType( TokenStream& tokens )
{
    Token typeName( *tokens++ );
    switch( typeName.GetTag() )
    {
        case kTokenBool:
            return kTypeBool;
        case kTokenInt:
            return kTypeInt;
        default:
            throw ParseError( "Expected type name" );
    }
}

static std::string parseId( TokenStream& tokens )
{
    Token id( *tokens++ );
    if( id.GetTag() != kTokenId )
        throw ParseError( "Invalid declaration (expected identifier)" );
    return id.GetId();
}

static VarDeclPtr parseVarDecl( VarDecl::Kind kind, TokenStream& tokens )
{
    Type        type( parseType( tokens ) );
    std::string id( parseId( tokens ) );
    return VarDeclPtr( new VarDecl( kind, type, id ) );
}

static StmtPtr parseStmt( TokenStream& tokens )
{
    Token token( *tokens );
    switch( token.GetTag() )
    {
        case kTokenId:
        {
            Token id( *tokens++ );
            if( *tokens == kTokenEq )
            {
                // Assignment
                ExpPtr rvalue( parseExp( ++tokens ) );
                skipToken( kTokenSemicolon, tokens );
                return StmtPtr( new AssignStmt( id.GetId(), std::move( rvalue ) ) );
            }
            else
            {
                // Call
                std::vector<ExpPtr> args( parseArgs( tokens ) );
                CallExpPtr          callExp( new CallExp( id.GetId(), std::move( args ) ) );
                skipToken( kTokenSemicolon, tokens );
                return StmtPtr( new CallStmt( std::move( callExp ) ) );
            }
        }
        case kTokenInt:
        case kTokenBool:
        {
            // Declaration
            VarDeclPtr varDecl( parseVarDecl( VarDecl::kLocal, tokens ) );
            ExpPtr     initExp;
            if( *tokens == kTokenEq )
            {
                initExp = parseExp( ++tokens );
            }
            skipToken( kTokenSemicolon, tokens );
            return StmtPtr( new DeclStmt( std::move( varDecl ), std::move( initExp ) ) );
        }
        case kTokenLbrace:
        {
            // Sequence
            return parseSeqStmt( tokens );
        }
        case kTokenReturn:
        {
            ++tokens;  // skip "return"
            ExpPtr returnExp( parseExp( tokens ) );
            skipToken( kTokenSemicolon, tokens );
            return StmtPtr( new ReturnStmt( std::move( returnExp ) ) );
        }
        case kTokenIf:
        {
            ++tokens;  // skip "if"
            skipToken( kTokenLparen, tokens );
            ExpPtr condExp( parseExp( tokens ) );
            skipToken( kTokenRparen, tokens );

            StmtPtr thenStmt( parseStmt( tokens ) );
            StmtPtr elseStmt;
            if( *tokens == kTokenElse )
            {
                ++tokens;  // skip "else"
                elseStmt = parseStmt( tokens );
            }
            return StmtPtr( new IfStmt( std::move( condExp ), std::move( thenStmt ), std::move( elseStmt ) ) );
        }
        case kTokenWhile:
        {
            ++tokens;  // skip "while"
            skipToken( kTokenLparen, tokens );
            ExpPtr condExp( parseExp( tokens ) );
            skipToken( kTokenRparen, tokens );

            StmtPtr bodyStmt( parseStmt( tokens ) );
            return StmtPtr( new WhileStmt( std::move( condExp ), std::move( bodyStmt ) ) );
        }
        default:
            throw ParseError( std::string( "Unexpected token: " ) + token.ToString() );
    }
}

static SeqStmtPtr parseSeqStmt( TokenStream& tokens )
{
    skipToken( kTokenLbrace, tokens );
    std::vector<StmtPtr> stmts;
    while( *tokens != kTokenRbrace )
    {
        stmts.push_back( parseStmt( tokens ) );
    }
    skipToken( kTokenRbrace, tokens );
    return SeqStmtPtr( new SeqStmt( std::move( stmts ) ) );
}

static FuncDefPtr parseFuncDef( TokenStream& tokens )
{
    // Parse return type and function id.
    Type        returnType( parseType( tokens ) );
    std::string id( parseId( tokens ) );

    // Parse parameter declarations
    skipToken( kTokenLparen, tokens );
    std::vector<VarDeclPtr> params;
    if( *tokens != kTokenRparen )
    {
        params.push_back( parseVarDecl( VarDecl::kParam, tokens ) );
        while( *tokens != kTokenRparen )
        {
            skipToken( kTokenComma, tokens );
            params.push_back( parseVarDecl( VarDecl::kParam, tokens ) );
        }
    }
    skipToken( kTokenRparen, tokens );

    // Parse function body.
    SeqStmtPtr body( parseSeqStmt( tokens ) );

    return FuncDefPtr( new FuncDef( returnType, id, std::move( params ), std::move( body ) ) );
}

ProgramPtr ParseProgram( TokenStream& tokens )
{
    try
    {
        ProgramPtr program( new Program );
        while( *tokens != kTokenEOF )
        {
            FuncDefPtr function( parseFuncDef( tokens ) );
            program->GetFunctions().push_back( std::move( function ) );
        }
        return std::move( program );
    }
    catch( const ParseError& error )
    {
        std::cerr << "Error: " << error.what() << std::endl;
        return ProgramPtr();
    }
}
