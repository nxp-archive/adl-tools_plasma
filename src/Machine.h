//
// Machine-specific code.
//
#ifndef _MACHINE_H_
#define _MACHINE_H_

namespace plasma {

  // ----------------------------------------------------------------------------
  //
  // user configurations
  //
  // ----------------------------------------------------------------------------

  // operating system: define one of the following
  // RT_LINUX   ... Linux
#define RT_LINUX

  // processor: define one of the following
  // RT_i386   ... Intel 386 and compatibles
#define RT_i386

  // thread package: define one of the following
  // RT_QT     ... quick threads package
#define RT_QT

  // ----------------------------------------------------------------------------
  //
  // signal handlers
  // defined SA_HANDLER (handler component of sigaction structure)
  //
  // ----------------------------------------------------------------------------

#ifdef RT_LINUX
#define SA_HANDLER(f) ((void (*)(int))(f))
#endif

  // ----------------------------------------------------------------------------
  //
  // threads
  //
  // ----------------------------------------------------------------------------

#ifdef RT_QT
#include "qt.h"        // the quick threads package
#endif

  // Atomic testAndSet operation
  // (sets the value of an integer to ONE and return its old value)

#ifdef RT_i386
  static inline int testAndSet(volatile int *mutex) 
  {
    int result;
    asm volatile("movl %2, %0; xchgl %0, (%1)" : "=&r" (result) : "r" (mutex),
                 "i" (1) );
    return result;
  }
#endif

}

#endif
