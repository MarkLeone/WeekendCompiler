
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
