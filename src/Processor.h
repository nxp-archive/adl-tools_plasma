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
    Thread *create(UserFunc *f,void *arg);
    std::pair<Thread *,void *> create(UserFunc *f,int nbytes,void *args);

    // Add an already created thread to the ready queue.  This must
    // have already been realized.
    void add_ready(Thread *t);

    // Causes the current thread to wait on the specified thread.
    void wait(Thread *t);

    // Yield on the current thread.  Switch to the next ready thread.
    void yield();

    // Terminate the current thread and switch to the next ready thread.
    void terminate();

    // Terminate a thread.  Will remove from ready queue if it's in it and
    // will mark as done.
    void terminate(Thread *t);

    // Current thread sleeps.  When awakened, gets handle value sent by
    // wake command.  Note:  You *must* have some other storage of this thread,
    // since it's removed from the ready queue.
    int sleep();

    // Wake specified thread, switching to it and supplying specified handle.
    void wake(Thread *t,int h);

    // Explicitly switch to the scheduler thread.
    // Caller must lock processor.
    void runscheduler();

    void lock();             // lock processor
    void unlock();           // unlock processor
    bool locked() const;     // is processor in kernel mode?

    void nopreempt();        // switch off alarm preemption
    void preempt();          // switch on alarm preemption

    void setCur(Thread *t);
    Thread *getCur() const;

  private:
    friend void *switch_ready(qt_t *sptr, void*, void *old);
    friend void *switch_term(qt_t *, void* old, void *);
    friend void *switch_block(qt_t *sptr, void*, void *old);

    bool in_scheduler() const;

    static bool init();

    // Execute thread new, saving data in old.
    void exec_ready(Thread *newthread,Thread *oldthread);
    // Execute next ready thread.  Old thread (_cur), is not added
    // to ready queue.
    void exec_block();

    friend void resetalarm();

    static unsigned _timeslice;        // Time slice period in usec.

    Thread     _main;                  // Main thread (the scheduler loop itself).
    Thread    *_cur;                   // Current thread.
    bool       _kernel;                // Set if thread is in kernel mode

    static sigset_t _empty_mask;    // empty signal mask
    static sigset_t _alarm_mask;    // mask with SIGALRM
  };

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

  inline void Processor::setCur(Thread *t)
  {
    _cur = t;
  }

  inline Thread *Processor::getCur() const
  {
    return _cur;
  }

  // processor-object, statically allocated in address space of current process
  extern Processor processor;               

}

#endif

