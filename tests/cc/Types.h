//
// Miscellaneous common types.
//

#ifndef _TYPES_H_
#define _TYPES_H_

#include <vector>

#include "gc_cpp.h"
#include "gc_allocator.h"

class Node;

typedef std::vector<Node *,traceable_allocator<Node *> > Nodes;

#endif

