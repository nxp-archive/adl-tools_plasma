//
// This is the main user interface to the threads package.
//

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

class Thread;

//
// This defines parameters that are adjustable by the user.
//
struct ConfigParms
{
  int      _stacksize; // size of thread stack
  bool     _verbose;   // verbosity flag
  bool     _preempt;   // preemption allowed.
  unsigned _timeslice; // Time slice length in usec.

  ConfigParms();
};

// The basic thread entry point is a one-argument function.
typedef void (UserFunc)(void *);

// Create a new thread and add it to the ready queue.
// Returns a handle to the new thread.
Thread *pSpawn(UserFunc *f,void *args);

// Wait on the specified thread.
void pWait(Thread *);

// Switch to next ready thread.
void pYield();

// Kill the current thread.
void pTerminate();

// Lock processor (prevent preemption).
void pLock(void);

// Unlock processor.
void pUnlock(void);        

// Terminate program with return code .
void pExit(int code);

// Abort program gracefully with error message and return exit code -1.
void pAbort(char *);

// Abort program immediately with error message and return exit code -1.
void pPanic(char *);

#endif

