//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
//
// Queue implementation for Processor objects.
//

#include "ProcQ.h"
#include "Proc.h"

#define PROC(x) (reinterpret_cast<Proc*>(x))

namespace plasma {

  Proc *ProcQ::get_non_empty()
  {
    QBase *iter = front();
    while (iter) {
      Proc *proc = PROC(iter);
      if (!proc->empty()) {
        remove(proc);
        return proc;
      } else {
        iter = proc->getnext();
      }
    }
    return 0;
  }

}
