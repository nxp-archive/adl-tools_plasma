//
// Simple thread queue class.
//

#include <iostream>

#include "Queue.h"

using namespace std;

namespace plasma {

  void Queue::add(QBase *next)
  {
    next->setnext(0);        // last thread in queue
    if (_tail)               // there was a previous thread in queue
      _tail->setnext(next);  // link previous thread with thread
    else                     // no previous thread in queue
      _head = next;          // this is the first thread in queue
    _tail = next;            // this is the last thread in queue
  }

  QBase *Queue::get()
  {
    if (!_head) return(0);        // cheap test first
    QBase *next = _head;         // first thread in queue
    _head = next->getnext();      // next thread in queue
    if (!_head) _tail = 0;        // queue is empty
    next->setnext(0);
    return(next);                 // return thread
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