//
// The processor class defines a currently running thread and
// contains the logic to preempt it and schedule a new one.
//
#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

#include <signal.h>

#include "Thread.h"

namespace plasma {

  class ConfigParms;

  class Processor 
  {
  public:
    Processor();
    void init(const ConfigParms &);

    // Main entry point.  This runs until there are no more user threads.
    void scheduler();

    // Create a thread and add to the ready queue.
    THandle create(UserFunc *f,void *arg);
    std::pair<THandle ,void *> create(UserFunc *f,int nbytes,void *args);

    // Add an already created thread to the ready queue.  This must
    // have already been realized.
    void add_ready(THandle t);

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
    // but internally, 0 is considered the highest priority.  Thus, use these
    // interface functions to get/set priorities rather than querying the thread
    // directly.
    void set_priority(unsigned p);
    unsigned get_priority() const;
    unsigned lowest_priority() const;

    // Explicitly switch to the scheduler thread.
    // Caller must lock processor.
    void runscheduler();

    void lock();             // lock processor
    void unlock();           // unlock processor
    bool locked() const;     // is processor in kernel mode?

    void nopreempt();        // switch off alarm preemption
    void preempt();          // switch on alarm preemption

    void setCur(THandle t);
    THandle getCur() const;

    // Returns true if it's okay to timeslice.
    bool ts_okay() const;

    // Print all ready queues.
    void print_ready(std::ostream &) const;

  private:
    friend void *switch_ready(qt_t *sptr, void*, void *old);
    friend void *switch_term(qt_t *, void* old, void *);
    friend void *switch_block(qt_t *sptr, void*, void *old);

    bool in_scheduler() const;

    unsigned convert_priority(unsigned p) const;

    static bool init();

    // Get next available thread, respecting priorities.
    Thread *get_ready();    
    // Try to remove thread from ready queue (if it exists).
    THandle get_ready(THandle t);

    // Execute thread new, saving data in old.
    void exec_ready(THandle newthread,THandle oldthread);
    // Execute next ready thread.  Old thread (_cur), is not added to ready queue.
    void exec_block();

    friend void resetalarm();

    static unsigned _timeslice;        // Time slice period in usec.

    QVect      _ready;                 // Vector of queues- element 0 is lowest priority.
    Thread     _main;                  // Main thread (the scheduler loop itself).
    Thread    *_cur;                   // Current thread.
    bool       _kernel;                // Set if thread is in kernel mode

    static sigset_t _empty_mask;    // empty signal mask
    static sigset_t _alarm_mask;    // mask with SIGALRM
  };

  inline unsigned Processor::lowest_priority() const
  {
    return _ready.size()-1;
  }

  inline void Processor::lock(void)
  {
    _kernel = 1;
  }

  inline void Processor::unlock(void)
  {
    _kernel = 0;
  }

  inline bool Processor::locked() const
  {
    return _kernel;
  }

  inline void Processor::setCur(THandle t)
  {
    _cur = t;
  }

  inline THandle Processor::getCur() const
  {
    return _cur;
  }

  // processor-object, statically allocated in address space of current process
  extern Processor processor;               

}

#endif

