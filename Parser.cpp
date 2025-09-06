#include "Parser.h"
#include "Exp.h"
#include "FuncDef.h"
#include "Program.h"
#include "Stmt.h"
#include "TokenStream.h"

namespace {

// Exceptions are used internally by the parser to simplify error checking.
// Any parse error is caught by the top-level parsing routine, which reports
// an error and returns an error status.
class ParseError : public std::runtime_error
{
  public:
    explicit ParseError( const std::string& msg )
        : std::runtime_error( msg )
    {
    }
};

// Forward declarations
ExpPtr parseExp( TokenStream& tokens );
std::vector<ExpPtr> parseArgs( TokenStream& tokens );
SeqStmtPtr parseSeq( TokenStream& tokens );
ExpPtr parseRemainingExp( ExpPtr leftExp, int leftPrecedence, TokenStream& tokens );
int getPrecedence( const Token& token );

    
// Skip the specified token, throwing ParseError if it's not present.
void skipToken( const Token& expected, TokenStream& tokens )
{
    Token token( *tokens++ );
    if( token != expected )
        throw ParseError( std::string( "Expected '" ) + expected.ToString() + "'" );
}

    
// PrimaryExp -> true | false
//             | Num
//             | Id
//             | Id ( Args )
//             | ( Exp )
//             | UnaryOp PrimaryExp
ExpPtr parsePrimaryExp( TokenStream& tokens )
{
    // Fetch the next token, advancing the token stream.  (Note that this
    // dereferences, then increments the TokenStream.)
    Token token( *tokens++ );
    switch( token.GetTag() )
    {
        // Boolean constant?
        case kTokenTrue:
            return std::make_unique<BoolExp>( true );
        case kTokenFalse:
            return std::make_unique<BoolExp>( false );
        // Integer constant?
        case kTokenNum:
            return std::make_unique<IntExp>( token.GetNum() );
        // An identifier might be a variable or the start of a function call.
        case kTokenId:
        {
            // If the next token is a left paren, it's a function call.
            if( *tokens == kTokenLparen )
                // Parse argument expressions and construct CallExp.
                return std::make_unique<CallExp>( token.GetId(), parseArgs( tokens ) );
            else
                // Construct VarExp
                return std::make_unique<VarExp>( token.GetId() );
        }
        // Type conversion?
        case kTokenBool:
        case kTokenInt:
        {
            return std::make_unique<CallExp>( token.ToString(), parseArgs( tokens ) );
        }
        // Parenthesized expression?
        case kTokenLparen:
        {
            skipToken( kTokenLparen, tokens );
            ExpPtr exp( parseExp( tokens ) );
            skipToken( kTokenRparen, tokens );
            return exp;
        }
        // Prefix minus?
        case kTokenMinus:
        {
            Token unaryOp( *tokens++ );
            ExpPtr exp( parsePrimaryExp( tokens ) );
            return std::make_unique<CallExp>( unaryOp.ToString(), std::move( exp ) );
        }
        default:
            throw ParseError( std::string( "Unexpected token: " ) + token.ToString() );
    }
}


// Args    -> ( ArgList )
// ArgList -> Exp
//          | Exp , ArgList
std::vector<ExpPtr> parseArgs( TokenStream& tokens )
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

    return exps;
}


// Parse an expression with infix operators.
ExpPtr parseExp( TokenStream& tokens )
{
    // First, parse a primary expression, which contains no infix operators.
    ExpPtr leftExp( parsePrimaryExp( tokens ) );

    // The next token might be an operator.  Call a helper routine
    // to parse the remainder of the expression.
    return parseRemainingExp( std::move( leftExp ), 0 /*initial precedence*/, tokens );
}

// This routine implements an operator precedence expression parser.
// It assembles primary expressions into call expressions based
// on the precedence of the operators it encounters.  For example,
// "1 + 2 * 3" is parsed as "1 + (2 * 3)" because multiplication has
// higher precedence than addition.
//
// After parsing an expression (leftExp) whose operator has the given
// precedence (or zero if it has no operator), parse the remainder of
// the expression from the given token stream.
ExpPtr parseRemainingExp( ExpPtr leftExp, int leftPrecedence, TokenStream& tokens )
{
    while( true )
    {
        // If the previous operator has higher precedence than the current one,
        // it claims the prevously parsed expression.
        int precedence = getPrecedence( *tokens );
        if( leftPrecedence > precedence )
            return leftExp;

        // Parse the current operator and the current primary expression.
        Token opToken( *tokens++ );
        ExpPtr rightExp = parsePrimaryExp( tokens );

        // If the next operator has higher precedence, it claims the current expression.
        int rightPrecedence = getPrecedence( *tokens );
        if( rightPrecedence > precedence )
        {
            rightExp = parseRemainingExp( std::move( rightExp ), precedence + 1, tokens );
        }

        // Construct a call expression with the left and right expressions.
        leftExp = std::make_unique<CallExp>( opToken.ToString(),
                                             std::move( leftExp ), std::move( rightExp ) );
    }
}

// If the given token is an operator, return its precedence (from 0 to 5).
// Otherwise return -1.
int getPrecedence( const Token& token )
{
    switch( token.GetTag() )
    {
        case kTokenTimes:
        case kTokenDiv:
            return 5;
        case kTokenMod:
        case kTokenPlus:
        case kTokenMinus:
            return 4;
        case kTokenLT:
        case kTokenLE:
        case kTokenGT:
        case kTokenGE:
            return 3;
        case kTokenEQ:
        case kTokenNE:
            return 2;
        case kTokenAnd:
            return 1;
        case kTokenOr:
            return 0;
        default:
            return -1;
    }
}


