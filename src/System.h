//
// Main system object:  Only one of these should ever exist.
//
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <iosfwd>

#include "ThreadQ.h"

namespace plasma {

  class ConfigParms;

  class System                    
  {
  public:
    System();                          // initialize object
    void init(const ConfigParms &);
    int stacksize() const;             // return default stacksize

    bool have_ready() const;           // Return true iff thread available.
    unsigned size_ready() const;       // Number of ready threads- O(n) operation.
    void add_ready(Thread *next);      // Place thread into ready queue.
    Thread *get_ready(void);           // Removes thread from ready queue and returns it.
    void print_ready(std::ostream &) const;

    void *newstack();                  // return appropriate thread stack
    void dispose(void *stack);         // dispose stack for later reuse

    void shutdown(int code);           // trigger program shutdown 
  
    int retcode() const;

    bool wantShutdown() const;

  private:
    ThreadQ _ready;            // Queue of threads ready to execute.

    int     _size;             // Default stack size of threads
    void   *_stacks;           // Disposed thread stacks
    int     _code;             // Exit code.
    bool    _wantshutdown;     // Flag indicates that a shutdown is desired.
  };

  inline int System::stacksize() const 
  { 
    return _size; 
  }

  inline bool System::have_ready() const
  {
    return !_ready.empty();
  }

  inline unsigned System::size_ready() const
  {
    return _ready.size();
  }

  inline int System::retcode() const
  {
    return _code;
  }

  inline bool System::wantShutdown() const
  {
    return _wantshutdown;
  }

  // Place thread into ready queue.
  inline void System::add_ready(Thread *next)
  {
    _ready.add(next);
  }

  // Get thread from ready queue.  Returns 0 if 
  // no threads are ready.
  inline Thread *System::get_ready(void)
  {
    return _ready.get();
  }

  inline void System::print_ready(std::ostream &o) const
  {
    _ready.print(o);
  }

  // Main processor object (only one per system).
  extern System thesystem;

}

#endif
