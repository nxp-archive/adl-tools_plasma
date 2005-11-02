//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//

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
