//
// Basic thread primitive.
//
#ifndef _THREAD_H_
#define _THREAD_H_

#include "Interface.h"
#include "ThreadQ.h"

typedef struct qt_t qt_t;

class Thread 
{
public:
  // Create a thread w/o a stack.  Only used to create the main thread.
  Thread();

  // Allocate a stack for this thread.  The thread will execute f(arg).
  void realize(UserFunc *f,void *arg);
 
  // Same as above, except nbytes allocated at front of stack.  The
  // pointer to f will be a pointer to this space.  Returns the pointer
  // to the data section.
  void *realize(UserFunc *f,int nbytes);

  // Destroy real thread i.e. deallocate its stack
  void destroy(void);

  // Saves stack pointer.
  void save(qt_t *sptr);
  qt_t *thread() { return _thread; };

  // Add a thread to the wait list.
  void add_waiter(Thread *t);
  // Get a thread from the wait list.
  Thread *get_waiter();

  bool done() const { return _done; };

  // Access to queue pointer.
  Thread *getnext() const;
  void setnext(Thread *);

private:
  bool    _done;             // True when thread is done executing.
  ThreadQ _waiters;          // Threads waiting on this thread.
  qt_t   *_thread;           // Thread handle.
  void   *_stack;            // Stack pointer.
  Thread *_next;             // Next thread in queue.
};

inline Thread::Thread() :
  _done(false),
  _thread(0),
  _stack(0),
  _next(0)
{
}

inline Thread *Thread::getnext() const
{
  return _next;
}

inline void Thread::setnext(Thread *t)
{
  _next = t;
}

inline void Thread::save(qt_t *sptr)
{
  _thread = sptr;
}

inline void Thread::add_waiter(Thread *t)
{
  _waiters.add(t);
}

inline Thread *Thread::get_waiter()
{
  return _waiters.get();
}

#endif
