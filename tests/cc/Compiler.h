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

  // Main entry point.
  // sym_only:    Stop after symbol table creation.
  // check_only:  Stop after type-checking and flow control analysis.
  void compileUnits(bool sym_only,bool check_only);

  // Print all ASTs.
  void print_ast_list(std::ostream &,bool printsyms);

  // Used internally.
  void compileUnit(Node *,bool,bool);
  bool compile(Node *,bool);
private:

  Nodes &_asts;

  bool   _error;  // Notes that we're in an error condition.
};

#endif
