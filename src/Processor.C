//
// A processor defines a group of threads.  Having
// multiple processors allows time to advance in parallel
// when using the time model.
//

#include <iostream>

#include "Processor.h"
#include "Cluster.h"

using namespace std;

namespace plasma {

  unsigned Processor::_numpriorities = 0;

  Processor::Processor() : 
    _ready(_numpriorities)
  {
    // Make sure that we've initialized this to a valid value.
    assert(_numpriorities);

    // Add ourselves to the cluster's processor queue.
    thecluster.add_proc(this);
  }

  Processor::~Processor()
  {
    // Remove ourselves from the processor queue.
    thecluster.get_proc(this);
  }

  // Add thread to relevant ready queue, based upon its
  // priority.  Thread is not added if marked as done.
  void Processor::add_ready(Thread *t)
  {
    if (!t->done()) {
      _ready[t->priority()].add(t);
    }
  }

  // Get next thread to execute.  Returns 0 if none available.
  // Starts at highest priority and works downwards.
  Thread *Processor::get_ready()
  {
    for (int i = _ready.size()-1; i >= 0; --i) {
      if (Thread *next = _ready[i].get()) {
        return next;
      }
    }
    return 0;
  }

  // Searches for the specified thread and removes it from the
  // ready queue.  Returns 0 if not found, otherwise the pointer
  // to the thread.
  Thread *Processor::get_ready(Thread *t)
  {
    int p = t->priority();
    return _ready[p].get(t);
  }

  void Processor::print_ready(ostream &o) const
  {
    o << "Ready queue:  ";
    for (unsigned i = 0; i != _ready.size(); ++i) {
      o << "  Priority " << i << ":  " << _ready[i] << endl;
    }
    o << endl;
  }

  unsigned Processor::numPriorities()
  {
    return _numpriorities;
  }

  void Processor::setNumPriorities(unsigned np)
  {
    _numpriorities = np;
  }

}
