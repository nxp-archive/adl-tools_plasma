//
// The cluster class defines a currently running thread and
// contains the logic to preempt it and schedule a new one.
//
#ifndef _CLUSTER_H_
#define _CLUSTER_H_

#include <signal.h>

#include "Thread.h"
#include "ProcQ.h"

namespace plasma {

  class ConfigParms;

  class Cluster 
  {
  public:
    Cluster();
    void init(const ConfigParms &);
    void reset(Proc *p);

    // Main entry point.  This runs until there are no more user threads.
    void scheduler();

    // Causes the current thread to wait on the specified thread.
    void wait(THandle t);

    // Yield on the current thread.  Switch to the next ready thread.
    void yield();

    // Terminate the current thread and switch to the next ready thread.
    void terminate();

    // Terminate a thread.  Will remove from ready queue if it's in it and
    // will mark as done.
    void terminate(THandle t);

    // Current thread sleeps.  When awakened, gets handle value sent by
    // wake command.  Note:  You *must* have some other storage of this thread,
    // since it's removed from the ready queue.
    HandleType sleep();

    // Wake specified thread, switching to it and supplying specified handle.
    void wake(THandle t,HandleType h);

    // Set the priority of the current thread.  Does not do any scheduling.
    // Note:  0 is highest priority from the point of view of the interface,
    // but internally, 0 is considered the highest priority.  We do all conversions
    // in Interface.C so that everything else has 0 as being lowest.
    void set_priority(unsigned p);
    unsigned get_priority() const;
    unsigned lowest_priority() const;

    // Add and remove processor objects.
    // Sets state to Running.
    void add_proc(Proc *);

    // Explicitly switch to the scheduler thread.
    // Caller must lock cluster.
    void runscheduler();

    // Delay current processor by specified amount of time.
    void delay(ptime_t);

    // Consume the specified amount of time.
    void busy(ptime_t);

    // Current time.
    ptime_t time() const;

    void lock();             // lock cluster
    void unlock();           // unlock cluster
    bool locked() const;     // is cluster in kernel mode?

    static void nopreempt();        // switch off alarm preemption
    static void preempt();          // switch on alarm preemption

    void setCur(THandle t);
    THandle curThread() const;

    Proc *curProc() const;

    // Returns true if it's okay to timeslice.
    bool ts_okay() const;

  private:
    friend void *switch_ready(qt_t *sptr, void*, void *old);
    friend void *switch_term(qt_t *, void* old, void *);
    friend void *switch_block(qt_t *sptr, void*, void *old);

    bool in_scheduler() const;

    static bool init();

    // Updates the current processor to one which has work to do.
    // Returns false if none available.
    bool update_proc();
    bool get_new_proc();
    // Updates current time and populates cluster.
    bool update_time();
    // Get next available thread, respecting priorities, from the
    // current processor.
    Thread *get_ready();
    Thread *next_ready() const;
    // Try to remove thread from ready queue (if it exists) from
    // the current processor.
    THandle get_ready(THandle t);
    // Add thread back to its processor, awakening the thread if not busy or
    // waking thread is of higher priority.
    void add_thread_to_proc(Thread *t);

    // Execute thread new, saving data in old.
    void exec_ready(THandle newthread,THandle oldthread);
    void exec_ready();
    // Execute next ready thread.  Old thread (_cur), is not added to ready queue.
    void exec_block();

    friend void resetalarm();

    void print_procs(std::ostream &) const;
    void print_procs() const;

    static bool     _preemptOkay;      // Is preemption allowed?
    static unsigned _timeslice;        // Time slice period in usec.
    static ptime_t  _busyts;           // Busy timeslice in time units.

    ProcQ      _procs;                 // Queue of processors we know about.
    Proc      *_curproc;               // Current processor.
    Thread     _main;                  // Main thread (the scheduler loop itself).
    Thread    *_cur;                   // Current thread.
    bool       _kernel;                // Set if thread is in kernel mode

    static sigset_t _empty_mask;    // empty signal mask
    static sigset_t _alarm_mask;    // mask with SIGALRM
  };

  inline void Cluster::lock(void)
  {
    _kernel = 1;
  }

  inline void Cluster::unlock(void)
  {
    _kernel = 0;
  }

  inline bool Cluster::locked() const
  {
    return _kernel;
  }

  inline void Cluster::setCur(THandle t)
  {
    _cur = t;
  }

  inline THandle Cluster::curThread() const
  {
    return _cur;
  }

  inline Proc *Cluster::curProc() const
  {
    return _curproc;
  }

}

// cluster-object, statically allocated in address space of current process
extern plasma::Cluster thecluster;

#endif

