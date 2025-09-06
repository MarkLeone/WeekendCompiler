
# One Weekend Compiler

The LLVM compiler toolkit makes it easy to implement a compiler, and the [LLVM
Tutorial](https://llvm.org/docs/tutorial/) (called "Kaleidoscope") is a good
place to start.

This example compiler provides an alternative introduction, with a bit more
focus on design details like the use of a visitor pattern for syntax traversal.
It consists of the following components:

- A regexp-based lexer, employing [re2c](http://re2c.org/) to generate an efficient state machine.

- A recursive-descent parser with an operator-precedence strategy for parsing
  expressions with infix operators.
  
- Well-engineered abstract syntax classes that provide a good foundation for extending the source language.

- A typechecker that supports simple overloading (without implicit type
  conversions).  A key aspect of the typechecker is that it resolves lexical
  scoping, linking variable references and function calls to the corresponding
  definitions.  This allows subsequent passes to operate without any knowledge
  of scoping rules.

- A simple code generator that contructs LLVM IR.

- Optimization and JIT code generation using off-the-shelf LLVM components.

# Grammar

The source language is a subset of C with the following grammar.

```  
  Prog -> FuncDef+
  
  FuncDef -> Type FuncId ( VarDecl* ) Seq
  
  Type -> bool | int
  
  FuncId -> Id | operator BinaryOp
  
  VarDecl -> Type Id
  
  Seq -> { Stmt* }
  
  Stmt -> Id = Exp ;
        | Id ( Args ) ;
        | VarDecl ;
        | Seq
        | return Exp ;
        | if ( Exp ) Stmt
        | if ( Exp ) Stmt else Stmt
        | while ( Exp ) Stmt
  
  Args -> Exp
        | Exp , Args
  
  Exp -> true | false
       | Num
       | Id
       | Id ( Args )
       | ( Exp )
       | UnaryOp Exp
       | Exp BinaryOp Exp
  
  UnaryOp  ->  -  |  !
  BinaryOp ->  *  |  /  |  %
            |  +  |  -
            |  <  |  <= |  >  |  >=
            |  == |  !=
```

Notation:
- `Prog -> FuncDef+` indicates that a program consists of one or more function definitions.
- `Exp -> true | false | ...` indicates that an expression can be a `true` or `false` keyword, etc.
- `VarDecl*` indicates zero or more variable declarations
- Other punctuation characters are program literals (e.g. parentheses, braces, semicolon, comma).

Example:
```
int main(int x)
{
    int sum = 0;
    int i = 1;
    while (i <= x)
    {
        sum = sum + i;
        i = i + 1;
    }
}

```

# Source files

Here is an overview of the source files:

- `main.cpp`: calls the lexer, parser, typechecker, and code generator
- `Token.h`: lexical tokens, e.g. constants, identifiers, and keywords.
- `Lexer.re`: regular expressions for lexical tokens (compiled by re2c)
- `TokenStream.h`: adapter that calls Lexer to produce a stream of tokens.
- `Parser.cpp`: recursive descent parser, which reads token stream and produces a syntax tree.
- `Exp.h Stmt.h VarDecl FuncDef.h Program.h`: syntax trees for expressions, statements, functions, etc.
- `Visitor.h`: visitor pattern for syntax traversal
- `Printer.h`: print syntax tree using Visitor
- `Typechecker.h`: a typechecker that supports overloading.
- `Scope.h`: scoped symbol table used by the typechecker.
- `Builtins.h`: declarations of built-in operators
- `Codegen.cpp`: generates LLVM IR from syntax tree
- `SimpleJit.h`: encapsulates LLVM ORC JIT engine

# Building

Prerequisites:

- CMake: https://cmake.org/download/
- LLVM 7.1:  http://releases.llvm.org/download.html

The Weekend Compiler project is built with CMake.  From the command line, the locations of LLVM and re2c can be specified as follows:

  cmake -DLLVM_ROOT_DIR=C:/LLVM-7.1 -DRE2C_EXE=C:/re2c-1.2.1/bin/re2c.exe

Alternatively these locations can be specified via cmake-gui.

The re2c tool is used to compile regular expressions in the lexer.  CMake
automatically downloads it from https://github.com/skvadrik/re2c and builds it
as an ExternalProject.
