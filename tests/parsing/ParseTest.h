
#include "opencxx/mop2.h"

// This has to be here b/c of a bug in OpenC++:  It doesn't
// recognize that a qualified name is still a valid metaclass.
using namespace Opencxx;

class ParseTest : public Class {
public:
  // Main entry point for translating new constructs.
  virtual Ptree* TranslateUserPlain(Environment*,Ptree*, Ptree*);
  
  // Setup code.
  static bool Initialize();

};
