//
// The cluster class defines a currently running thread and
// contains the logic to preempt it and schedule a new one.
//

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>
#include <stdexcept>

#include "Machine.h"
#include "Cluster.h"
#include "System.h"
#include "Processor.h"

using namespace std;

namespace plasma {

  sigset_t Cluster::_empty_mask;  // empty signal mask
  sigset_t Cluster::_alarm_mask;  // mask with SIGVTALRM
  unsigned Cluster::_timeslice;

  // This is the virtual alarm signal associated with ITIMER_VIRTUAL.
  // It counts CPU time, rather than real time.
  const int AlarmSignal = SIGVTALRM;

  static void preempt(int);        // called on preemption of thread

  // Switch off preemption
  void Cluster::nopreempt(void)
  {
    sigprocmask(SIG_BLOCK, &_alarm_mask, 0);
  }

  // Switch on preemption
  void Cluster::preempt(void)
  {
    sigprocmask(SIG_UNBLOCK, &_alarm_mask, 0);
  }

  // Set handler for alarm timer (preempt thread)
  // AlarmSignal must be blocked during execution of the handler!!!
  static inline void setalarm(void) 
  {
    struct sigaction action;
    action.sa_handler = SA_HANDLER(preempt);
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    int ret = sigaction(AlarmSignal, &action, 0);
    if (ret < 0) pAbort("Installation of alarm handler failed");
  }

  // Resets alarm to 50usec interval.  This is the virtual alarm
  // it only increments while the process is getting CPU time.
  void resetalarm()
  {
    static struct itimerval value = { {0,Cluster::_timeslice}, {0,Cluster::_timeslice} };
    setitimer(ITIMER_VIRTUAL,&value,0);
  }

  // Preempt current thread and reschedule AlarmSignal
  // AlarmSignal is blocked during execution of the handler!!!
  // There is always a thread on the ready queue, since there's always
  // at least the main thread.
  // The cluster will be unlocked by the call to yield(), unless we're 
  // switching to the scheduling thread.
  static void preempt(int) 
  {
    resetalarm();                          // reset alarm timer
    if (!thecluster.ts_okay()) {
      // cluster in kernel mode or running
      // non-preemptable task.
      return;
    }
    thecluster.lock();                      // prevent further preemption
    thecluster.preempt();                   // return of sig handler
    thecluster.yield();                     // switch to new ready thread
    thecluster.nopreempt();                 // in sig handler again
  }

  // The thread for this cluster will only be init'd
  // when we swap to *another* thread.  Typically, at startup,
  // that first thread is the special main routine written by
  // the user.
  Cluster::Cluster() :
    _curproc(0),
    _main(),
    _cur(0),
    _kernel(true)
  {
    static bool dummy = init();

    setalarm();                              // set alarm timer
  }

  // Static initialization.
  bool Cluster::init()
  {
    sigemptyset(&_empty_mask);                // empty signal mask
    sigemptyset(&_alarm_mask);                // mask with AlarmSignal only
    sigaddset(&_alarm_mask, AlarmSignal);
    return true;
  }

  void Cluster::init(const ConfigParms &cp)
  {
    if (!cp._preempt) {
      nopreempt();
    }
    _timeslice = cp._timeslice;
    if (cp._numpriorities < 1) {
      throw runtime_error("Number of priorities must be greater than 0.");
    }
    Processor::setNumPriorities(cp._numpriorities);
    resetalarm();
  }

  // Poll for ready thread and bring it to execution.
  // For now, if there are no ready threads or if the system
  // shutdown flag has been set, we return.  Since the main
  // polling routine is itself a thread, there's always something
  // on the queue, unless we get to here (the scheduler itself).
  // If there's nothing on the queue, it means that this is the only
  // thread, so there are no more user threads and we should exit.
  void Cluster::scheduler()
  {
    // Unlock the cluster to begin.
    unlock();
    // This loops until there are no more items in the queue, then it exits.
    while (1) {
      // Has the system requested a shutdown?
      if (thesystem.wantShutdown()) {
        return;
      }
      // Get a new processor to work with.  If none available, return.
      // Note:  We add to the queue, then take from the queue, so that if
      // only one processor exists, things should still work.
      add_proc(_curproc);
      if (! (_curproc = _procs.get()) ) {
        return;
      }
      // Try to get a ready thread from the current processor.
      //    print_ready();
      if (Thread *ready = get_ready()) {
        // Switch to thread from main thread.  We always save into
        // _main, so that we can clobber current.
        exec_ready(ready,&_main);
      } else {       
        // No thread available:  Exit.
        return;
      }
    }
  }

