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
#include "Proc.h"

using namespace std;

namespace plasma {

  sigset_t Cluster::_empty_mask;  // empty signal mask
  sigset_t Cluster::_alarm_mask;  // mask with SIGVTALRM
  unsigned Cluster::_timeslice;
  ptime_t  Cluster::_busyts;
  bool     Cluster::_preemptOkay = true;

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
    // Only allow if global flag not set.
    if (_preemptOkay) {
      sigprocmask(SIG_UNBLOCK, &_alarm_mask, 0);
    }
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
    _curproc(),
    _main(),
    _cur(0),
    _kernel(true)
  {
    static bool dummy = init();

    System::add_active_thread(&_main);
    _main.setStackBegin(GC_stackbottom);

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
    // We don't use preemption if we're using the time model
    // or the user has turned off preemption.
    if (!cp._preempt || cp._busyokay) {
      _preemptOkay = false;
      nopreempt();
    }
    _timeslice = cp._timeslice;
    _busyts = cp._simtimeslice;
  }

  void Cluster::reset(Proc *p)
  {
    _curproc = p;
    p->setState(Proc::Running);

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
      // Setup current processor.  If no processors exist that have work to do, exit.
      // We only do this in the scheduler because we do not timeslice between processors-
      // each processor executes until finished.
      if (!update_proc()) {
        return;
      }
      // Try to get a ready thread from the current processor.
      if (Thread *ready = get_ready()) {
        // Switch to thread from main thread.  We always save into
        // _main, so that we can clobber current.  We update main's
        // processor to the current processor, since the scheduler hops
        // around so that the current processor always has at least one thread
        // to which it can yield.
        _main.setProc(_curproc);
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

  // This adds a thread to a processor and adds the processor
  // to the processor queue if the processor is either not busy or
  // it is busy and the thread has a higher priority than the busy
  // thread.
  void Cluster::add_thread_to_proc(Thread *t)
  {
    Proc *p = t->proc();
    p->add_ready(t);
    if (p->state() == Proc::Busy) {
      Thread *bt = p->busythread();
      assert(bt);
      if (t->priority() > bt->priority()) {
        bt->setTime((time()-bt->starttime()));
        add_proc(p);
      }
    } else {
      add_proc(p);
    }
  }

  // We simply add the new thread to the ready queue so that the
  // operation is valid even if it's on another processor.  One exception
  // to this is that if the processor is busy, we must examine the priority
  // of the busy thread vs. the thread we're trying to wake.  If the waking
  // thread has a higher priority, then it is awakened and the busy thread will
  // re-busy itself to use up all of its time.  If the waking thread has a lower
  // priority, then we wake the thread, but the processor stays busy- when it
  // completes, the waking thread will then execute.
  void Cluster::wake(Thread *t,HandleType h)
  {
    lock();
    t->setHandle(h);
    add_thread_to_proc(t);
    unlock();
  }

  // This will wake a busy processor- it's usually used in conjunction with
  // busysleep.
  void Cluster::busywake(Thread *t,HandleType h)
  {
    lock();
    t->setHandle(h);
    Proc *p = t->proc();
    p->clearBusyThread();
    add_proc(p);
    unlock();
  }

  void Cluster::delay(ptime_t t)
  {
    lock();
    // Add to delay queue.
    thesystem.add_delay(t,_cur);
    // Switch to next thread- do not add this thread back to ready queue.
    exec_block();
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

  // This causes a thread to sleep until something else wakes it up.
  HandleType Cluster::sleep()
  {
    // Prevent preemption.
    lock();
    // Switch to next thread- do not add this thread back to ready queue.
    exec_block();
    // Return current handle of this thread.
    return _cur->handle();
  }

  // Assuming we've setup the thread and processor to be busy, now we update time.
  void Cluster::do_busy(ptime_t requested_time)
  {
    // If we already have a busy thread, it's b/c we're a higher priority thread
    // preempting the other.  Add the other back to the schedule queue so that it'll
    // be run later.
    _curproc->clearBusyThread();
    // In case of processors sharing an issue queue, make sure that thread has
    // correct processor.
    _cur->setProc(_curproc);
    // Add to various busy queues.
    thesystem.add_busy(requested_time,_cur);
    // We have to move the scheduling thread from the old processor to the
    // new processor.  We do this *before* updating the processor so that we make
    // our decision of whether to advance time or not based upon whether real threads exist,
    // not just the scheduling thread.
    _main.proc()->remove_ready(&_main);
    // Update- we know something is available b/c we just added something.
    // This updates the time.
    get_new_proc();
    // Now add the scheduling thread back in to the new processor.
    _main.setProc(_curproc);
    _curproc->add_ready(&_main);
    // Switch to new item w/o adding current thread to ready queue.
    // We make sure that we're not going to switch to ourselves.  This can
    // occur if, for example, there is only one thread in the processor.  This
    // means that we've updated the time and what we added above is now back in
    // the queue.
    if (_curproc->next_ready() != _cur) {
      // Switch to next thread.
      exec_block();
    } else {
      // We would be switching to ourselves, so just remove from queue.
      _curproc->get_ready();
    }
  }

  // This is a cross between sleep and busy.  The processor remains busy
  // until something wakes it up.
  HandleType Cluster::busysleep(ptime_t ts)
  {
    if (!thesystem.busyokay()) {
      pAbort("Error:  Attempt to consume time with pBusy(), but this system was not configured for time consumption.");
    }
    // Loop until the handle is updated.  This handles the case where a higher priority thread
    // wakes up the processor- we don't want to continue until we've gotten an actual signal.
    _cur->clearHandle();
    do {
      // Setup the thread.  Do not add to busy queue, since we're doing this for an indefinite
      // amount of time.
      do_busy(ts);
    } while (!_cur->validHandle());
    // Return current handle of this thread.
    return _cur->handle();
  }

  // Normal busy call- this will make the processor busy for a certain amount of time.
  // The user can supply a timeslice amount for lowest-priority processes.  If <= 0, the
  // default timeslice will be used.
  void Cluster::busy(ptime_t total_time,ptime_t userts)
  {
    if (!thesystem.busyokay()) {
      pAbort("Error:  Attempt to consume time with pBusy(), but this system was not configured for time consumption.");
    }
    // Loop until all time has been used.
    // Processor is not locked b/c preemption is deactivated in busy-okay mode.
    //cout << "\nThread " << _cur << ":  busy for " << total_time << endl;
    while (total_time) {
      // Add to system busy queue.
      // If we're dealing with a time-slice process, only add for the timeslice amount.
      ptime_t requested_time = 
      (_cur->priority() || total_time < _busyts) ? total_time : 
      ((userts > 0) ? userts : _busyts);
      // Update time.
      do_busy(requested_time);
      // Update time remaining- loop if we still have busy stuff to do.
      //cout << "\nThread " << _cur << ":  woke up at " << time() << ", initial time remaining is " << total_time 
      //      << ", thread used " << _cur->time() << endl;
      total_time -= _cur->time();
      //cout << "\nThread " << _cur << ":  woke up at " << time() << ", time remaining is " << total_time << endl;
    }
  }

  ptime_t Cluster::time() const 
  { 
    return thesystem.time(); 
  }

  // This changes the priority, then yields so that the
  // new priority will take effect.
  void Cluster::set_priority(unsigned p)
  {
    lock();
    unsigned op = _cur->priority();
    _cur->setPriority( p );
    // If the new priority is higher than what we have, we don't
    // swap, since we will be switching to ourselves again.
    if (p <= op) {
      _curproc->add_ready(_cur);
      exec_block();
    }
  }

  unsigned Cluster::get_priority() const
  {
    return (_cur) ? _cur->priority() : 0;
  }

  // Get next thread to execute.  Returns 0 if none available.
  // Starts at highest priority and works downwards.  This also
  // updates the current processor, switching to one which has work
  // to do.
  inline Thread *Cluster::get_ready()
  {
    return _curproc->get_ready();
  }

  inline Thread *Cluster::next_ready() const
  {
    return _curproc->next_ready();
  }

  // Searches for the specified thread and removes it from the
  // ready queue.  Returns 0 if not found, otherwise the pointer
  // to the thread.
  Thread *Cluster::get_ready(Thread *t)
  {
    return _curproc->get_ready(t);
  }

  // We only add to the queue if the processor is not already running.  This
  // avoids duplicate entries, since many threads share a single processor.
  void Cluster::add_proc(Proc *p)
  {
    if (p && !(p->state() == Proc::Running)) {
      p->setState(Proc::Running);
      _procs.add(p);
    }
  }

  // Updates current processor to a new processor with threads.  If we have no
  // more processors available, then advances time.  If that still yields
  // nothing, then returns false.  If the current processor is empty, we set
  // it to the Waiting state.
  inline bool Cluster::update_proc()
  {
    assert(_curproc);
    if (_curproc->empty()) {
      // Update state.
      _curproc->setState(Proc::Waiting);
      if (!get_new_proc()) {
        return false;
      }
    }
    return true;
  }

  // Gets a new processor.  If none is available, advances time.
  // If nothing available, returns false.
  // The current processor is overwritten, so it should exist somewhere else.
  bool Cluster::get_new_proc()
  {
    // No threads- get next processor.  We assume that this processor
    // is stored in some waiting thread, so we can clobber this pointer.
    // If we don't get a processor that has a thread, we advance time.
    // We call get_non_empty, rather than just get, b/c there might be processors
    // w/o threads if we have processors sharing a common queue- our current
    // processor might have used up the threads that the other processors thought
    // they had.  However, we don't want to remove these processors b/c we want
    // them available should a processor become busy.  Therefore, we leave them in.
    // This looping shouldn't cause much of a performance issue b/c most of the time
    // the first processor in the queue will have threads available.
    if (! (_curproc = _procs.get_non_empty())) {
      // No new processors at this time, so try to advance time.  If that
      // fails, then we fail.
      return update_time();
    }
    return true;
  }

  // Updates system time and populates cluster with stuff that's ready to run.
  // Returns false if nothing is available.
  bool Cluster::update_time()
  {       
    // No processors in ready queue- try to advance time.  Repeat until there is
    // either nothing left to do or we get a processor and it has a thread.  We
    // have to check for the empty processor case b/c it's possible that a
    // delayed thread was terminated while it was delayed.
    while (!_curproc) {
      if ( !thesystem.update_time()) {
        return false;
      }
      // New time is valid- there's stuff to do.
      // Grab stuff from delay queue.
      Thread *t;
      while ( (t = thesystem.get_delay())) {
        // Add thread to processor and set processor to run.
        // If processor of delayed thread is busy, then only run it if its
        // priority is higher.  In that case, we record the amount of time
        // consumed.  The processor will block again when we switch to the
        // thread w/remaining busy time- the busy() routine will see that busy
        // time is left and will add the processor back to the busy queue.
        add_thread_to_proc(t);
      }
      // Grab stuff from busy queue.
      Proc *p;
      while ( (p = thesystem.get_busy())) {
        p->clearBusyThread();
        add_proc(p);
      }
      // Update the current processor.
      _curproc = _procs.get();
      // If empty, mark as waiting and try to advance time.  We mark as waiting
      // so that the processor can be added to the queue again.
      if (_curproc && _curproc->empty()) {
        _curproc->setState(Proc::Waiting);
        _curproc = 0;
      }
    }
    return true;
  }

  // Switch to next running thread from current thread.
  void Cluster::yield()
  {
    lock();
    exec_ready();
  }

  // Explicitly switch to the scheduler thread.
  void Cluster::runscheduler()
  {
    Thread *old = _cur;
    _cur = &_main;
    exec_ready(&_main,old);
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
    t->proc()->get_ready(t);

    // Mark as done.
    t->destroy();

    unlock();
  }

  // Terminate the current thread, continue execution with next ready thread.
  void Cluster::terminate()
  {
    // Lock cluster to prevent preemption.
    lock();
    
    // Schedule any waiting threads.  We do this before a call to get_ready()
    // b/c it might be the case that a waiter is the next thread that should be
    // executed, e.g. if a waiting thread has a higher priority than any existing
    // ready threads.
    // For every waiting thread, add it back to the ready queue of its parent processor.
    // Then make sure that the processor is running.
    while (Thread *next = _cur->get_waiter()) {
      next->setHandle(_cur->handle());
      next->proc()->add_ready(next);
      thecluster.add_proc(next->proc());
    }

    Thread *ready = get_ready();
    Thread *old = _cur;
    _cur = ready;
    QT_ABORT(switch_term, old, 0, ready->thread());
  }

  // This terminates a thread, returning the stack to the
  // cluster for use by another thread.
  void *switch_term(qt_t *, void* old, void *)
  {
    Thread *oldthread = (Thread*)old;         // get current thread
    oldthread->destroy();                     // free stack
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
  inline void Cluster::exec_ready()
  {
    Thread *ready = get_ready();
    Thread *old = _cur;
    ready->setProc(_curproc);
    exec_ready(ready,old);    
  }

  // Switch from old thread to new thread; old thread is put into ready queue
  inline void Cluster::exec_ready(Thread *newthread,Thread *oldthread)
  {
    _cur = newthread;
    void *dummy=0;
    oldthread->setStackEnd();
    QT_BLOCK(switch_ready, 0, oldthread, newthread->thread());
  }

  // Switch old thread to new thread
  // old thread is to be placed in ready queue
  // caller is in charge of locking cluster!!!
  void *switch_ready(qt_t *sptr, void*, void *old)
  {
    Thread *oldthread = (Thread*)old;
    oldthread->save(sptr);                        // save stack pointer
    oldthread->proc()->add_ready(oldthread);      // place in ready queue
    thecluster.add_proc(oldthread->proc());       // make sure parent proc is running.
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
    newthread->setProc(_curproc);
    old->setStackEnd();
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

  void Cluster::print_procs(ostream &o) const
  {
    o << "Current proc:  " << _curproc << "\n"
      << "Proc queue:  " << _procs << endl;
  }

  void Cluster::print_procs() const  
  {
    print_procs(cerr);
  }

}
