//
// This class takes ASTs and performs all of the compilation passes.
//

#ifndef _COMPILER_H_
#define _COMPILER_H_

#include <iosfwd>

#include "Types.h"

#include "gc_cpp.h"
#include "gc_allocator.h"

// Used for spawning off threads and then collecting up their results
// when needed.
typedef plasma::Result<bool> CompRes;
typedef std::vector<CompRes,traceable_allocator<CompRes> > Results;

class Node;
class CodeGen;

class Compiler {
public:
  Compiler();

  // sym_only:    Stop after symbol table creation.
  // check_only:  Stop after type-checking and flow control analysis.
  // return:      False:  An error occurred.
  bool compileUnit(Node *,const char *filename,bool sym_only,bool check_only);

  // Used internally.
  bool dochecks(Node *,bool);

  bool gencode(CodeGen *cg,Node *n);
};

#endif
