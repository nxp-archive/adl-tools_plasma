//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
//
// Random number test.
//

#include <iostream>
#include <sstream>

#include "Random.h"

using namespace std;
using namespace plasma;

// Wrapper class for random number generator streams.
struct RandBase {
  virtual ~RandBase() {};
  virtual unsigned num_streams() const = 0;
  virtual unsigned seed() const = 0;
  virtual unsigned genrand(unsigned) = 0;
  virtual unsigned rgenrand(unsigned) = 0;
  virtual void reset() = 0;
  virtual void save(ostream &) = 0;
  virtual void load(istream &) = 0;
};

template<class R>
class RandT : public RandBase
{
public:
  RandT(unsigned n = 1,unsigned s = 0) : _r(n,s) {};
  virtual unsigned num_streams() const { return _r.num_streams(); };
  virtual unsigned seed() const { return _r.seed(); };
  virtual unsigned genrand(unsigned s) { return _r.genrand(s); };
  virtual unsigned rgenrand(unsigned s) { return _r.rgenrand(s); };
  virtual void reset() { _r.reset(); };
  virtual void save(ostream &os) { _r.save(os); };
  virtual void load(istream &is) { _r.load(is); };
private:
  Random<R> _r;
};

void testRand(const char *msg,RandBase &,RandBase &,bool testrev);

const int N = 100000;
const int S = 100;

unsigned buf[N];
unsigned tbuf[S][N];

int main(int argc,const char *argv[])
{
  unsigned seed = 0;
  if (argc > 1) {
    seed = atoi(argv[1]);
  }

  RandT<LcgRand> l1(1,seed),l2(1,l1.seed());
  cout << "LCG:  Seed:  " << l1.seed() << endl;
  testRand("LCG",l1,l2,true);

  RandT<KissRand> k1(10,seed),k2(10,k1.seed());
  cout << "KISS:  Seed:  " << k1.seed() << endl;
  testRand("KISS",k1,k2,false);

  RandT<MtRand> m1(100,seed),m2(100,m1.seed());
  cout << "MT:  Seed:  " << m1.seed() << endl;
  testRand("MT",m1,m2,true);

  return 0;
}

void testRand(const char *msg,RandBase &r1,RandBase &r2,bool testrev)
{
  assert((int)r1.num_streams() <= S && (int)r2.num_streams() <= S);

  // First, make sure that for a given seed, we get the same numbers.
  for (int i = 0; i != N; ++i) {
    buf[i] = r1.genrand(0);
  }

  for (int i = 0; i != N; ++i) {
    if (buf[i] != r2.genrand(0)) {
      cerr << msg << ":  Mismatch between r1 and r2 at element " << i << endl;
      exit (1);
    }
  }

  // Make sure that reset works.
  r1.reset();
  for (int i = 0; i != N; ++i) {
    if (buf[i] != r1.genrand(0)) {
      cerr << msg << ":  Mismatch between r1 and r1 (reset) at element " << i << endl;
      exit (1);
    }
  }

  // Make sure that reverse works.
  if (testrev) {
    for (int i = N-1; i >= 0; --i) {
      if (buf[i] != r1.rgenrand(0)) {
        cerr << msg << ":  Mismatch in reverse at element " << i << endl;
        exit (1);
      }
    }
  }

  // Make sure that streams are independent by generating numbers
  // in ascending and descending stream order.
  {
    r1.reset();
    r2.reset();

    int s = min(r1.num_streams(),r2.num_streams());
    for (int i = 0; i != s; ++i) {
      for (int j = 0; j != N; ++j) {
        tbuf[i][j] = r1.genrand(i);
      }
    }
    for (int i = s-1; i >= 0; --i) {
      for (int j = 0; j != N; ++j) {
        if (tbuf[i][j] != r2.genrand(i)) {
          cerr << msg << ":  Mismatch r1 vs. r2, stream " << i << ", element " << j << endl;
          exit (1);
        }
      }
    }
  }

  // Test load/save features.
  {
    // First, cycle the generators a bit to advance past the reset state.
    r1.reset();
    for (int i = 0; i != (int)r1.num_streams(); ++i) {
      for (int j = 0; j != 1000; ++j) {
        r1.genrand(i);
      }
    }

    ostringstream os;
    r1.save(os);
  
    // Now save some data.
    for (int i = 0; i != (int)r1.num_streams(); ++i) {
      for (int j = 0; j != N; ++j) {
        tbuf[i][j] = r1.genrand(i);
      }
    }

    // Restore the generators.
    istringstream is(os.str());
    r1.load(is);

    // Make sure that the data is equal.
    for (int i = 0; i != (int)r1.num_streams(); ++i) {
      for (int j = 0; j != N; ++j) {
        if (tbuf[i][j] != r1.genrand(i)) {
          cerr << msg << ":  Mismatch r1 vs. r1 loaded, stream " << i << ", element " << j << endl;
          exit (1);
        }
      }
    }    

  }  

  cout << msg << ":  All tests pass." << endl;
}

int pMain(int argc,const char *[])
{
  return 0;
}
