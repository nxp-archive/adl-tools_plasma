//
// Main system class.  Only one of these ever exists.
//

#include "Interface.h"
#include "System.h"
#include "Thread.h"
#include "Proc.h"

namespace plasma {

  bool greater_time::operator()(const Thread *x,const Thread *y)
  {
    return x->time() > y->time();
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
    th->setTime(_time+t);
    Proc *p = th->proc();
    p->setState(Proc::Busy);
    p->setBusyThread(th);
    _busy.push(th);
  }

  void System::add_delay(ptime_t t,Thread *th)
  {
    th->setTime(_time+t);
    _delay.push(th);
  }

  Thread *System::get_current(PriQueue &q)
  {
    if (!q.empty()) {
      Thread *t = q.top();
      if (t->time() <= _time) {
        q.pop();
        return t;
      }
    }
    return 0;
  }

  Thread *System::get_busy()
  {
    return get_current(_busy);
  }

  Thread *System::get_delay()
  {
    return get_current(_delay);
  }

  bool System::update_time()
  {
    if (_busy.empty() && _delay.empty()) {
      return false;
    } else if (_busy.empty()) {
      _time = _delay.top()->time();
    } else if (_delay.empty()) {
      _time = _busy.top()->time();
    } else {
      greater_time gt;
      if (gt(_delay.top(),_busy.top())) {
        _time = _busy.top()->time();
      } else {
        _time = _delay.top()->time();
      }
    }
    return true;
  }

}
