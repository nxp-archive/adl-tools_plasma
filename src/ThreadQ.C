//
// Simple thread queue class.
//

#include <iostream>

#include "Thread.h"
#include "ThreadQ.h"

using namespace std;

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
