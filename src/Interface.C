
//
// Miscellaneous user routines. 
//

#include <sstream>
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

  void pWake(pair<THandle,HandleType> p)
  {
    thecluster.wake(p.first,p.second);
  }

  void pBusyWake(pair<THandle,HandleType> p)
  {
    thecluster.busywake(p.first,p.second);
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

  void pBusy(ptime_t t,ptime_t ts)
  {
    thecluster.busy(t,ts);
  }

  void pBusySleep(ptime_t ts)
  {
    thecluster.busysleep(ts);
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

  const char *make_procsname(const char *name,int index)
  {
    if (name) {
      ostringstream ss;
      ss << name << "[" << index << "]";
      return gc_strdup(ss.str());
    } else {
      return 0;
    }
  }

  Processors::Processors(unsigned n,const char *name,bool shared)
  {
    Proc *p = 0;
    for (unsigned i = 0; i != n; ++i) {
      if (!i || !shared) {
        push_back(Processor(make_procsname(name,i)));
        p = back()();
      } else {
        push_back(make_sharedproc(make_procsname(name,i),p));
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
