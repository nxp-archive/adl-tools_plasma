//
// Main system class.  Only one of these ever exists.
//

#include "Interface.h"
#include "System.h"
#include "Thread.h"
#include "Proc.h"

namespace plasma {

  template<class T1,class T2>
  bool greater_time<T1,T2>::operator()(const T1 *x,const T2 *y)
  {
    return x->endtime() > y->endtime();
  }

  System::System() :
    _size(0x1000), 
    _stacks(0), 
    _code(0),
    _wantshutdown(false),
    _busyokay(false),
    _time(0)
  {
  }

  void System::init(const ConfigParms &cp)
  {
    _size = cp._stacksize;
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
    void *st = _stacks;
    if (st) {
      _stacks = *(void**)st;
    } else {
      //    st = new char[_size];
      st = new char[_size];
    }
    return(st);
  }

  // Dispose stack
  // caller is in charge of locking the processor!!!
  void System::dispose(void *st)
  {
    *(void**)st = _stacks;
    _stacks = st;
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

}
