//
// Main system class.  Only one of these ever exists.
//

#include "plasma-interface.h"
#include "System.h"

namespace plasma {

  System::System() :
    _size(0x1000), 
    _stacks(0), 
    _code(0),
    _wantshutdown(false)
  {
  }

  void System::init(const ConfigParms &cp)
  {
    _size = cp._stacksize;
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

}
