//
// C++-only interface stuff.  This is used internally by the RTOS
// and implicitly included within Plasma programs.
//

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include "gc_cpp.h"
#include "gc_allocator.h"

#include <functional>
#include <vector>
#include <assert.h>

namespace plasma {

  class Thread;
  class Proc;

  typedef Thread * THandle;

  typedef std::vector<THandle,traceable_allocator<THandle> > TVect;

  typedef std::pair<int,int> HandleType;

  // Time is a 64-bit integer.  This may need to be changed depending
  // upon the compiler used.
  typedef unsigned long long int uint64;
  typedef long long int int64;
  typedef uint64 ptime_t;

  //
  // This defines parameters that are adjustable by the user.
  //
  struct ConfigParms
  {
    int      _stacksize;      // size of thread stack
    bool     _verbose;        // verbosity flag
    bool     _preempt;        // preemption allowed.
    unsigned _timeslice;      // Time slice length in usec.
    int      _numpriorities;  // Number of supported priorities.
    bool     _busyokay;       // Indicates that pBusy is legal- default is false.
    ptime_t  _simtimeslice;   // Size of time slice for low-priority threads in pBusy.

    ConfigParms();
  };

  // A Processor object encapsulates a Proc object and provides a means for
  // users to group threads together.  A processor may be given a name.  The
  // name string ptr is stored only- no copying is done, so it's legal to compare
  // pointers against the name for quick identification.  The name is not deleted-
  // use gc_strdup to copy a non-constant string.
  class Processor {
  public:
    Processor(const char *name = 0);
    Processor(Proc *p) : _proc(p) {};
    Proc *operator()() { return _proc; };
    const char *name() const;
    void setName(const char *);
    bool operator==(const Processor &p) const { return _proc == p._proc; };
    bool operator!=(const Processor &p) const { return _proc != p._proc; };

    friend struct HashProc;
  private:
    Proc *_proc;
  };

  struct HashProc {
    size_t operator()( Processor p ) const { return (size_t)p._proc; };
  };


  // Call this function to create a processor which shares its ready-queue
  // with another processor.  This can be used to model a cluster of processors
  // all able to get work from a single issue queue.
  Processor make_sharedproc(Processor p);
  Processor make_sharedproc(const char *name,Processor p);
  
  // Vector of processors.  The idea is that you use
  // this, rather than just a standard container, so that
  // you get N unique Processor objects from the constructor.
  // If share_queue is true, they will all share the same ready queue.
  struct Processors : public std::vector<Processor,traceable_allocator<Processor> >
  {
    Processors() {};
    Processors(unsigned n,const char *name = 0,bool shared = false);
  };

  // The following threads relate to the "current" processor, unless
  // a Cluster argument is supplied (assuming a function is used that
  // takes one as an argument).

  // The basic thread entry point is a one-argument function.
  typedef void (UserFunc)(void *);

  // Create a new thread and add it to the ready queue.
  // Returns a handle to the new thread.  If the priority is -1, it means
  // to use the current thread's priority.
  THandle pSpawn(UserFunc *f,void *args,int priority);
  THandle pSpawn(Proc *p,UserFunc *f,void *args,int priority);

  // Same as above, except that the data pointed to be args is copied to the
  // thread stack (nbytes worth).  The thread will receive a pointer to this
  // information.  If the priority is -1, it means to use the current thread's
  // priority.
  std::pair<THandle,void *> pSpawn(UserFunc *f,int nbytes,void *args,int priority);
  std::pair<THandle,void *> pSpawn(Proc *p,UserFunc *f,int nbytes,void *args,int priority);

  // Add a thread to the ready queue, but do not task switch.
  void pAddReady(THandle);

  // Add a waiting thread to the specified thread.
  void pAddWaiter(THandle,THandle waiter);

  // Clear a waiter- removes a thread from a thread's wait queue.
  void pClearWaiter(THandle,THandle waiter);

  // Return a handle to the current thread.
  THandle pCurThread();

  // Return a handle to the current processor.
  Processor pCurProc();

  // Returns true if a thread is finished.
  bool pDone(const THandle);

  // Wait on the specified thread.
  void pWait(THandle);

  // Switch to next ready thread.
  void pYield();

  // Put the current thread to sleep.  When it wakes, it is returned a handle
  // passed by pWake.
  HandleType pSleep();

  // Wake the specified thread, giving it the handle values.
  void pWake(std::pair<THandle,HandleType>);

  // Wake a busy thread that was made busy by pBusySleep.
  void pBusyWake(std::pair<THandle,HandleType>);

  // Return the thread's handle value (same as returned by pSleep()).
  HandleType pHandle(THandle);

  // Set a thread's handle.
  void pSetHandle(THandle,HandleType);

  // Set current thread's priority.  This has the effect of performing a thread
  // swap so that the priority takes effect immediately.
  // Note:  0 is highest priority.
  void pSetPriority(unsigned);

