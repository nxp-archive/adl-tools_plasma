//
// Miscellaneous common types.
//

#ifndef _TYPES_H_
#define _TYPES_H_

#include <vector>

#include "gc_cpp.h"
#include "gc_allocator.h"

// Used for spawning off threads and then collecting up their results
// when needed.
typedef plasma::Result<bool> CompRes;
typedef std::vector<CompRes,traceable_allocator<CompRes> > Results;


#endif

