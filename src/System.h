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

    void *newstack();                  // return appropriate thread stack
    void dispose(void *stack);         // dispose stack for later reuse

    void shutdown(int code);           // trigger program shutdown 
  
    int retcode() const;

    bool wantShutdown() const;

  private:
    int     _size;             // Default stack size of threads
    void   *_stacks;           // Disposed thread stacks
    int     _code;             // Exit code.
    bool    _wantshutdown;     // Flag indicates that a shutdown is desired.
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
