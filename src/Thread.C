//
// Basic thread primitive.
//

#include "Machine.h"
#include "Thread.h"
#include "Cluster.h"
#include "System.h"

namespace plasma {

  /* `alignment' must be a power of 2. */
#define STP_STKALIGN(sp, alignment) \
  ((void *)((((qt_word_t)(sp)) + (alignment) - 1) & ~((alignment)-1)))

  static void shell(void *u, void *t, UserFunc *f)
  {
    // Execute user function with argument.
    (*(UserFunc *)f)(u);
    // We're done, so go on to next thread.
    thecluster.terminate();
  }

  // Allocates a stack and adds this item to
  // the system's list of running threads.
  void Thread::realize(UserFunc *f,void *arg)
  {
    _stack = thesystem.newstack();
    void *sto = STP_STKALIGN (_stack, QT_STKALIGN);
    _thread = QT_SP(sto,thesystem.stacksize()-QT_STKALIGN);
    _thread = QT_ARGS(_thread,arg,this,(qt_userf_t*)f,shell);
    System::add_active_thread(this);
  }

  // Destroy real thread i.e. deallocate its stack
  void Thread::destroy()
  {
    thesystem.dispose(_stack);
    _stack = 0;
    _done = true;
    System::remove_active_thread(this);
  }

}