  // We can timeslice to another thread if we're not in kernel mode and
  // our current thread is at the lowest priority (0).
  inline bool Cluster::ts_okay() const
  {
    return (!locked() && (_cur->priority() == 0));
  }

  // Create a thread and add to ready queue.
  Thread *Cluster::create(UserFunc *f,void  *arg)
  {
    lock();
    Thread *t = new Thread;
    t->realize(f,arg);
    t->setPriority(_cur->priority());
    add_ready(t);
    unlock();
    return t;
  }

  // Create a thread and add to ready queue.  This allocates extra space at the
  // end of the thread object for use by the caller.  Nbytes of data pointed
  // to by args is copied over to this free space and the thread will receive a
  // pointer to this extra space.
  pair<Thread *,void *> Cluster::create(UserFunc *f,int nbytes,void  *args)
  {
    lock();
    // Allocate thread object + nbytes.
    void *tmp = Thread::operator new(sizeof(Thread)+nbytes);
    // Construct object at this space.
    Thread *t = new (tmp) Thread;
    // Argument to thread is pointer to free space.
    void *d = t->endspace();
    t->realize(f,d);
    t->setPriority(_cur->priority());
    // Copy over data to free space.
    memcpy(d,args,nbytes);
    add_ready(t);
    unlock();
    return make_pair(t,d);
  }

  // Causes the current thread to wait on the specified thread.
  void Cluster::wait(Thread *t)
  {
    assert(t);
    // Prevent preemption.
    lock();
    if (t->done()) {
      // Thread has already finished.
      unlock();
      return;
    }
    // Not done- add waiting thread to other thread's wait queue.
    t->add_waiter(_cur);
    //  print_ready();
    // Block and continue to another thread.
    exec_block();
  }

  HandleType Cluster::sleep()
  {
    // Prevent preemption.
    lock();
    // Switch to next thread- do not add this thread back to ready queue.
    exec_block();
    // Return current handle of this thread.
    return _cur->handle();
  }

  void Cluster::wake(Thread *t,HandleType h)
  {
    lock();
    t->setHandle(h);
    Thread *old = _cur;
    exec_ready(t,old);
  }

  // This changes the priority, then yields so that the
  // new priority will take effect.
  void Cluster::set_priority(unsigned p)
  {
    lock();
    if (p >= Processor::numPriorities()) {
      pAbort("Bad priority value");
    }
    _cur->setPriority( convert_priority(p) );
    yield();
    //runscheduler();
  }

  unsigned Cluster::get_priority() const
  {
    return convert_priority(_cur->priority());
  }

  unsigned Cluster::lowest_priority() const
  {
    return Processor::numPriorities()-1;
  }

  inline unsigned Cluster::convert_priority(unsigned priority) const
  {
    int p = priority;
    int s = Processor::numPriorities();
    return (abs((p - (s - 1)) % s));
  }

  // Add thread to relevant ready queue, based upon its
  // priority.  Thread is not added if marked as done.
  void Cluster::add_ready(Thread *t)
  {
    _curproc->add_ready(t);
  }

  // Get next thread to execute.  Returns 0 if none available.
  // Starts at highest priority and works downwards.
  Thread *Cluster::get_ready()
  {
    return _curproc->get_ready();
  }

  // Searches for the specified thread and removes it from the
  // ready queue.  Returns 0 if not found, otherwise the pointer
  // to the thread.
  Thread *Cluster::get_ready(Thread *t)
  {
    return _curproc->get_ready(t);
  }

  // If item is non-null, then add to queue, unless no current
  // processor exists, in which case we store it as the current processor.
  void Cluster::add_proc(Processor *p)
  {
    if (p) {
      if (!_curproc) {
        _curproc = p;
      } else {
        _procs.add(p);
      }
    }
  }

