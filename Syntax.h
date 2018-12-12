#pragma once

#include <memory>

// Forward declarations for syntax classes

class Exp;
class BoolExp;
class IntExp;
class VarExp;
class CallExp;
class CondExp;

class Stmt;
class AssignStmt;
class CallStmt;
class DeclStmt;
class IfStmt;
class ReturnStmt;
class SeqStmt;
class WhileStmt;

class FuncDef;
class Program;
class VarDecl;

using ExpPtr     = std::unique_ptr<Exp>;
using CallExpPtr = std::unique_ptr<CallExp>;
using FuncDefPtr = std::unique_ptr<FuncDef>;
using ProgramPtr = std::unique_ptr<Program>;
using SeqStmtPtr = std::unique_ptr<SeqStmt>;
using StmtPtr    = std::unique_ptr<Stmt>;
using VarDeclPtr = std::unique_ptr<VarDecl>;


