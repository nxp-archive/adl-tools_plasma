
//
// Miscellaneous user routines. 
//

#include <stdarg.h>
#include <string>

#include "Interface.h"
#include "Proc.h"
#include "Cluster.h"
#include "System.h"

using namespace std;

namespace plasma {

  ConfigParms::ConfigParms() :
    _stacksize(StackMin),
    _verbose(false),
    _preempt(true),
    _timeslice(50000),
    _numpriorities(32),
    _busyokay(false),
    _simtimeslice(10)
  {}

  inline unsigned convert_priority(unsigned priority)
  {
    if (priority >= Proc::numPriorities()) {
      pAbort("Bad priority value");
    }
    int p = priority;
    int s = Proc::numPriorities();
    return (abs((p - (s - 1)) % s));
  }

  // Create a new thread and make it ready.
  Thread *pSpawn(UserFunc *f,void *a,int pr)
  {
    return thecluster.curProc()->create(f,a,(pr < 0) ? pr : convert_priority(pr));
  }

  Thread *pSpawn(Proc *p,UserFunc *f,void *a,int pr)
  {
    thecluster.add_proc(p);
    return p->create(f,a,(pr < 0) ? pr : convert_priority(pr));
  }

  // Create a new thread and make it ready.
  pair<Thread *,void *> pSpawn(UserFunc *f,int nbytes,void *a,int pr)
  {
    return thecluster.curProc()->create(f,nbytes,a,(pr < 0) ? pr : convert_priority(pr));
  }

  pair<Thread *,void *> pSpawn(Proc *p,UserFunc *f,int nbytes,void *a,int pr)
  {
    thecluster.add_proc(p);
    return p->create(f,nbytes,a,(pr < 0) ? pr : convert_priority(pr));
  }

  void pAddReady(Thread *t)
  {
    t->proc()->add_ready(t);
    thecluster.add_proc(t->proc());
  }

  void pAddWaiter(THandle t,THandle waiter)
  {
    t->add_waiter(waiter);
  }

  void pClearWaiter(THandle t,THandle waiter)
  {
    t->get_waiter(waiter);
  }

  Thread *pCurThread()
  {
    return thecluster.curThread();
  }

  Processor pCurProc()
  {
    return Processor(thecluster.curProc());
  }

  void pWait(Thread *t)
  {
    thecluster.wait(t);
  }

  bool pDone(const THandle t)
  {
    return t->done();
  }

  void pYield()
  {
    thecluster.yield();
  }

  void pTerminate()
  {
    thecluster.terminate();
  }

  void pTerminate(THandle t)
  {
    thecluster.terminate(t);
  }

  HandleType pSleep()
  {
    return thecluster.sleep();
  }

  void pWake(THandle t,HandleType h)
  {
    thecluster.wake(t,h);
  }

  HandleType pHandle(THandle t)
  {
    return t->handle();
  }

  void pSetHandle(THandle t,HandleType h)
  {
    t->setHandle(h);
  }

  void pSetPriority(unsigned p)
  {
    thecluster.set_priority(convert_priority(p));
  }

  unsigned pGetPriority()
  {
    return convert_priority(thecluster.get_priority());
  }

  unsigned pLowestPriority()
  {
    return convert_priority(thecluster.lowest_priority());
  }

  void pDelay(ptime_t t)
  {
    thecluster.delay(t);
  }

  void pBusy(ptime_t t)
  {
    thecluster.busy(t);
  }

  ptime_t pTime()
  {
    return thesystem.time();
  }

  // Lock thecluster.
  void pLock(void)
  {
    thecluster.lock();
  }

  // Unlock thecluster.
  void pUnlock(void)
  {
    thecluster.unlock();
  }

  bool pIsLocked()
  {
    return thecluster.locked();
  }

  /////////////// Timeout ///////////////

  void timeout(void *a)
  {
    Timeout *to = (Timeout *)a;
    pDelay(to->delay());
    pWake(to->reset(),to->_h);
  }

  THandle Timeout::clear_notify()
  {
    if (_writet) {
      pTerminate(_writet);
      _writet = 0;
    }
    return reset();
  }

  THandle Timeout::reset() 
  { 
    THandle t = _readt; 
    _readt = 0; 
    return t; 
  };

