//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
//
// A proc defines a group of threads.  Having
// multiple processors allows time to advance in parallel
// when using the time model.
//

#include <iostream>
#include <stdexcept>

#include "Proc.h"
#include "Cluster.h"

using namespace std;

void setup();

namespace plasma {

  unsigned Proc::_numpriorities = 0;

  // Ensure that we've initialized everything.  This guards against
  // Processors declared globally in a user program occurring before
  // setup time.
  Proc::Proc(const char *n) :
    _ready(new QVect(numPriorities()))
  {
    init_internal(n);
  }

  // Create a processor with a shared ready queue.
  Proc::Proc(Proc *parent,const char *n) :
    _ready(parent->_ready)
  {
    init_internal(n);
  }

  inline void Proc::init_internal(const char *n)
  {
    _busythread = 0;
    _name = n;
    _state = Waiting;
  }

  void Proc::init(const ConfigParms &cp)
  {
    if (cp._numpriorities < 1) {
      throw runtime_error("Number of priorities must be greater than 0.");
    }
    _numpriorities = cp._numpriorities;
  }

  // Create a thread and add to ready queue.
  Thread *Proc::create(UserFunc *f,void  *arg,int pr)
  {
    thecluster.lock();
    Thread *t = new Thread;
    t->realize(f,arg);
    t->setPriority((pr < 0) ? thecluster.curThread()->priority() : pr);
    t->setProc(this);
    add_ready(t);
    thecluster.unlock();
    return t;
  }

  // Create a thread and add to ready queue.  This allocates extra space at the
  // end of the thread object for use by the caller.  Nbytes of data pointed
  // to by args is copied over to this free space and the thread will receive a
  // pointer to this extra space.
  pair<Thread *,void *> Proc::create(UserFunc *f,int nbytes,void  *args,int pr)
  {
    thecluster.lock();
    // Allocate thread object + nbytes.
    void *tmp = Thread::operator new(sizeof(Thread)+nbytes);
    // Construct object at this space.
    Thread *t = new (tmp) Thread;
    // Argument to thread is pointer to free space.
    void *d = t->endspace();
    t->realize(f,d);
    t->setPriority((pr < 0) ? thecluster.curThread()->priority() : pr);
    t->setProc(this);
    // Copy over data to free space.
    memcpy(d,args,nbytes);
    add_ready(t);
    thecluster.unlock();
    return make_pair(t,d);
  }

  // Add thread to relevant ready queue, based upon its
  // priority.  Thread is not added if done or ready (since
  // it's already been added).
  void Proc::add_ready(THandle t)
  {
    if (t->state() == Thread::Run || t->state() == Thread::Busy) {
      t->setState(Thread::Ready);
      int p = t->priority();
      (*_ready)[p].add(t);
      ++_ready->_count;
    }
  }

  // Add a busy thread.
  void Proc::add_busy(THandle t)
  {
    if (t->state() == Thread::Run) {
      t->setState(Thread::Busy);
      _busythread = t;
    }
  }

  inline THandle Proc::return_thread(THandle t)
  {
    --_ready->_count;
    t->setState(Thread::Run);
    return t;
  }

  // Get next thread to execute.  Returns 0 if none available.  Starts with next
  // item in the busy queue, then goes to highest priority and works downwards.
  Thread *Proc::get_ready()
  {
    for (int i = (*_ready).size()-1; i >= 0; --i) {
      if (Thread *next = (*_ready)[i].get()) {
        return return_thread(next);
      }
    }
    return 0;
  }

  Thread *Proc::next_ready() const
  {
    for (int i = (*_ready).size()-1; i >= 0; --i) {
      if (Thread *next = (*_ready)[i].front()) {
        return next;
      }
    }
    return 0;
  }
  
  // Searches for the specified thread and removes it from the
  // ready queue.  Returns 0 if not found, otherwise the pointer
  // to the thread.
  Thread *Proc::get_ready(Thread *t)
  {
    int p = t->priority();
    if (Thread *next = (*_ready)[p].get(t)) {
      return return_thread(next);
    }
    return 0;
  }

  // Don't bother checking the busy queue b/c this should only
  // be used for moving the scheduler thread.
  void Proc::remove_ready(THandle t)
  {
    assert(t->proc() == this);
    int p = t->priority();
    (*_ready)[p].remove(t);
    return_thread(t);
  }

  void Proc::clearBusyThread()
  {
    if (_busythread) {
      add_ready(_busythread);
      _busythread = 0;
    }
  }

  // We return the busy time of the highest priority thread in the system.
  // We look at the back of the queue b/c we assume that the busy thread is
  // the one that was just added back to the system.
  ptime_t Proc::endtime() const
  {
    return (_busythread ) ? _busythread->endtime() : 0;
  }

  void Proc::print_ready(ostream &o) const
  {
    o << "Ready queue:  ";
    for (unsigned i = 0; i != (*_ready).size(); ++i) {
      o << "  Priority " << i << ":  " << (*_ready)[i] << endl;
    }
    o << endl;
  }

  // Useful for workin with gdb.  Prints to cerr so that it's unbuffered.
  void Proc::print_ready() const
  {
    print_ready(cerr);
  }

  unsigned Proc::numPriorities()
  {
    // Ensures proper initialization.
    setup();
    // Make sure that we've initialized this to a valid value.
    assert(_numpriorities);
    return _numpriorities;
  }

}
