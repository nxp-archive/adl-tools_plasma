//
// Queue implementation for Processor objects.
//

#ifndef _PROCQ_H_
#define _PROCQ_H_

#include "Queue.h"

namespace plasma {

  class Processor;

  class ProcQ : private Queue
  {
  public:
    void add(Processor *t) { Queue::add(reinterpret_cast<QBase*>(t)); };

    // Get from head- removes item from queue.
    Processor *get() { return reinterpret_cast<Processor*>(Queue::get()); };
    // Get if it exists in queue.  Removes it.
    // Returns 0 if not in queue.
    Processor *get(Processor *t) { return reinterpret_cast<Processor*>(Queue::get(reinterpret_cast<QBase*>(t))); };

    friend std::ostream &operator<<(std::ostream &,const ProcQ &);
  };

  inline std::ostream &operator<<(std::ostream &o,const ProcQ &tq)
  {
    return operator<<(o,reinterpret_cast<const Queue&>(tq));
  }

}

#endif
