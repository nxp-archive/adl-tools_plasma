
//
// Basic thread primitive.
//
#ifndef _THREAD_H_
#define _THREAD_H_

#include "Interface.h"
#include "ThreadQ.h"

typedef struct qt_t qt_t;

namespace plasma {

  class Thread 
  {
  public:
    // Create a thread w/o a stack.  Only used to create the main thread.
    Thread();

    // Allocate a stack for this thread.  The thread will execute f(arg).
    void realize(UserFunc *f,void *arg);
 
    // Destroy real thread i.e. deallocate its stack
    void destroy(void);

    // Saves stack pointer.
    void save(qt_t *sptr);
    qt_t *thread() { return _thread; };

    // Add a thread to the wait list.
    void add_waiter(Thread *t);
    // Get a thread from the wait list.
    Thread *get_waiter();
    // Remove a thread from the wait queue.
    // Returns 0 if it doesn't exist.
    Thread *get_waiter(Thread *t);

    bool done() const { return _done; };

    // Access to queue pointer.
    Thread *getnext() const;
    void setnext(Thread *);

    HandleType handle() const { return _handle; };
    void setHandle(HandleType h) { _handle = h; };

    unsigned priority() const { return _priority; };
    void setPriority(unsigned p) { _priority = p; };

    void *endspace() const { return (void*)&_extraspace[0]; };

    static void *operator new(size_t sz) { return ::operator new(sz); };
    static void *operator new(size_t sz,void *p) { return ::operator new(sz,p); };

  private:
    bool        _done;             // True when thread is done executing.
    ThreadQ     _waiters;          // Threads waiting on this thread.
    qt_t       *_thread;           // Thread handle.
    void       *_stack;            // Stack pointer.
    Thread     *_next;             // Next thread in queue.
    HandleType  _handle;           // Miscellaneous handle value.
    unsigned    _priority;         // Current thread priority.
    char        _extraspace[];     // Allows for extra space to be allocated at end.
  };

  inline Thread::Thread() :
    _done(false),
    _thread(0),
    _stack(0),
    _next(0),
    _priority(0)
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

  inline Thread *Thread::get_waiter(Thread *t)
  {
    return _waiters.get(t);
  }

}

#endif
