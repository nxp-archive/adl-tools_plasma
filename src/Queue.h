//
// Simple queue class.  Requires that the target class
// have a next pointer inside of itself.  This means that
// queue manipulation requires only some pointer operations
// and not an allocation, as would be requiired by using STL.
//

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <iosfwd>

namespace plasma {

  // Objects used in the Queue should be derived from this type.
  // Note:  Templates were not used here b/c we deal just with
  // pointers.
  class QBase {
  public:
    QBase() : _next(0) {};

    // Access to queue pointer.
    QBase *getnext() const;
    void setnext(QBase *);

  private:
    QBase *_next;
  };

  // Very simple queue class of objects.
  class Queue {
  public:
    Queue() : _head(0), _tail(0) {};

    void add(QBase *t);

    // Get from head- removes item from queue.
    QBase *get();
    // Get if it exists in queue.  Removes it.
    // Returns 0 if not in queue.
    QBase *get(QBase *t);
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
