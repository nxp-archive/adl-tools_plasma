//
// Main system class.  Only one of these ever exists.
//

#include <sstream>
#include <stdexcept>

#include "gc_cpp.h"
#include "Interface.h"
#include "System.h"
#include "Thread.h"
#include "Cluster.h"
#include "Proc.h"

typedef char * ptr_t;
extern "C" void (*GC_push_other_roots) GC_PROTO((void));
extern "C" void GC_push_all_stack GC_PROTO((ptr_t b, ptr_t t));

using namespace std;

namespace plasma {

  template<class T1,class T2>
  bool greater_time<T1,T2>::operator()(const T1 *x,const T2 *y)
  {
    return x->endtime() > y->endtime();
  }

  extern "C" void GC_stop_world()
  {
    //printf ("Stopping the world.\n");
    Cluster::nopreempt();
  }

  extern "C" void GC_start_world()
  {
    //printf ("Starting the world.\n");
    Cluster::preempt();
  }

  // Do nothing b/c no kernel threads in this implementation.
  extern "C" void GC_lock()
  {
  }

  Thread *System::_active_list = 0;

  System::System() :
    _size(0x1000), 
    _code(0),
    _wantshutdown(false),
    _busyokay(false),
    _time(0)
  {
    GC_push_other_roots = System::push_other_roots;    
  }

  // The garbage collector needs to know about all of the "roots" in the
  // program.  These are the static data sets (one for the main program + one
  // per dynamic library), and the stack.  Since we're multithreaded, each
  // thread has a stack.  This function takes care of recording all of the
  // stacks for the threads we have.  Each active thread is stored in
  // _active_list.  We iterate over the list and push each stack.  The GC then
  // uses this information to search for allocated objects.
  void System::push_other_roots(void)
  {
    //    printf ("push_other_roots called:  %d threads.\n",System::num_active_threads());
    Thread *n = _active_list;
    while (n) {
      if (n->stack()) {
        //printf ("  %p:  %p:%p.\n",n,(ptr_t)n->stack(),(ptr_t)n->stackend()+1);
        GC_push_all_stack((ptr_t)n->stack(),(ptr_t)n->stackend()+1);
      }
      n = n->nt();
    }
  }

  void System::init(const ConfigParms &cp)
  {
    _size = cp._stacksize;
    if (_size < StackMin) {
      ostringstream ss;
      ss << "The stack must be a minimumx of " << hex << StackMin << " bytes.";
      throw runtime_error(ss.str());
    }
    _busyokay = cp._busyokay;
  }

  bool System::busyokay() const
  {
    return _busyokay;
  }

  // Trigger program shutdown
  void System::shutdown(int c)
  {
    _wantshutdown = true;
    _code = c;                 // program return code
  }

  // Allocate new stack
  void *System::newstack()
  {
    return GC_MALLOC(_size);
  }

  // Dispose stack
  // caller is in charge of locking the processor!!!
  void System::dispose(void *st)
  {
    GC_FREE(st);
  }

  void System::add_busy(ptime_t t,Thread *th)
  {
    th->setTime(t);
    th->setStartTime(_time);
    Proc *p = th->proc();
    p->setState(Proc::Busy);
    _busy.push(p);
  }

  void System::add_delay(ptime_t t,Thread *th)
  {
    th->setTime(t);
    th->setStartTime(_time);
    _delay.push(th);
  }

  template <class C,class T>
  T *System::get_current(C &q)
  {
    if (!q.empty()) {
      T *t = q.top();
      if (t->endtime() <= _time) {
        q.pop();
        return t;
      }
    }
    return 0;
  }

  Proc *System::get_busy()
  {
    return get_current<PPriQueue,Proc>(_busy);
  }

  Thread *System::get_delay()
  {
    return get_current<TPriQueue,Thread>(_delay);
  }

  bool System::update_time()
  {
    if (_busy.empty() && _delay.empty()) {
      return false;
    } else if (_busy.empty()) {
      _time = _delay.top()->endtime();
    } else if (_delay.empty()) {
      _time = _busy.top()->endtime();
    } else {
      if (_busy.top()->endtime() < _delay.top()->endtime() ) {
        _time = _busy.top()->endtime();
      } else {
        _time = _delay.top()->endtime();
      }
    }
    return true;
  }

  // Add element x to head of list.  Returns new list pointer.
  void System::add_active_thread(Thread *x)
  {
    x->setnt(_active_list);
    x->setpt(0);
    if (_active_list) {
      _active_list->setpt(x);
    }
    _active_list = x;
  }

  // Remove element x from list.
  void System::remove_active_thread(Thread *x)
  {
    Thread *t = x->pt();
    if (t) {
      // x was pointed to, so therefore is not head of list.
      t->setnt(x->nt());
    } else {
      // x must be head of list.
      _active_list = x->nt();
      if (_active_list) {
        _active_list->setpt(0);
      }
    }
    // For safety.
    x->setnt(0);
    x->setpt(0);
  }  

  // Number of active threads- linear time.
  unsigned System::num_active_threads()
  {
    Thread *n = _active_list;
    unsigned c = 0;
    while (n) {
      ++c;
      n = n->nt();
    }
    return c;
  }

}
