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
    // Add to back.
    void add(Thread *t) { Queue::add(reinterpret_cast<QBase*>(t)); };
    // Add to fron.
    void push(Thread *t) { Queue::push(reinterpret_cast<QBase*>(t)); };

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

    using Queue::empty;

    friend std::ostream &operator<<(std::ostream &,const ThreadQ &);
  };

  struct QVect : public std::vector<ThreadQ,traceable_allocator<ThreadQ> > {
    QVect(unsigned np) : 
      std::vector<ThreadQ,traceable_allocator<ThreadQ> >(np),
      _count(0)
    {};

    bool thread_empty() const { return _count == 0; };
    unsigned thread_size() const { return _count; };

    // This should be made private at some point.
    int _count;
  };

  inline std::ostream &operator<<(std::ostream &o,const ThreadQ &tq)
  {
    return operator<<(o,reinterpret_cast<const Queue&>(tq));
  }

}

#endif
