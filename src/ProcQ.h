//
// Queue implementation for Processor objects.
//

#ifndef _PROCQ_H_
#define _PROCQ_H_

#include "Queue.h"

namespace plasma {

  class Proc;

  class ProcQ : private Queue
  {
  public:
    void add(Proc *t) { Queue::add(reinterpret_cast<QBase*>(t)); };

    // Get next item from head.
    Proc *get() { return reinterpret_cast<Proc*>(Queue::get()); };
    // Get if it exists in queue.  Removes it.
    // Returns 0 if not in queue.
    Proc *get(Proc *t) { return reinterpret_cast<Proc*>(Queue::get(reinterpret_cast<QBase*>(t))); };
    // Top of queue- does not remove item.
    Proc *front() const { return reinterpret_cast<Proc*>(Queue::front()); };
    Proc *back() const { return reinterpret_cast<Proc*>(Queue::back()); };

    friend std::ostream &operator<<(std::ostream &,const ProcQ &);
  };

  inline std::ostream &operator<<(std::ostream &o,const ProcQ &tq)
  {
    return operator<<(o,reinterpret_cast<const Queue&>(tq));
  }

}

#endif
