//
// This file contains a generic means for hashing values
// against processors, and a particular implementation which
// hashes energy values as doubles against processors.
//

#include "Energy.h"

namespace plasma {

  // Our energy storage object.
  ProcValue<energy_t> energy;

  // Specify how much energy is used.  This adds to the current processor's
  // energy count.
  energy_t pEnergy(energy_t e)
  {
    return energy.add(e);
  }

  // Get the energy count for this processor.  Clears the count.
  energy_t pGetEnergy(Processor p)
  {
    return energy.get(p);
  }

  // Same as above but does not clear the count.
  energy_t pReadEnergy(Processor p)
  {
    return energy[p];
  }

}
