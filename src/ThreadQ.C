//
// Simple thread queue class.
//

#include <iostream>

#include "Thread.h"
#include "ThreadQ.h"

using namespace std;

namespace plasma {

  void ThreadQ::add(Thread *next)
  {
    next->setnext(0);        // last thread in queue
    if (_tail)               // there was a previous thread in queue
      _tail->setnext(next);  // link previous thread with thread
    else                     // no previous thread in queue
      _head = next;          // this is the first thread in queue
    _tail = next;            // this is the last thread in queue
  }

  Thread *ThreadQ::get()
  {
    if (!_head) return(0);        // cheap test first
    Thread *next = _head;         // first thread in queue
    _head = next->getnext();      // next thread in queue
    if (!_head) _tail = 0;        // queue is empty
    next->setnext(0);
    return(next);                 // return thread
  }

  Thread *ThreadQ::get(Thread *t)
  {
    if (!_head || !t) return 0;   // Empty or null query.
    if (_head == t) {             // Same as normal get.
      return get();
    }
    Thread *cur = _head;
    while(cur) {                 // Query next pointer- remove if found.
      Thread *next = cur->getnext();
      if (next == t) {
        cur->setnext(t->getnext());
        return t;
      }
      cur = next;
    }
    return 0;                     // Not found.
  }

  unsigned ThreadQ::size() const
  {
    unsigned size = 0;
    for (Thread *i = _head; i; i = i->getnext(),++size);
    return size;
  }

  void ThreadQ::print(ostream &o) const
  {
    o << "[ ";
    for (Thread *i = _head; i; i = i->getnext()) {
      o << i << " ";
    }
    o << "]";
  }

}