  Processor *Cluster::get_proc()
  {
    return _procs.get();
  }

  Processor *Cluster::get_proc(Processor *p)
  {
    return _procs.get(p);
  }

  // Switch to next running thread from current thread.
  void Cluster::yield()
  {
    lock();
    Thread *ready = get_ready();
    Thread *old = _cur;
    exec_ready(ready,old);
  }

  // Explicitly switch to the scheduler thread.
  void Cluster::runscheduler()
  {
    Thread *old = _cur;
    _cur = &_main;
    exec_ready(&_main,old);
  }

  // Terminate the current thread, continue execution with next ready thread.
  void Cluster::terminate()
  {
    // Lock cluster to prevent preemption.
    lock();
    Thread *ready = get_ready();
    Thread *old = _cur;
    _cur = ready;
    QT_ABORT(switch_term, old, 0, ready->thread());
  }

  // Terminate a thread.  Remove from ready queue if it's in there and mark as done.
  void Cluster::terminate(Thread *t)
  {
    if (!t) return;

    lock();

    // If the thread is already finished, we're done.
    if (t->done()) {
      unlock();
      return;
    }

    // If we're killing the current thread, use the other terminate.
    if (t == _cur) {
      terminate();
      return;
    }

    // Remove from ready queue if it's in there.
    get_ready(t);

    // Mark as done.
    t->destroy();

    unlock();
  }

  // This terminates a thread, returning the stack to the
  // cluster for use by another thread.
  void *switch_term(qt_t *, void* old, void *)
  {
    Thread *oldthread = (Thread*)old;         // get current thread
    oldthread->destroy();                     // free stack
    // For every waiting thread, add it back to the ready queue.
    while (Thread *next = oldthread->get_waiter()) {
      next->setHandle(oldthread->handle());
      thecluster.add_ready(next);
    }
    if (!thecluster.in_scheduler()) {
      // If we're not in (switching to) the scheduler, make sure that preemption
      // is on.  Otherwise, the scheduler will run and unlock the cluster once
      // it chooses a new thread.
      thecluster.unlock();
    }
    return(0);                                // satisfy type checker
  }

  // Returns true if the current thread is the scheduling (main) thread.
  inline bool Cluster::in_scheduler() const
  {
    return (_cur == &_main);
  }

  // Switch from old thread to new thread; old thread is put into ready queue
  inline void Cluster::exec_ready(Thread *newthread,Thread *oldthread)
  {
    _cur = newthread;
    QT_BLOCK(switch_ready, 0, oldthread, newthread->thread());
  }

  // Switch old thread to new thread
  // old thread is to be placed in ready queue
  // caller is in charge of locking cluster!!!
  void *switch_ready(qt_t *sptr, void*, void *old)
  {
    Thread *oldthread = (Thread*)old;
    oldthread->save(sptr);                        // save stack pointer
    thecluster.add_ready(oldthread);               // place in ready queue
    if (!thecluster.in_scheduler()) {
      // If we're not in (switching to) the scheduler, make sure that preemption
      // is on.  Otherwise, the scheduler will run and unlock the cluster once
      // it chooses a new thread.
      thecluster.unlock();
    }
    return(0);                                    // satisfy type checker
  }

  // Switch to a new ready thread.  The old thread is *not* placed back onto the
  // ready queue.
  inline void Cluster::exec_block()
  {
    Thread *newthread = get_ready();
    Thread *old = _cur;
    _cur = newthread;
    QT_BLOCK(switch_block, 0, old, newthread->thread());
  }

  void *switch_block(qt_t *sptr, void*, void *old)
  {
    Thread *oldthread = (Thread*)old;
    oldthread->save(sptr);                        // save stack pointer
    if (!thecluster.in_scheduler()) {
      // If we're not in (switching to) the scheduler, make sure that preemption
      // is on.  Otherwise, the scheduler will run and unlock the cluster once
      // it chooses a new thread.
      thecluster.unlock();
    }
    return(0);                                    // satisfy type checker
  }

}
