//
// Simple thread queue class.
//

#include <assert.h>
#include <iostream>

#include "Queue.h"

using namespace std;

namespace plasma {

  void Queue::add(QBase *next)
  {
    // Assertion:  Item must not already be in a queue.
    // This doesn't catch the case where there's only a single item,
    // though.
    assert(!next->getnext() && !next->getprev());
    next->setnext(0);        // last thread in queue
    if (_tail) {             // there was a previous thread in queue
      _tail->setnext(next);  // link previous thread with thread
      next->setprev(_tail);
    } else {                 // no previous thread in queue
      _head = next;          // this is the first thread in queue
      next->setprev(0);
    }
    _tail = next;            // this is the last thread in queue
  }

  void Queue::push(QBase *next)
  {
    // Assertion:  Item must not already be in a queue.
    // This doesn't catch the case where there's only a single item,
    // though.
    assert(!next->getnext() && !next->getprev());
    next->setprev(0);        // first thread in queue.
    if (_head) {             // there was a previous thread in queue
      _head->setprev(next);  // previous head points back to new item
      next->setnext(_head);
    } else {                 // no previous thread in queue
      _tail = next;          // this is the first thread in queue
      next->setnext(0);
    }
    _head = next;            // this is the first thread in queue
  }

  QBase *Queue::get()
  {
    if (!_head) return(0);       // cheap test first
    QBase *next = _head;         // first thread in queue
    _head = next->getnext();     // next thread in queue
    if (!_head) {
      _tail = 0;                 // queue is empty
    } else {
      _head->setprev(0);
    }
    next->setprev(0);
    next->setnext(0);
    return(next);                // return thread
  }

  QBase *Queue::get(QBase *t)
  {
    if (!_head || !t) return 0;   // Empty or null query.
    if (_head == t) {             // Same as normal get.
      return get();
    }
    QBase *cur = _head;
    while(cur) {                 // Query next pointer- remove if found.
      QBase *next = cur->getnext();
      if (next == t) {
        cur->setnext(t->getnext());
        if (!cur->getnext()) {
          _tail = cur;
        }
        return t;
      }
      cur = next;
    }
    return 0;                     // Not found.
  }

  void Queue::remove(QBase *x)
  {
    if (!x->getprev()) {
      // Is head of queue- normal remove case.
      // Also handles case where there's only a single element.
      get();
      return;
    } 
    // Middle or tail case- remove x from queue.
    // No need to adjust head or tail.
    x->getprev()->setnext(x->getnext());
    if (!x->getnext()) {
      // Tail case.
      _tail = x->getprev();
    }
    // For safety.
    x->setnext(0);
    x->setprev(0);
  }

  unsigned Queue::size() const
  {
    unsigned size = 0;
    for (QBase *i = _head; i; i = i->getnext(),++size);
    return size;
  }

  ostream &operator<<(ostream &o,const Queue &q)
  {
    o << "[ ";
    for (QBase *i = q._head; i; i = i->getnext()) {
      o << i << " ";
    }
    o << "]";
    return o;
  }

}
