//
// Implements the KISS random number generator.  This is a simple,
// but quite good, random number generator with a period of 2^127.
// The function is not reversible (that I know of).  It requires 
// five words to store state.
//
// $Id$
//

#include <stdexcept>
#include <iostream>

#include "KissRand.h"

using namespace std;

namespace plasma {

  void writeInt(std::ostream &,unsigned);
  unsigned readInt(std::istream &);

  KissRand::KissRand()
  {
    set_seed(4937);
  }

  // Specify a seed and init the generator.
  void KissRand::set_seed (unsigned seed)
  {
    _x = seed | 1;
    _y = seed | 2;
    _z = seed | 4;
    _w = seed | 8;
    _carry = 0;
  }

  unsigned KissRand::rgenrand()
  {
    throw runtime_error("Reverse generation not supported by KissRand.");
  }

  void KissRand::save(ostream &os) const
  {
    writeInt(os,_x);
    writeInt(os,_y);
    writeInt(os,_z);
    writeInt(os,_w);
    writeInt(os,_carry);
  }

  void KissRand::load(istream &is) 
  {
    _x = readInt(is);
    _y = readInt(is);
    _z = readInt(is);
    _w = readInt(is);
    _carry = readInt(is);
  }

}