// Type -> bool | int
Type parseType( TokenStream& tokens )
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

// Parse an identifier.
std::string parseId( TokenStream& tokens )
{
    Token id( *tokens++ );
    if( id.GetTag() != kTokenId )
        throw ParseError( "Invalid declaration (expected identifier)" );
    return id.GetId();
}


// VarDecl -> Type Id
VarDeclPtr parseVarDecl( VarDecl::Kind kind, TokenStream& tokens )
{
    Type        type( parseType( tokens ) );
    std::string id( parseId( tokens ) );
    return std::make_unique<VarDecl>( kind, type, id );
}

    
// Stmt -> Id = Exp ;
//       | Id ( Args ) ;
//       | VarDecl ;
//       | Seq
//       | return Exp ;
//       | if ( Exp ) Stmt
//       | if ( Exp ) Stmt else Stmt
//       | while ( Exp ) Stmt
StmtPtr parseStmt( TokenStream& tokens )
{
    Token token( *tokens );
    switch( token.GetTag() )
    {
        case kTokenId:
        {
            Token id( *tokens++ );
            if( *tokens == kTokenAssign )
            {
                // Assignment
                ExpPtr rvalue( parseExp( ++tokens ) );
                skipToken( kTokenSemicolon, tokens );
                return std::make_unique<AssignStmt>( id.GetId(), std::move( rvalue ) );
            }
            else
            {
                // Call
                std::vector<ExpPtr> args( parseArgs( tokens ) );
                CallExpPtr          callExp( std::make_unique<CallExp>( id.GetId(), std::move( args ) ) );
                skipToken( kTokenSemicolon, tokens );
                return std::make_unique<CallStmt>( std::move( callExp ) );
            }
        }
        case kTokenInt:
        case kTokenBool:
        {
            // Declaration
            VarDeclPtr varDecl( parseVarDecl( VarDecl::kLocal, tokens ) );
            ExpPtr     initExp;
            if( *tokens == kTokenAssign )
            {
                initExp = parseExp( ++tokens );
            }
            skipToken( kTokenSemicolon, tokens );
            return std::make_unique<DeclStmt>( std::move( varDecl ), std::move( initExp ) );
        }
        case kTokenLbrace:
        {
            // Sequence
            return parseSeq( tokens );
        }
        case kTokenReturn:
        {
            ++tokens;  // skip "return"
            ExpPtr returnExp( parseExp( tokens ) );
            skipToken( kTokenSemicolon, tokens );
            return std::make_unique<ReturnStmt>( std::move( returnExp ) );
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
            return std::make_unique<IfStmt>( std::move( condExp ), std::move( thenStmt ), std::move( elseStmt ) );
        }
        case kTokenWhile:
        {
            ++tokens;  // skip "while"
            skipToken( kTokenLparen, tokens );
            ExpPtr condExp( parseExp( tokens ) );
            skipToken( kTokenRparen, tokens );

            StmtPtr bodyStmt( parseStmt( tokens ) );
            return std::make_unique<WhileStmt>( std::move( condExp ), std::move( bodyStmt ) );
        }
        default:
            throw ParseError( std::string( "Unexpected token: " ) + token.ToString() );
    }
}

// Seq -> { Stmt* }
SeqStmtPtr parseSeq( TokenStream& tokens )
{
    skipToken( kTokenLbrace, tokens );
    std::vector<StmtPtr> stmts;
    while( *tokens != kTokenRbrace )
    {
        stmts.push_back( parseStmt( tokens ) );
    }
    skipToken( kTokenRbrace, tokens );
    return std::make_unique<SeqStmt>( std::move( stmts ) );
}

// FuncId -> Id | operator BinaryOp
std::string parseFuncId( TokenStream& tokens )
{
    if( *tokens == kTokenOperator )
    {
        ++tokens;
        Token op( *tokens++ );
        if( !op.IsOperator() )
            throw ParseError( "Invalid operator" );
        return op.ToString();
    }
    else
        return parseId( tokens );
}

// FuncDef -> Type Id ( VarDecl* ) Seq
FuncDefPtr parseFuncDef( TokenStream& tokens )
{
    // Parse return type and function id.
    Type        returnType( parseType( tokens ) );
    std::string id( parseFuncId( tokens ) );

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

    // Parse function body (if any);
    SeqStmtPtr body;
    if( *tokens == kTokenLbrace )
        body = parseSeq( tokens );
    else
        skipToken( kTokenSemicolon, tokens );

    return std::make_unique<FuncDef>( returnType, id, std::move( params ), std::move( body ) );
}

} // anonymouse namespace

// Parse the given tokens, adding function definitions to the given program.
// Returns zero for success (otherwise an error message is reported).
int ParseProgram( TokenStream& tokens, Program* program )
{
    try
    {
        do {
            FuncDefPtr function( parseFuncDef( tokens ) );
            program->GetFunctions().push_back( std::move( function ) );
        } while( *tokens != kTokenEOF );
        return 0;
    }
    catch( const ParseError& error )
    {
        std::cerr << "Error: " << error.what() << std::endl;
        return -1;
    }
}
