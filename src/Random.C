//
// Random number generation routines.
//

#include <iostream>
#include <sys/time.h>
#include <unistd.h>

#include "Random.h"

using namespace std;

namespace plasma {

  void writeInt(ostream &os,unsigned x)
  {
    os.write((char *)&x,sizeof(x));
  }

  unsigned readInt(istream &is)
  {
    unsigned x;
    is.read((char *)&x,sizeof(x));
    return x;
  }

  // The default seed is system time xor'd with the pid.
  unsigned defaultSeed()
  {
    struct timeval t;
    gettimeofday( &t, NULL );
    return t . tv_sec ^ getpid();
  }

  unsigned triangleImpl(unsigned lower,unsigned mode,unsigned upper,
                        bool upper_half,double r1,double r2)
  {
    if (upper_half) {
      // Return in upper half of triangle.
      lower = mode;
    } else {
      // Return in lower half of triangle.
      upper = mode;
    }
    unsigned range = upper - lower;
    if (!range) {
      return upper;
    } else {
      unsigned rv = (unsigned)(r1 * range);
      if (upper_half) {
        upper = upper - rv;
      } else {
        lower = rv + lower;
      }
      range = upper - lower;
      if (!range) {
        return upper;
      } else {
        rv = (unsigned)(r2 * range);
        return (lower + rv);
      }
    }
  }

  unsigned normalImpl(unsigned mean,double std_dev,double r1,double r2)
  {
    static double value1 = 0.0;
    static double value2 = 0.0;
    double pi = acos(-1.0);

    double result;
    if (value1 == 0.0 && value2 == 0.0) {
      // Need new randoms.
      value1 = r1;
      value2 = r2;
      result = sqrt(-2 * log(value1)) * cos(2*pi*value2);
    } else {
      // Create the other independent normal variate.
      result = sqrt(-2 * log(value2)) * sin(2*pi*value1);
      value1 = 0.0;
      value2 = 0.0;
    }
    return (unsigned)(mean + result + std_dev);
  }

}
