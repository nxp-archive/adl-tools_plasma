//
// This is the main user interface to the threads package.
// It is implicitly included so that a user program does not
// need to have an include statement for it.
//

#ifndef _PLSMAA_INTERFACE_H_
#define _PLSMAA_INTERFACE_H_

namespace plasma {

  // We want to process any calls to Processor::spawn.
  // Note:  This *must* be in the plasma namespace or it
  // will not work b/c Processor is declared in plasma.
  metaclass Plasma Processor;

}

// Dummy class to implement spawn operator.  This is
// outside of the plasma namespace so that it affects
// everything and thus looks like an operator.
pSpawner class Spawn {};
Spawn spawn;

#include "Interface.h"

#endif

