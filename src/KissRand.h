//
// Do not instantiate directly- use the Random class and specify
// this as the template parameter.
//
// Implements the KISS random number generator.  This is a simple,
// but quite good, random number generator with a period of 2^127.
// The function is not reversible (that I know of).  It requires 
// five words to store state.
//
// $Id$
//

#ifndef _KISSRAND_H_
#define _KISSRAND_H_

namespace plasma {

  // KISS random number generator.
  class KissRand {
  public:
    KissRand ();
    // Specify a seed.
    void set_seed (unsigned seed);
    // Generate the next value.
    unsigned genrand ();
    // This is not implemented- it's stubbed out here for compatibility with
    // Random.
    unsigned rgenrand ();
    // Save/load state from a binary stream.
    void save(std::ostream &) const;
    void load(std::istream &);
  private:
    // These are the state variables for the function.
    // Note:  Using signed arithmetic increased the
    // quality of the random numbers.
    int _x, _y, _z, _w, _carry;
  };

  // Generate the next value.
  inline unsigned KissRand::genrand ()
  {
    int k, m;
    _x = _x * 69069 + 1;
    _y ^= _y << 13;
    _y ^= _y >> 17;
    _y ^= _y << 5;
    k = (_z >> 2) + (_w >> 3) + (_carry >> 2);
    m = _w + _w + _z + _carry;
    _z = _w;
    _w = m;
    _carry = k >> 30;
    return _x + _y + _w;
  }

}
#endif
