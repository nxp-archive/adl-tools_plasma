//
// Queue implementation for Processor objects.
//

#include "ProcQ.h"
#include "Proc.h"

#define PROC(x) (reinterpret_cast<Proc*>(x))

namespace plasma {

  // Searches for first non-empty item.
  Proc *ProcQ::get_nonempty()
  {
    if (!_head) return 0;           // Empty.
    if (!(PROC(_head)->empty())) {  // Same as normal get.
      return get();
    }
    Proc *cur = PROC(_head);
    while(cur) {                    // Query next pointer- remove if found.
      Proc *next = PROC(cur->getnext());
      if (next && !next->empty()) {
        cur->setnext(next->getnext());
        if (!cur->getnext()) {
          _tail = cur;
        }
        return next;
      }
      cur = next;
    }
    return 0;                       // Not found.    
  }

}
