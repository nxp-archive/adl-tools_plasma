//
// This file contains a generic means for hashing values
// against processors, and a particular implementation which
// hashes energy values as doubles against processors.
//

# include <ext/hash_map>
#include "Interface.h"

#ifndef _ENERGY_H_
#define _ENERGY_H_

namespace plasma {

  class Proc;

  //
  // Processor/value storage class.
  //

  template <typename Data>
  struct ProcValue : public __gnu_cxx::hash_map<Processor,Data,HashProc> { 
    typedef __gnu_cxx::hash_map<Processor,Data,HashProc> Base;
    // This adds to the value for the specified processor.
    Data add(Processor p,Data d);
    // This adds to the current processor.
    Data add(Data d);
    // This gets the processor's value and clears the value.
    // Returns Data() if the item is not there.
    Data get(Processor p);
    // Reads the value, returns Data() if item is not there.
    Data read(Processor p) const;
  };
  
  //
  // Energy/Power API.
  //

  // Energy is a double.
  typedef double energy_t;

  // Specify how much energy is used.  This adds to the current processor's
  // energy count and returns the new value.
  energy_t pEnergy(energy_t);

  // Get the energy count for this processor.  Clears the count.
  energy_t pGetEnergy(Processor p);

  // Same as above but does not clear the count.
  energy_t pReadEnergy(Processor p);

  //
  // Implementation.
  //

  template <typename Data>
  Data ProcValue<Data>::add(Processor p,Data d)
  {
    return (this->operator[](p) += d);
  }

  template <typename Data>
  Data ProcValue<Data>::add(Data d)
  {
    return add(pCurProc(),d);
  }

  template <typename Data>
  Data ProcValue<Data>::get(Processor p)
  {
    typename Base::iterator i = Base::find(p);
    if (i != Base::end()) {
      Data tmp = i->second;
      i->second = Data();
      return tmp;
    } else {
      return Data();
    }
  }

  template <typename Data>
  Data ProcValue<Data>::read(Processor p) const
  {
    typename Base::const_iterator i = Base::find(p);
    if (i != Base::end()) {
      return i->second;
    } else {
      return Data();
    }
  }

}

#endif
