//
// A processor defines a group of threads.  Having
// multiple processors allows time to advance in parallel
// when using the time model.
//

#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

#include "Interface.h"
#include "ThreadQ.h"

namespace plasma {

  class Processor : public QBase {
  public:
    Processor();
    ~Processor();

    // Add an already created thread to the ready queue.  This must
    // have already been realized.
    void add_ready(THandle t);
    // Get next available thread, respecting priorities.
    Thread *get_ready();    
    // Try to remove thread from ready queue (if it exists).
    THandle get_ready(THandle t);

    // Printt the ready queue to the specified stream.
    void print_ready(std::ostream &o) const;

    static unsigned numPriorities();
    static void setNumPriorities(unsigned np);
  private:
    static unsigned _numpriorities; // Number of allowed priorities.

    QVect _ready;          // Ready threads, in priority order.
  };

}

#endif
