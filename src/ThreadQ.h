//
// Simple thread queue class.
//

#ifndef _THREADQ_H_
#define _THREADQ_H_

#include <iosfwd>
#include <vector>

namespace plasma {

  class Thread;

  // Very simple queue class of thread objects.
  class ThreadQ {
  public:
    ThreadQ() : _head(0), _tail(0) {};

    void add(Thread *t);

    // Get from head- removes item from queue.
    Thread *get();
    // Get if it exists in queue.  Removes it.
    // Returns 0 if not in queue.
    Thread *get(Thread *t);

    bool empty() const { return !_head; };

    // O(n) operation for size()!
    unsigned size() const;

    friend std::ostream &operator<<(std::ostream &,const ThreadQ &);
  private:
    Thread *_head; // Head of the queue.
    Thread *_tail; // Tail of the queue.
  };

  typedef std::vector<ThreadQ> QVect;

}

#endif
