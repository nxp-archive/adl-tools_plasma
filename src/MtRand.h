//
// Do not instantiate directly- use the Random class and specify
// this as the template parameter.
//
// This is a class which implements the Mersenne Twister 
// random number generator.  This is a really powerful
// linear pseudo-random number generator with a period of 2^19937-1.
// It is theoretically reversible, but the code that I found for
// the reverse function has a bug in it, so the reversability
// is not implemented.  Its state consists of 624 words, making it
// fairly expensive to save, but it gives you a great distribution.
//
// $Id$
//

#ifndef MTRANDOM_H
#define MTRANDOM_H

#include <iosfwd>

namespace plasma {

  // Mersenne Twister random number generator class.
  class MtRand {
  public:
    MtRand ();
    // Specify a seed.
    void set_seed (unsigned seed);
    // Generate the next value.
    unsigned genrand ();
    // Generate the previous value.
    unsigned rgenrand ();
    // Save/load state from a binary stream.
    void save(std::ostream &) const;
    void load(std::istream &);
  private:
    void reset();

    enum { N=624 };
    unsigned _mt[N]; /* the array for the state vector  */
    int _mti;
  };

}

#endif
