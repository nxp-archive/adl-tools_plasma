//
// Simple queue class.  Requires that the target class
// have a next pointer inside of itself.  This means that
// queue manipulation requires only some pointer operations
// and not an allocation, as would be requiired by using STL.
//

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "gc/gc_cpp.h"

#include <iosfwd>

namespace plasma {

  // Objects used in the Queue should be derived from this type.
  // Note:  Templates were not used here b/c we deal just with
  // pointers.  Also note that Queue objects are garbage collected.
  class QBase : public gc {
  public:
    QBase() : _prev(0), _next(0) {};

    // Access to queue pointers.
    QBase *getprev() const;
    void setprev(QBase *);
    QBase *getnext() const;
    void setnext(QBase *);

  private:
    QBase *_prev;
    QBase *_next;
  };

  // Very simple queue class of objects.
  class Queue {
  public:
    Queue() : _head(0), _tail(0) {};

    // Add to back.
    void add(QBase *t);
    // Add to front- for using this structure as a stack.
    void push(QBase *t);

    // Get from head- removes item from queue.
    QBase *get();
    // Get item from queue if it exists, returns 0 otherwise.
    // Linear-time algorithm.
    QBase *get(QBase *t);
    // Removes item from queue.
    void remove(QBase *t);
    // Get ptr to top item, or 0 if queue is empty.
    // Does not remove it from the queue.
    QBase *front() const;
    QBase *back() const;

    bool empty() const { return !_head; };

    // O(n) operation for size()!
    unsigned size() const;

    friend std::ostream &operator<<(std::ostream &,const Queue &);
  
  protected:
    QBase *_head; // Head of the queue.
    QBase *_tail; // Tail of the queue.
  };

  inline QBase *QBase::getprev() const
  {
    return _prev;
  }

  inline void QBase::setprev(QBase *t)
  {
    _prev = t;
  }

  inline QBase *QBase::getnext() const
  {
    return _next;
  }

  inline void QBase::setnext(QBase *t)
  {
    _next = t;
  }

  inline QBase *Queue::front() const
  {
    return _head;
  }

  inline QBase *Queue::back() const
  {
    return _tail;
  }

}

#endif
