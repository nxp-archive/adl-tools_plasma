//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
//
// Basic thread primitive.
//

#include <iostream>

#include "Machine.h"
#include "Thread.h"
#include "Cluster.h"
#include "System.h"

using namespace std;

namespace plasma {

  /* `alignment' must be a power of 2. */
#define STP_STKALIGN(sp, alignment) \
  ((void *)((((qt_word_t)(sp)) + (alignment) - 1) & ~((alignment)-1)))

  static void shell(void *u, void *t, UserFunc *f)
  {
    // Execute user function with argument.
    (*(UserFunc *)f)(u);
    // We're done, so go on to next thread.
    thecluster.terminate();
  }

  // Allocates a stack and adds this item to
  // the system's list of running threads.
  void Thread::realize(UserFunc *f,void *arg)
  {
    _stack = thesystem.newstack();
    void *sto = STP_STKALIGN (_stack, QT_STKALIGN);
    _thread = QT_SP(sto,thesystem.stacksize()-QT_STKALIGN);
    _thread = QT_ARGS(_thread,arg,this,(qt_userf_t*)f,shell);
    System::add_active_thread(this);
    //cout << "Thread " << this << " realized with stack " << _stack << endl;
  }

  // Destroy real thread i.e. deallocate its stack
  void Thread::destroy()
  {
    thesystem.dispose(_stack);
    //cout << "Thread " << this << " done- disposed of stack." << endl;
    _stack = 0;
    _state = Done;
    System::remove_active_thread(this);
  }

  THandle Thread::get_waiter()
  {
    if (_waiters.empty()) {
      return 0;
    } else {
      THandle t = _waiters.back();
      _waiters.pop_back();
      return t;
    }
  }

  Thread *Thread::get_waiter(Thread *t)
  {
    for (Waiters::iterator i = _waiters.begin(); i != _waiters.end(); ++i) {
      if ( *i == t) {
        _waiters.erase(i);
        return t;
      }
    }
    return 0;
  }

}
