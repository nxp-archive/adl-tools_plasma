//
// Simple thread queue class.
//

#ifndef _THREADQ_H_
#define _THREADQ_H_

#include <iosfwd>

namespace plasma {

  class Thread;

  // Very simple queue class of thread objects.
  class ThreadQ {
  public:
    ThreadQ() : _head(0), _tail(0) {};

    void add(Thread *t);
    Thread *get();

    bool empty() const { return !_head; };

    // O(n) operation for size()!
    unsigned size() const;

    void print(std::ostream &) const;
  private:
    Thread *_head; // Head of the queue.
    Thread *_tail; // Tail of the queue.
  };

}

#endif
