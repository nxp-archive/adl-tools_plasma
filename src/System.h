//
// Main system object:  Only one of these should ever exist.
//
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <iosfwd>
#include <queue>

#include "Interface.h"
#include "ThreadQ.h"

namespace plasma {

  class ConfigParms;

  // Returns true if x's time is greater than y's time.
  struct greater_time {
    bool operator()(const Thread *x,const Thread *y);
  };

  // Sorts Threads by descending time- the smallest time value will be first.
  typedef std::priority_queue<Thread*,std::vector<Thread*>,greater_time> PriQueue;

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

    // Add a busy processor.  The time specifies for how long to be busy.
    void add_busy(ptime_t t,Thread *th);

    // Add a delayed thread.  The time specifies for how long to delay.
    void add_delay(ptime_t t,Thread *th);

    // Returns the next thread with the same time as the current time.
    // Returns 0 if no elements remain (at the current time).
    Thread *get_delay();

    // Returns the next processor with the same time as the current time.
    // Returns 0 if no elements remain (at the current time).
    Thread *get_busy();

  private:
    Thread *get_current(PriQueue &q);

    int     _size;             // Default stack size of threads
    void   *_stacks;           // Disposed thread stacks
    int     _code;             // Exit code.
    bool    _wantshutdown;     // Flag indicates that a shutdown is desired.
    bool    _busyokay;         // Is time consumption legal?

    ptime_t   _time;           // Current time of the system.
    PriQueue  _delay;          // Delayed threads.
    PriQueue  _busy;           // Busy processors.
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

  // Main system object (only one per system).
  extern System thesystem;
}

#endif
