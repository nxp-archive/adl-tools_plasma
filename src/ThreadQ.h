//
// Queue implementation for Thread objects.
//

#ifndef _THREADQ_H_
#define _THREADQ_H_

#include <vector>

#include "Queue.h"

namespace plasma {

  class Thread;

  class ThreadQ : private Queue
  {
  public:
    void add(Thread *t) { Queue::add(reinterpret_cast<QBase*>(t)); };

    // Get from head- removes item from queue.
    Thread *get() { return reinterpret_cast<Thread*>(Queue::get()); };
    // Get if it exists in queue.  Removes it.
    // Returns 0 if not in queue.
    Thread *get(Thread *t) { return reinterpret_cast<Thread*>(Queue::get(reinterpret_cast<QBase*>(t))); };    
    // Removes from queue.
    void remove(Thread *t) { return Queue::remove(reinterpret_cast<QBase*>(t)); };
    // Top of queue- does not remove item.
    Thread *front() const { return reinterpret_cast<Thread*>(Queue::front()); };
    Thread *back() const { return reinterpret_cast<Thread*>(Queue::back()); };

    friend std::ostream &operator<<(std::ostream &,const ThreadQ &);
  };

  typedef std::vector<ThreadQ,traceable_allocator<ThreadQ> > QVect;

  inline std::ostream &operator<<(std::ostream &o,const ThreadQ &tq)
  {
    return operator<<(o,reinterpret_cast<const Queue&>(tq));
  }

}

#endif
