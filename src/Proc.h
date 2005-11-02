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

    // Creates a standalone processor.
    Proc(const char *name = 0);
    Proc(Proc *parent,const char *name = 0);

    static void init(const ConfigParms &cp);

    const char *name() const { return _name; };
    void setName(const char *n) { _name = n; };

    // Create a thread and add to the ready queue.
    THandle create(UserFunc *f,void *arg,int pr = -1);
    std::pair<THandle ,void *> create(UserFunc *f,int nbytes,void *args,int pr = -1);

    // Add an already created thread to the ready queue.  This must
    // have already been realized.
    void add_ready(THandle t);
    // Add a thread to the busy queue.
    void add_busy(THandle t);
    // Get next available thread, respecting priorities.
    THandle get_ready();    
    // Try to remove thread from ready queue (if it exists).
    THandle get_ready(THandle t);
    // Pointer to next avail thread- does not remove.
    THandle next_ready() const;

    // Remove thread.  Use with care, as we can't know if
    // the item is actually in the queue.
    void remove_ready(THandle t);

    // Processor state.
    State state() const { return _state; };
    void setState(State s) { _state = s; };

    // Access busy thread.
    Thread *busythread() const { return _busythread; };
    void clearBusyThread();

    // Number of threads in object.
    unsigned size() const { return _ready->thread_size(); };
    // Returns true if queue is empty.
    bool empty() const { return _ready->thread_empty(); };

    // Amount of busy time left (if applicable).
    // This is starttime + time.
    ptime_t endtime() const;

    // Printt the ready queue to the specified stream.
    void print_ready(std::ostream &o) const;
    // Prints to cout.
    void print_ready() const;

    static unsigned numPriorities();

  private:
    THandle return_thread(THandle t);
    void init_internal(const char *n);

    static unsigned _numpriorities; // Number of allowed priorities.

    const char *_name;           // Optional name- memory not managed.
    QVect      *_ready;          // Ready threads, in priority order.
    THandle     _busythread;     // Current busy thread, if any.
    State       _state;          // Current processor state.
  };

}

#endif
