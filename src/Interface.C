
//
// Miscellaneous user routines. 
//

#include "plasma-interface.h"
#include "Processor.h"
#include "System.h"

using namespace std;

namespace plasma {

  ConfigParms::ConfigParms() :
    _stacksize(8192),
    _verbose(false),
    _preempt(true),
    _timeslice(50000)
  {}

  // Create a new thread and make it ready.
  Thread *pSpawn(UserFunc *f,void *a)
  {
    return processor.create(f,a);
  }

  // Create a new thread and make it ready.
  pair<Thread *,void *> pSpawn(UserFunc *f,int nbytes,void *a)
  {
    return processor.create(f,nbytes,a);
  }

  void pAddReady(Thread *t)
  {
    processor.add_ready(t);
  }

  Thread *pCurThread()
  {
    return processor.getCur();
  }

  void pWait(Thread *t)
  {
    processor.wait(t);
  }

  bool pDone(const THandle t)
  {
    return t->done();
  }

  void pYield()
  {
    processor.yield();
  }

  void pTerminate()
  {
    processor.terminate();
  }

  int pSleep()
  {
    return processor.sleep();
  }

  void pWake(Thread *t,int h)
  {
    processor.wake(t,h);
  }

  // Lock processor.
  void pLock(void)
  {
    processor.lock();
  }

  // Unlock processor.
  void pUnlock(void)
  {
    processor.unlock();
  }

}
