//
// Random number generation.  There are N independent streams, where N
// is set by the user.
//

#include <assert.h>

#include "gc_cpp.h"

#include "Random.h"

#define rvM ((1L << 31) - 1L)
#define rvd (16807L)
#define rvquotient (rvM/rvd)
#define rvremainder (rvM%rvd)

Random::Random(unsigned num_streams) : 
  _streams(0)
{
  reset(num_streams);
}

void Random::reset(int n) 
{
  //if we already have some, throw them away and start over...
  int init_random_val = 1043618065L;
  _ns = n;
  _streams = new (GC) int[n];
  for (int i = 0; i != n; ++i) {
    // initialize random number generators to different non-zero numbers
    _streams[i] = i + init_random_val;
  }
}

unsigned Random::num_streams() const
{
  return _ns;
}

int Random::get(unsigned stream) 
{
  assert(stream < _ns);
  long result = _streams[stream];
  result = (rvd * (result % rvquotient)) -
    (rvremainder * (result / rvquotient));
  if (result <= 0) {
    result += rvM;
  }
  _streams[stream] = result;
  return result;
}

/* returns a uniformly distributed rv in base..limit
   the smallest number returned is base
   the largest number returned is base + (limit - 1)
   base, limit MUST both be >= 0
   limit MUST be > base
   these are NOT checked
*/
int Random::uniform(unsigned stream, int base, int limit) 
{
  int result;
  result = get(stream);
  /* got a uniform rv in 0..2**31, convert to uniform rv in base..limit */
  result %= (limit - base);
  if (result < 0) result = - result;
  return (base + result);
}
