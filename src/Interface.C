
//
// Miscellaneous user routines. 
//

#include "Interface.h"
#include "Processor.h"
#include "System.h"

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
Thread *pSpawn(UserFunc *f,int nbytes,void *a)
{
  return processor.create(f,nbytes,a);
}

void pWait(Thread *t)
{
  processor.wait(t);
}

void pYield()
{
  processor.yield();
}

void pTerminate()
{
  processor.terminate();
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
