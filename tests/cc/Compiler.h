//
// This class takes ASTs and performs all of the compilation passes.
//

#ifndef _COMPILER_H_
#define _COMPILER_H_

#include <iosfwd>

#include "Types.h"

class Node;

class Compiler {
public:
  Compiler();

  // sym_only:    Stop after symbol table creation.
  // check_only:  Stop after type-checking and flow control analysis.
  // return:      False:  An error occurred.
  bool compileUnit(Node *,bool sym_only,bool check_only);

  // Used internally.
  bool compile(Node *,bool);
};

#endif
