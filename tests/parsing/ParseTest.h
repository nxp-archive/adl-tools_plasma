//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//

#include "opencxx/mop2.h"

//
// Note: We now setup this metaclass manually, rather than using occ, so that we
// don't run into issues interfacing with newer gcc compilers, where OCC no
// longer works.
//
class ParseTest : public Opencxx::Class {
public:
  // Main entry point for translating new constructs.
  virtual Opencxx::Ptree* TranslateUserPlain(Opencxx::Environment*,Opencxx::Ptree*, Opencxx::Ptree*);
  
  // Setup code.
  static bool Initialize();

  const char* MetaclassName() { return "ParseTest"; }

};
