//
// A proc defines a group of threads.  Having
// multiple processors allows time to advance in parallel
// when using the time model.
//

#ifndef _PROC_H_
#define _PROC_H_

#include "Interface.h"
#include "ThreadQ.h"

namespace plasma {

  // Note:  This class does not do any implicit queue management wrt. Clusters.
  // Therefore, it's up to the user to make sure that a Proc doesn't go out of
  // scope while it's in a Cluster's ready queue.  Normally, end users only deal
  // with Procs through the Processor class, which doesn't delete the object, so
  // garbage collection should only get rid of it when it's completely done.
  class Proc : public QBase {
  public:
    enum State {  Waiting, Running, Busy };

    Proc();

    static void init(const ConfigParms &cp);

    // Create a thread and add to the ready queue.
    THandle create(UserFunc *f,void *arg);
    std::pair<THandle ,void *> create(UserFunc *f,int nbytes,void *args);

    // Add an already created thread to the ready queue.  This must
    // have already been realized.
    void add_ready(THandle t);
    // Get next available thread, respecting priorities.
    Thread *get_ready();    
    // Try to remove thread from ready queue (if it exists).
    THandle get_ready(THandle t);
    // Pointer to next avail thread- does not remove.
    THandle next_ready() const;

    // Processor state.
    State state() const { return _state; };
    void setState(State s) { _state = s; };

    // Number of threads in object.
    unsigned size() const { return _numthreads; };
    // Returns true if queue is empty.
    bool empty() const { return !_numthreads; };

    // Amount of busy time left (if applicable).
    // This is starttime + time.
    ptime_t endtime() const;

    // Printt the ready queue to the specified stream.
    void print_ready(std::ostream &o) const;

    static unsigned numPriorities();

  private:
    static unsigned _numpriorities; // Number of allowed priorities.

    QVect    _ready;          // Ready threads, in priority order.
    int      _numthreads;     // Count of threads in this object.
    State    _state;          // Current processor state.
  };

}

#endif
