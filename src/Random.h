//
// Random number support for Plasma simulation.
// The basic Random class supports 1 or more independent streams
// of random numbers.  The class is parameterized on the actual
// generator used.  Each class is independent and may be instantiated
// as many times as desired.
//
// A generator is simply a class which provides a certain interface.
// Thus the user may create a generator and use it with Random.  The
// generator needs to provide:
//
// Default constructor.
//
// void set_seed(unsigned):          Init the generator with the given seed.
//
// unsigned genrand():               Return the next random number.
//
// unsigned rgenrand():              Return the previous random number, i.e. go in reverse.
//                                   This must be defined, but may simply abort.  In other words,
//                                   we don't require that a generator is reversible, just that it
//                                   has this function.
// 
// void save(std::ostream &) const:  Save state to the stream.    
//
// void load(std::istream &):        Load state from the stream.
//

#ifndef _RANDOM_H_
#define _RANDOM_H_

#include <iosfwd>
#include <vector>
#include <stdexcept>
#include <assert.h>

#include "LcgRand.h"
#include "KissRand.h"
#include "MtRand.h"

namespace plasma {

  template <class Gen = KissRand>
  class Random {
  public:
    // If the seed is 0, we will use the system time to set the seed.
    // Each generator is set with seed + n, where n is the index of the generator.
    Random(unsigned num_streams=1,unsigned seed = 0);
    unsigned num_streams() const { return _randgens.size(); };
    unsigned seed() const { return _seed; };
    // Get the next random number (32 bit).
    unsigned genrand(unsigned s=0) { good_stream(s); return _randgens[s].genrand(); };
    // Get the previous random number (32 bit).
    unsigned rgenrand(unsigned s=0) { good_stream(s); return _randgens[s].rgenrand(); };
    // Generate a double in [0,1].
    double gendbl(unsigned stream=0);
    // Uniform distribution of [base,limit).  Limit must be greater than base.
    unsigned uniform(unsigned stream,unsigned base,unsigned limit);
    // Triangle distribution in [lower,upper).  Upper must be greater than lower.
    unsigned triangle(unsigned stream,unsigned lower,unsigned mode,unsigned upper);
    // Exponential distribution.  Range is [0,scale], with lambda describing fall-off rate.
    unsigned exponential(unsigned stream,unsigned scale,double lambda);
    // Normal distribution.
    unsigned normal(unsigned stream,unsigned mean,double std_dev);
    // Reset all generators back to original seed.
    void reset();
    // Write the state of all generators to the specified stream.
    void save(std::ostream &) const;
    // Read the state of all genereators.  Will throw a runtime_error
    // if the number of streams is different than what it reads.
    void load(std::istream &);
  private:
    void good_stream(unsigned s) { assert(s < num_streams()); };

    typedef std::vector<Gen> GenStore;

    unsigned  _seed;
    GenStore  _randgens;
  };

  void writeInt(std::ostream &,unsigned);
  unsigned readInt(std::istream &);

  unsigned defaultSeed();
  unsigned triangleImpl(unsigned lower,unsigned mode,unsigned upper,
                        bool upper_half,double r1,double r2);
  double normalImpl(unsigned mean,double std_dev,double r1,double r2);

  template <class Gen>
  Random<Gen>::Random(unsigned num_streams,unsigned seed) : 
    _seed( (seed) ? seed : defaultSeed()),
    _randgens(num_streams)
  {
    reset();
  }

  template <class Gen>
  double Random<Gen>::gendbl(unsigned s)
  {
    good_stream(s);
    return genrand(s)*(1.0/4294967295.0); 
  }

  template <class Gen>
  unsigned Random<Gen>::uniform(unsigned s,unsigned base,unsigned limit)
  {
    assert(base <= limit);
    if (base == limit) {
      return base;
    } else {
      unsigned result = genrand(s) % (limit - base);
      return (base + result);
    }
  }

  template <class Gen>
  unsigned Random<Gen>::triangle(unsigned s,unsigned lower,unsigned mode,unsigned upper)
  {
    return triangleImpl(lower,mode,upper,genrand(s)%2,gendbl(s),gendbl(s));
  }

  template <class Gen>
  unsigned Random<Gen>::exponential(unsigned s,unsigned scale,double lambda)
  {
    assert(lambda);
    double result = (-log(1-gendbl(s)))/lambda;
    unsigned r = (unsigned)(result * scale);
    return (r > scale) ? scale : r;
  }

  template <class Gen>
  unsigned Random<Gen>::normal(unsigned s,unsigned mean,double std_dev)
  {
    return (unsigned)normalImpl(mean,std_dev,gendbl(s),gendbl(s));
  }

  template <class Gen>
  void Random<Gen>::reset()
  {
    for (unsigned i = 0; i != _randgens.size(); ++i) {
      _randgens[i].set_seed(_seed+i);
    }
  }
  
  template <class Gen>
  void Random<Gen>::save (std::ostream &os) const
  {
    writeInt(os,_randgens.size());
    for (unsigned i = 0; i != _randgens.size(); ++i) {
      _randgens[i].save(os);
    }
  }

  template <class Gen>
  void Random<Gen>::load (std::istream &is)
  {
    unsigned n = readInt(is);
    if (n != _randgens.size()) {
      throw std::runtime_error("Size mismatch on number of random streams.");
    }
    for (unsigned i = 0; i != _randgens.size(); ++i) {
      _randgens[i].load(is);
    }
  }

}

#endif
