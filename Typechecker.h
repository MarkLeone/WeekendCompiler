#pragma once

class Program;

/// Typecheck the given program.  Reports an error and returns a non-zero
/// value if a type error is encountered.  The typechecker decorates each
/// expression with its type, and it resolves lexical scoping, linking
/// variable references and function calls to the corresponding definitions.
/// This allows subsequent passes (e.g. Codegen) to operate without any
/// knowledge of scoping rules.
int Typecheck( Program& program );


