//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
//
// Line directive leaf- this class stores a line directive and makes
// sure that when the object is written, it starts at the beginning of the line.
//

#ifndef _LDLEAF_H_
#define _LDLEAF_H_

#include <iosfwd>

#include "opencxx/parser/Ptree.h"
#include "opencxx/parser/DupLeaf.h"

namespace Opencxx {
  class Environment;
}

// This derived class exists so that the double-dispatch mechanism will
// end up calling TranslateVariable.  By default, you generally just get
// a character string, which won't work when there are nested par blocks.
class LdLeaf : public Opencxx::DupLeaf {
public:
  LdLeaf(char *s,int l) : Opencxx::DupLeaf(s,l) {};
  int Write(std::ostream&, int);
};

Opencxx::Ptree *lineDirective(Opencxx::Environment *env,Opencxx::Ptree *expr);

#endif
