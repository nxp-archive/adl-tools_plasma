//
// C++-only interface stuff.  This is used internally by the RTOS
// and implicitly included within Plasma programs.
//

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include <vector>

namespace plasma {

  class Thread;

  typedef std::vector<Thread *> TVect;

  typedef Thread * THandle;

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
  THandle pSpawn(UserFunc *f,void *args);

  // Same as above, except that the data pointed to be args is copied to the
  // thread stack (nbytes worth).  The thread will receive a pointer to this
  // information.
  std::pair<THandle,void *> pSpawn(UserFunc *f,int nbytes,void *args);

  // Add a thread to the ready queue, but do not task switch.
  void pAddReady(THandle);

  // Add a waiting thread to the specified thread.
  void pAddWaiter(THandle,THandle waiter);

  // Clear a waiter- removes a thread from a thread's wait queue.
  void pClearWaiter(THandle,THandle waiter);

  // Return a handle to the current thread.
  THandle pCurThread();

  // Returns true if a thread is finished.
  bool pDone(const THandle);

  // Wait on the specified thread.
  void pWait(THandle);

  // Switch to next ready thread.
  void pYield();

  // Put the current thread to sleep.  When it wakes, it is returned a handle
  // passed by pWake.
  int pSleep();

  // Wake the specified thread, giving it the handle value.  This switches
  // to this thread.
  void pWake(THandle,int);

  // Return the thread's handle value (same as returned by pSleep()).
  int pHandle(THandle);

  // Set a thread's handle.
  void pSetHandle(THandle,int);

  // Kill the current thread.
  void pTerminate();

  // Kill a thread.  If not already done, will remove from the ready queue.
  void pTerminate(THandle);

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

  // A result class is returned by the spawn operator.  It acts like the
  // "futures" feature MultiLisp:  The spawn operator initiates a thread; to
  // get the thread's value, call the value() method.  If the thread is
  // not yet finished, the current thread will block.  You can also explicitly
  // wait on a thread (useful if you don't need the value at that point) or
  // kill a thread.  If you kill a thread before it's finished, the result is
  // equal to the value supplied by the default constructor of the return type.
  //
  // To use this with an alt block, use ResChan<R> instead.
  template<class R>
  class Result {  
  public:
    Result(std::pair<THandle,void *> a) : _result((R*)a.second),_t(a.first) {}; 

    // Get the thread's result.
    R value() const;
    // Is the thread done?
    bool done() const { return pDone(_t); };
    // Wait on the thread.
    void wait() const;
    // Kill the thread.
    void kill();

  protected:
    THandle thread() { return _t; }

  private:
    R       *_result; // Pointer to result object.
    THandle  _t;      // Thread returning a result.
  };

  //////////////////////////////////////////////////////////////////////////////
  //
  // Implementation.
  //
  //////////////////////////////////////////////////////////////////////////////

  template <typename R>
  R Result<R>::value() const
  {
    wait();
    return *_result;
  }

  template <typename R>
  void Result<R>::wait() const
  {
    if (!pDone(_t)) {
      pWait(_t);
    }
  }

  template <typename R>
  void Result<R>::kill()
  {
    if (!pDone(_t)) {
      pTerminate(_t);
    }
  }

}

#endif