  void Timeout::set_notify(plasma::THandle t,plasma::HandleType h)
  {
    _readt = t;
    _h = h;
    assert(!_writet);
    _writet = pSpawn(timeout,this,-1);
  }

  /////////////// ClockChan ///////////////

  // Returns true if we're on a clock edge, given a clock period.
  bool ClockChanImpl::is_phi() const
  {
    return (pTime() % _period) == 0;
  }

  // Returns the time of the next clock cycle.
  ptime_t ClockChanImpl::next_phi() const
  {
    return (pTime() / _period + 1)*_period;
  }

  THandle ClockChanImpl::reset() 
  { 
    THandle t = _readt; 
    _readt = 0; 
    _waket = 0;
    return t; 
  }

  void ClockChanImpl::set_notify(THandle t,HandleType h) 
  { 
    assert(!_readt); 
    _readt = t; 
    _h = h; 
    // If we have data, start the waker.  We don't need to check whether
    // the data is current b/c we wouldn't be here if it weren't.
    if (!empty()) {
      start_waker();
    }
  }

  THandle ClockChanImpl::clear_notify()
  {
    if (_waket) {
      pTerminate(_waket);
      _waket = 0;
    }
    return reset();
  }

  void delayed_waker(void *a)
  {
    ClockChanImpl *cc = (ClockChanImpl *)a;
    pDelay(cc->_delay);
    pWake(cc->reset(),cc->_h);
  }

  // Start a wake-up thread only if one doesn't already exist.
  void ClockChanImpl::start_waker()
  {
    if (!_waket) {
      _delay = next_phi() - pTime();
      assert(!_waket);
      _waket = pSpawn(delayed_waker,this,-1);
    }
  }

  void ClockChanImpl::delayed_wakeup(bool current_data)
  {
    if (is_phi() && current_data) {
      // We're on a clock edge- wake up thread.
      pWake(reset(),_h);
    } else {
      // Not on a clock edge- schedule a thread to
      // wake up the reader at the correct time.
      start_waker();
    }
  }

  void ClockChanImpl::delayed_reader_wakeup(bool have_data)
  {
    if (have_data) {
      // We're not empty, so we're not ready b/c
      // we're not on a clock edge.  Launch a
      // wakeup thread if we don't have on already.
      start_waker();
    }
    // We're empty- sleep until we get data.
    set_notify(pCurThread(),HandleType());
    pSleep();
  }

  //
  // Mutex I/O routines.
  //

  int mprintf(const char *format, ... )
  {
    pLock();
    va_list ap;
    va_start (ap,format);
    int c = vprintf(format,ap);
    pUnlock();
    return c;
  }

  int mfprintf(FILE *f,const char *format, ...)
  {
    pLock();
    va_list ap;
    va_start (ap,format);
    int c = vfprintf(f,format,ap);
    pUnlock();
    return c;
  }

  int mvprintf(const char *format, va_list ap)
  {
    pLock();
    int c = vprintf(format,ap);
    pUnlock();
    return c;
  }

  int mvfprintf(FILE *f,const char *format,va_list ap)
  {
    pLock();
    int c = vfprintf(f,format,ap);
    pUnlock();
    return c;
  }

  //
  // GC string methods.
  //

  char *gc_strdup(const char *s)
  {
    char *ns = new (GC) char[strlen(s)+1];
    return strcpy(ns,s);
  }

  char *gc_strdup(const std::string &s)
  {
    char *ns = new (GC) char[s.size()];
    return strcpy(ns,s.c_str());
  }

  //
  // Processor/Processors methods.
  //

  Processor::Processor(const char *name) :
    _proc(new Proc(name))
  {
  }

  const char *Processor::name() const
  {
    return _proc->name();
  }

  void Processor::setName(const char *name)
  {
    _proc->setName(name);
  }

  Processors::Processors(unsigned n,const char *name,bool shared)
  {
    Proc *p = 0;
    for (unsigned i = 0; i != n; ++i) {
      if (!i || !shared) {
        push_back(Processor(name));
        p = back()();
      } else {
        push_back(make_sharedproc(p));
      }
    }
  }

 Processor make_sharedproc(Processor p)
  {
    Proc *np = new Proc(p());
    thecluster.add_proc(np);
    return Processor(np);
  }

  Processor make_sharedproc(const char *name,Processor p)
  {
    Proc *np = new Proc(p(),name);
    thecluster.add_proc(np);
    return Processor(np);
  }

}
