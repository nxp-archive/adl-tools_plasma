//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//

//
// Basic thread primitive.
//
#ifndef _THREAD_H_
#define _THREAD_H_

#include "Interface.h"
#include "ThreadQ.h"

typedef struct qt_t qt_t;

namespace plasma {

  class Thread : public QBase
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

    void *stack() const { return _stack; };
    void *stackend() const { return _stackend; };
    void *stackbegin() const { return _thread; };

    // Set the current top of the stack for this thread.
    void setStackEnd();
    // Shouldn't normally be used- only used to set the beginning
    // for the main thread.
    void setStackBegin(void *);

    // Add a thread to the wait list.
    void add_waiter(THandle t);
    // Get a thread from the wait list.
    Thread *get_waiter();
    // Remove a thread from the wait queue.
    // Returns 0 if it doesn't exist.
    Thread *get_waiter(THandle t);

    // Ready:  A thread is in ready queue.
    // Run:    Thread is not in the ready queue (may be executing or blocked).
    // Done:   Thread is finished.
    enum State { Done, Ready, Busy, Run };
    State state() const { return _state; };
    void setState(State s) { _state = s; };
    bool done() const { return _state == Done; };

    // State flag available for miscellaneous purposes.
    bool valid() const { return _valid; };
    void setValid(bool v) { _valid = v; };

    Proc *proc() const { return _proc; };
    void setProc(Proc *p) { _proc = p; };

    unsigned priority() const { return _priority; };
    void setPriority(unsigned p) { _priority = p; };

    ptime_t time() const { return _time; };
    ptime_t starttime() const { return _starttime; };
    ptime_t endtime() const { return _starttime + _time; };

    void setTime(ptime_t t) { _time = t; };
    void setStartTime(ptime_t t) { _starttime = t; };    

    void *endspace() const { return (void*)&_extraspace[0]; };

    Thread *nt() const { return _nt; };
    Thread *pt() const { return _pt; };
    void setnt(Thread *n) { _nt = n; };
    void setpt(Thread *p) { _pt = p; };

#   ifdef GC_DISABLED
    static void *operator new(size_t sz) { return ::operator new(sz); };
    static void *operator new(size_t sz,void *p) { return ::operator new(sz,p); };
#   else
    static void *operator new(size_t sz) { return gc::operator new(sz); };
    static void *operator new(size_t sz,void *p) { return gc::operator new(sz,p); };
#   endif

  private:

    typedef std::vector<THandle,gc_allocator<THandle> > Waiters;

    State       _state;            // Current thread state.
    bool        _valid;            // Extra flag usable for state.
    Waiters     _waiters;          // Threads waiting on this thread.
    qt_t       *_thread;           // Thread handle.
    void       *_stack;            // Stack pointer.
    void       *_stackend;         // End of stack pointer.
    Proc       *_proc;             // Parent processor.
    unsigned    _priority;         // Current thread priority.
    ptime_t     _time;             // Busy or delay time of the thread.
    ptime_t     _starttime;        // Start time of a busy or delay.
    Thread     *_pt,*_nt;          // Linked-list pointers for active threads.
    char        _extraspace[];     // Allows for extra space to be allocated at end.
  };

  inline Thread::Thread() :
    _state(Run),
    _valid(true),
    _thread(0),
    _stack(0),
    _stackend(0),
    _proc(0),
    _priority(0),
    _time(0),
    _starttime(0),
    _pt(0),
    _nt(0)
  {
  }

  inline void Thread::save(qt_t *sptr)
  {
    _thread = sptr;
  }

  inline void Thread::setStackEnd()
  {
    volatile void *dummy;
    _stackend = &dummy; 
  }

  inline void Thread::setStackBegin(void *s)
  {
    _thread = (qt_t *)s; 
  }

  inline void Thread::add_waiter(THandle t)
  {
    _waiters.push_back(t);
  }

}

#endif
