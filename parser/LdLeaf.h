//
// Line directive leaf- this class stores a line directive and makes
// sure that when the object is written, it starts at the beginning of the line.
//

#ifndef _LDLEAF_H_
#define _LDLEAF_H_

#include <iosfwd>

#include "opencxx/ptree.h"

// This derived class exists so that the double-dispatch mechanism will
// end up calling TranslateVariable.  By default, you generally just get
// a character string, which won't work when there are nested par blocks.
class LdLeaf : public DupLeaf {
public:
  LdLeaf(char *s,int l) : DupLeaf(s,l) {};
  int Write(std::ostream&, int);
};

Ptree *lineDirective(Environment *env,Ptree *expr);

#endif
