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

  class Proc : public QBase {
  public:
    Proc();
    ~Proc();

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

    // Number of threads in object.
    unsigned size() const { return _numthreads; };
    // Returns true if queue is empty.
    bool empty() const { return !_numthreads; };

    // Printt the ready queue to the specified stream.
    void print_ready(std::ostream &o) const;

    static unsigned numPriorities();
    static void setNumPriorities(unsigned np);
  private:
    static unsigned _numpriorities; // Number of allowed priorities.

    QVect _ready;          // Ready threads, in priority order.
    int   _numthreads;     // Count of threads in this object.
  };

}

#endif