  // Return current thread's priority.
  unsigned pGetPriority();

  // Returns lowest (highest value) priority.
  unsigned pLowestPriority();

  // Kill the current thread.
  void pTerminate();

  // Kill a thread.  If not already done, will remove from the ready queue.
  void pTerminate(THandle);

  // Lock processor (prevent preemption).
  void pLock(void);

  // Unlock processor.
  void pUnlock(void);        

  // Returns lock status.
  bool pIsLocked();

  // Delays the current thread for the time specified.
  void pDelay(ptime_t);

  // Consumes the specified amount of time.  Note:  This is only
  // valid if the system has been configured to allow for time consumption.
  // Second parameter is a timeslice amount- if <= 0, the default timeslice
  // amount will be used.
  void pBusy(ptime_t,ptime_t ts = 0);

  // This makes a processor busy for an indefinite amount of time.
  // The processor and thread will only awake if pBusyWake() is called.
  // We do timeslice, however, if ts > 0.
  void pBusySleep(ptime_t ts = 0);

  // Return the current simulation time.
  ptime_t pTime();

  // Terminate program with return code .
  void pExit(int code);

  // Abort program gracefully with error message and return exit code -1.
  void pAbort(const char *);

  // Abort program immediately with error message and return exit code -1.
  void pPanic(const char *);

  //
  // A result class is returned by the spawn operator.  It acts like the
  // "futures" feature MultiLisp:  The spawn operator initiates a thread; to
  // get the thread's value, call the value() method.  If the thread is
  // not yet finished, the current thread will block.  You can also explicitly
  // wait on a thread (useful if you don't need the value at that point) or
  // kill a thread.  If you kill a thread before it's finished, the result is
  // equal to the value supplied by the default constructor of the return type.
  //
  // To use this with an alt block, use ResChan<R> instead.
  //
  template<typename R = int>
  class Result {  
  public:
    Result() : _result(0), _t(0) {};
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

  // The code below implements a common idiom of the spawn function: Collecting
  // up a bunch of values and then applying a predicate.  The most general case follows:
  // This collects a series of Result objects of a given type.  You then apply a predicate
  // by calling check().  If the predicate fails, check() returns false.  False is returned
  // immediately, meaning that an unchecked result may still have a running thread.

  template <typename R,typename Pred>
  struct ResultCheck {
    typedef Pred Predicate;
    typedef plasma::Result<R> ResType;
    
    ResultCheck(Pred p) : _pred(p) {};
    void push_back(ResType r) { _results.push_back(r); };
    void clear() { _results.clear(); };
    bool check();
  private:
    typedef std::vector<ResType,traceable_allocator<ResType> > ResStore;
    
    Predicate _pred;
    ResStore  _results;
  };

  // A common case is where the predicate tests that each Result object returns an expected
  // value, e.g. true.  If that is not true, e.g. a thread returns false, we expect check to
  // fail.  The following type generator produces such a type.  You use it like this:
  //
  // ValueCheckGen<Foo> results(make_valuecheck(foo));
  //
  // Where Foo is a type and foo is an instance of the type, e.g. bool and true.
  //
  // You then add threads like this:
  //
  // results.push_back(spawn(myfunc(1,2,3)));
  //
  // And check the results like this:
  //
  // if (!results.check()) { return false; };
  //
  // You can reuse it by calling clear:
  //
  // results.clear();

  template <typename T>
  struct ValueCheckGen {
    typedef ResultCheck<T,std::binder2nd<std::equal_to<T> > > type;
  };

  template <typename T>
  typename ValueCheckGen<T>::type make_valuecheck(T value) { return typename ValueCheckGen<T>::type(std::bind2nd(std::equal_to<T>(),value)); };

  // A common case for the above is to test a boolean, e.g. all threads return true.  The
  // code below implements this.  You use it like this:
  //
  // BoolCheck results(make_boolcheck(true));
  //
  // Every thread must return true, or else check will fail.

  typedef ValueCheckGen<bool>::type BoolCheck;

  inline BoolCheck make_boolcheck(bool value) { return make_valuecheck(value); };
  
  //////////////////////////////////////////////////////////////////////////////
  //
  // Implementation.
  //
  //////////////////////////////////////////////////////////////////////////////

  template <typename R>
  R Result<R>::value() const
  {
    assert(_t);
    wait();
    return *_result;
  }

  template <typename R>
  void Result<R>::wait() const
  {
    assert(_t);
    if (!pDone(_t)) {
      pWait(_t);
    }
  }

  template <typename R>
  void Result<R>::kill()
  {
    assert(_t);
    if (!pDone(_t)) {
      pTerminate(_t);
    }
  }

  template <class R,class Pred> bool ResultCheck<R,Pred>::check()
  {
    typename ResStore::iterator i = _results.begin();
    for (; i != _results.end(); ++i) {
      if ( !_pred(i->value()) ) {
        return false;
      }
    }  
    return true;
  }

}

#endif
