//
// The processor class defines a currently running thread and
// contains the logic to preempt it and schedule a new one.
//

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>

#include "Machine.h"
#include "Processor.h"
#include "System.h"

using namespace std;

sigset_t Processor::_empty_mask;  // empty signal mask
sigset_t Processor::_alarm_mask;  // mask with SIGVTALRM
unsigned Processor::_timeslice;

// This is the virtual alarm signal associated with ITIMER_VIRTUAL.
// It counts CPU time, rather than real time.
const int AlarmSignal = SIGVTALRM;

static void preempt(int);        // called on preemption of thread

// Switch off preemption
void Processor::nopreempt(void)
{
  sigprocmask(SIG_BLOCK, &_alarm_mask, 0);
}

// Switch on preemption
void Processor::preempt(void)
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
  static struct itimerval value = { {0,Processor::_timeslice}, {0,Processor::_timeslice} };
  setitimer(ITIMER_VIRTUAL,&value,0);
}

// Preempt current thread and reschedule AlarmSignal
// AlarmSignal is blocked during execution of the handler!!!
// There is always a thread on the ready queue, since there's always
// at least the main thread.
// The processor will be unlocked by the call to yield(), unless we're 
// switching to the scheduling thread.
static void preempt(int) 
{
  resetalarm();                          // reset alarm timer
  if (processor.locked()) {              // processor in kernel mode
    return;
  }
  processor.lock();                      // prevent further preemption
  processor.preempt();                   // return of sig handler
  processor.yield();                     // switch to new ready thread
  processor.nopreempt();                 // in sig handler again
}

// The thread for this processor will only be init'd
// when we swap to *another* thread.  Typically, at startup,
// that first thread is the special main routine written by
// the user.
Processor::Processor() :
  _main(),
  _cur(0),
  _kernel(true)
{
  static bool dummy = init();

  setalarm();                              // set alarm timer
}

// Static initialization.
bool Processor::init()
{
  sigemptyset(&_empty_mask);                // empty signal mask
  sigemptyset(&_alarm_mask);                // mask with AlarmSignal only
  sigaddset(&_alarm_mask, AlarmSignal);
  return true;
}

void Processor::init(const ConfigParms &cp)
{
  if (!cp._preempt) {
    nopreempt();
  }
  _timeslice = cp._timeslice;
  resetalarm();
}

void print_ready()
{
  cout << "Ready queue:  ";
  thesystem.print_ready(cout);
  cout << endl;
}

// Poll for ready thread and bring it to execution.
// For now, if there are no ready threads or if the system
// shutdown flag has been set, we return.  Since the main
// polling routine is itself a thread, there's always something
// on the queue, unless we get to here (the scheduler itself).
// If there's nothing on the queue, it means that this is the only
// thread, so there are no more user threads and we should exit.
void Processor::scheduler()
{
  // Unlock the processor to begin.
  unlock();
  // This loops until there are no more items in the queue, then it exits.
  while (1) {
    // Has the system requested a shutdown?
    if (thesystem.wantShutdown()) {
      return;
    }
    // Try to get a ready thread.
    //    print_ready();
    if (Thread *ready = thesystem.get_ready()) {
      // Switch to thread from main thread.  We always save into
      // _main, so we can clobber current.
      exec_ready(ready,&_main);
    } else {       
      // No thread available:  Exit.
      return;
    }
  }
}

// Create a thread and add to ready queue.
Thread *Processor::create(UserFunc *f,void  *arg)
{
  lock();
  Thread *t = new Thread;
  t->realize(f,arg);
  thesystem.add_ready(t);
  unlock();
  return t;
}

// Causes the current thread to wait on the specified thread.
void Processor::wait(Thread *t)
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

void Processor::add_ready(Thread *t)
{
  thesystem.add_ready(t);
}

// Switch to next running thread from current thread.
void Processor::yield()
{
  Thread *ready = thesystem.get_ready();
  Thread *old = _cur;
  exec_ready(ready,old);
}

// Terminate the current thread, continue execution with next ready thread.
void Processor::terminate()
{
  // Lock processor to prevent preemption.
  lock();
  Thread *ready = thesystem.get_ready();
  Thread *old = _cur;
  _cur = ready;
  QT_ABORT(switch_term, old, 0, ready->thread());
}

// Explicitly switch to the scheduler thread.
void Processor::runscheduler()
{
  Thread *old = _cur;
  _cur = &_main;
  exec_ready(&_main,old);
}

// This terminates a thread, returning the stack to the
// processor for use by another thread.
void *switch_term(qt_t *, void* old, void *)
{
  Thread *oldthread = (Thread*)old;         // get current thread
  oldthread->destroy();                     // free stack
  // For every waiting thread, add it back to the ready queue.
  while (Thread *next = oldthread->get_waiter()) {
    thesystem.add_ready(next);
  }
  if (!processor.in_scheduler()) {
    // If we're not in (switching to) the scheduler, make sure that preemption
    // is on.  Otherwise, the scheduler will run and unlock the processor once
    // it chooses a new thread.
    processor.unlock();
  }
  return(0);                                // satisfy type checker
}

// Returns true if the current thread is the scheduling (main) thread.
inline bool Processor::in_scheduler() const
{
  return (_cur == &_main);
}

// Switch from old thread to new thread; old thread is put into ready queue
inline void Processor::exec_ready(Thread *newthread,Thread *oldthread)
{
  _cur = newthread;
  QT_BLOCK(switch_ready, 0, oldthread, newthread->thread());
}

// Switch old thread to new thread
// old thread is to be placed in ready queue
// caller is in charge of locking processor!!!
void *switch_ready(qt_t *sptr, void*, void *old)
{
  Thread *oldthread = (Thread*)old;
  oldthread->save(sptr);                        // save stack pointer
  thesystem.add_ready(oldthread);               // place in ready queue
  if (!processor.in_scheduler()) {
    // If we're not in (switching to) the scheduler, make sure that preemption
    // is on.  Otherwise, the scheduler will run and unlock the processor once
    // it chooses a new thread.
    processor.unlock();
  }
  return(0);                                    // satisfy type checker
}

// Switch to a new ready thread.  The old thread is *not* placed back onto the
// ready queue.
inline void Processor::exec_block()
{
  Thread *newthread = thesystem.get_ready();
  Thread *old = _cur;
  _cur = newthread;
  QT_BLOCK(switch_block, 0, old, newthread->thread());
}

void *switch_block(qt_t *sptr, void*, void *old)
{
  Thread *oldthread = (Thread*)old;
  oldthread->save(sptr);                        // save stack pointer
  if (!processor.in_scheduler()) {
    // If we're not in (switching to) the scheduler, make sure that preemption
    // is on.  Otherwise, the scheduler will run and unlock the processor once
    // it chooses a new thread.
    processor.unlock();
  }
  return(0);                                    // satisfy type checker
}
