//
// This class takes ASTs and performs all of the compilation passes.
//

#ifndef _COMPILER_H_
#define _COMPILER_H_

#include <iosfwd>

#include "Types.h"

class Compiler {
public:
  Compiler( Nodes &);
  // Query error condition.
  bool error() const { return _error; };

  // Main entry point- 
  void compile();

  // Print all ASTs.
  void print_ast_list(std::ostream &,bool printsyms);

  void compile(Node *tu);
private:

  Nodes &_asts;

  bool   _error;  // Notes that we're in an error condition.
};

#endif
