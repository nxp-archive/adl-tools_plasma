//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
//
// Main system object:  Only one of these should ever exist.
//
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <iosfwd>
#include <queue>

#include "Interface.h"
#include "ThreadQ.h"

// Rule of thumb minimum to keep the gc happy.
const int StackMin = 0x8000;

namespace plasma {

  class ConfigParms;

  // Returns true if x's time is greater than y's time.
  template <class T1,class T2>
  struct greater_time {
    bool operator()(const T1 *x,const T2 *y);
  };

  // Sorts Threads by descending time- the smallest time value will be first.
  typedef std::priority_queue<Thread*,std::vector<THandle,traceable_allocator<THandle> >,greater_time<Thread,Thread> > TPriQueue;
  struct PPriQueue : public std::priority_queue<Proc*,std::vector<Proc*,traceable_allocator<Proc*> >,greater_time<Proc,Proc> >
  {
    void print() const;
  };

  class System                    
  {
  public:
    System();                          // initialize object
    void init(const ConfigParms &);
    int stacksize() const;             // return default stacksize

    void *newstack();                  // return appropriate thread stack
    void dispose(void *stack);         // dispose stack for later reuse

    void shutdown(int code);           // trigger program shutdown 
  
    int retcode() const;

    bool wantShutdown() const;

    bool busyokay() const;

    // Current time.
    ptime_t time() const { return _time; };

    // This updates time to the next time value, which is the lesser of
    // the top of _busy or _delay.  Returns false if nothing else is
    // available, or true if there is something available.
    bool update_time();

    // Add a busy thread.  The time specifies for how long to be busy.
    // Stores the processor in the busy queue.
    void add_busy(ptime_t t,Thread *th);

    // Add a delayed thread.  The time specifies for how long to delay.
    void add_delay(ptime_t t,Thread *th);

    // Returns the next thread with the same time as the current time.
    // Returns 0 if no elements remain (at the current time).
    Thread *get_delay();

    // Returns the next processor with the same time as the current time.
    // Returns 0 if no elements remain (at the current time).
    Proc *get_busy();

    // Add or remove threads from active list.
    static void add_active_thread(Thread *);
    static void remove_active_thread(Thread *);
    static unsigned num_active_threads();

  private:
    template <class C,class T> T *get_current(C &q);

    static void push_other_roots();

    int     _size;             // Default stack size of threads
    int     _code;             // Exit code.
    bool    _wantshutdown;     // Flag indicates that a shutdown is desired.
    bool    _busyokay;         // Is time consumption legal?

    static Thread *_active_list; // Active threads- used by gc- see push_other_roots() for more info.

    ptime_t   _time;           // Current time of the system.
    TPriQueue _delay;          // Delayed threads.
    PPriQueue _busy;           // Busy processors.
  };

  inline int System::stacksize() const 
  { 
    return _size; 
  }

  inline int System::retcode() const
  {
    return _code;
  }

  inline bool System::wantShutdown() const
  {
    return _wantshutdown;
  }

}

// Main system object (only one per system).
extern plasma::System thesystem;

#endif
