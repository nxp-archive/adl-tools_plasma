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

namespace plasma {

  unsigned Proc::_numpriorities = 0;

  Proc::Proc() : 
    _ready(_numpriorities),
    _numthreads(0),
    _state(Waiting)
  {
    // Make sure that we've initialized this to a valid value.
    assert(_numpriorities);
  }

  void Proc::init(const ConfigParms &cp)
  {
    if (cp._numpriorities < 1) {
      throw runtime_error("Number of priorities must be greater than 0.");
    }
    _numpriorities = cp._numpriorities;
  }

  // Create a thread and add to ready queue.
  Thread *Proc::create(UserFunc *f,void  *arg)
  {
    thecluster.lock();
    Thread *t = new Thread;
    t->realize(f,arg);
    t->setPriority(thecluster.curThread()->priority());
    t->setProc(this);
    add_ready(t);
    thecluster.unlock();
    return t;
  }

  // Create a thread and add to ready queue.  This allocates extra space at the
  // end of the thread object for use by the caller.  Nbytes of data pointed
  // to by args is copied over to this free space and the thread will receive a
  // pointer to this extra space.
  pair<Thread *,void *> Proc::create(UserFunc *f,int nbytes,void  *args)
  {
    thecluster.lock();
    // Allocate thread object + nbytes.
    void *tmp = Thread::operator new(sizeof(Thread)+nbytes);
    // Construct object at this space.
    Thread *t = new (tmp) Thread;
    // Argument to thread is pointer to free space.
    void *d = t->endspace();
    t->realize(f,d);
    t->setPriority(thecluster.curThread()->priority());
    t->setProc(this);
    // Copy over data to free space.
    memcpy(d,args,nbytes);
    add_ready(t);
    thecluster.unlock();
    return make_pair(t,d);
  }

  // Add thread to relevant ready queue, based upon its
  // priority.  Thread is not added if marked as done.
  void Proc::add_ready(Thread *t)
  {
    if (!t->done()) {
      int p = t->priority();
      _ready[p].add(t);
      ++_numthreads;
    }
  }

  // Get next thread to execute.  Returns 0 if none available.
  // Starts at highest priority and works downwards.
  Thread *Proc::get_ready()
  {
    for (int i = _ready.size()-1; i >= 0; --i) {
      if (Thread *next = _ready[i].get()) {
        --_numthreads;
        return next;
      }
    }
    return 0;
  }

  Thread *Proc::next_ready() const
  {
    for (int i = _ready.size()-1; i >= 0; --i) {
      if (Thread *next = _ready[i].front()) {
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
    Thread *next = _ready[p].get(t);
    if (next) {
      --_numthreads;
    }
    return next;
  }

  // We return the busy time of the highest priority thread in the system.
  // We look at the back of the queue b/c we assume that the busy thread is
  // the one that was just added back to the system.
  ptime_t Proc::endtime() const
  {
    for (int i = _ready.size()-1; i >= 0; --i) {
      if (Thread *n = _ready[i].back()) {
        return n->endtime();
      }
    }
    return 0;
  }

  void Proc::print_ready(ostream &o) const
  {
    o << "Ready queue:  ";
    for (unsigned i = 0; i != _ready.size(); ++i) {
      o << "  Priority " << i << ":  " << _ready[i] << endl;
    }
    o << endl;
  }

  unsigned Proc::numPriorities()
  {
    return _numpriorities;
  }

}
