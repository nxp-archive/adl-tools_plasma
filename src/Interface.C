
//
// Miscellaneous user routines. 
//

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
    thecluster.curProc()->add_ready(t);
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

  Processor::Processor() :
    _proc(new Proc)
  {
  }

}
