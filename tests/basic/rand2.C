//
// Tests of the various distributions possible.
//

#include <iostream>
#include "Random.h"

using namespace std;
using namespace plasma;

typedef Random<> Rand;

const int N = 10000;

void testDbl(unsigned);
void testUniform(unsigned);
void testTriangle(unsigned);
void testExponential(unsigned);
void testNormal(unsigned);

int main(int argc,const char *argv[])
{
  unsigned seed = defaultSeed();
  if (argc > 1) {
    seed = atoi(argv[1]);
  }

  cout << "Seed:  " << seed << endl;

  testDbl(seed);
  testUniform(seed);
  testTriangle(seed);
  //  testExponential(seed);
  //  testNormal(seed);
}

void testDbl(unsigned s)
{
  Rand r(1,s);

  for (int i = 0; i != N; ++i) {
    double d = r.gendbl();
    if (d < 0 || d > 1) {
      cerr << "gendbl:  Got " << d << " on pick #" << i << endl;
      exit (1);
    }
  }
  cout << "gendbl test passed." << endl;
}

void uniformImpl(unsigned seed,unsigned upper,unsigned count)
{
  Rand r(1,seed);

  for (int i = 0; i != 100; ++i) {
    unsigned l = r.uniform(0,0,upper);
    unsigned u = r.uniform(0,l,upper);
    if (l > upper) {
      cerr << "uniform:  Failed on initial lower pick." << endl;
      exit (1);
    }
    if (u < l) {
      cerr << "uniform:  Failed on initial upper pick." << endl;
      exit (1);
    }
    for (unsigned j = 0; j != count; ++j) {
      unsigned p = r.uniform(0,l,u);
      if (p < l || p > u) {
        cerr << "uniform:  Bad pick:  Got " << p << " for range [" << l << "," << u << "]." << endl;
        exit (1);
      }
    }
  }
}

void testUniform(unsigned seed)
{
  uniformImpl(seed,10,1000);
  uniformImpl(seed,100,2000);
  uniformImpl(seed,10000,3000);
  uniformImpl(seed,(unsigned)-1,N);
  cout << "uniform test passed." << endl;
}

void triangleImpl(unsigned seed,unsigned upper,unsigned count)
{
  Rand r(1,seed);

  int fails = 0;
  for (int i = 0; i != 100; ++i) {
    unsigned l = r.uniform(0,0,upper-20);
    unsigned u = r.uniform(0,l+20,upper);
    unsigned m = r.uniform(0,l+10,u-10);
    unsigned lbin=0,mbin=0,ubin=0;
    unsigned lp = ((m - l) / 2) + l;
    unsigned up = ((u - m) / 2) + m;
    if (l > upper) {
      cerr << "triangle:  Failed on initial lower pick." << endl;
      exit (1);
    }
    if (u < l) {
      cerr << "triangle:  Failed on initial upper pick." << endl;
      exit (1);
    }
    if (m < l || m > u) {
      cerr << "triangle:  Failed on mode pick." << endl;
      exit (1);
    }
    for (unsigned j = 0; j != count; ++j) {
      unsigned p = r.triangle(0,l,m,u);
      //      cout << "pick:  " << p << endl;
      if (p < l || p > u) {
        cerr << "triangle:  Bad pick:  Got " << p << " for range [" << l << "," << u << "], mode " << m << "." << endl;
        exit (1);
      }
      if ( (p > up-10) && (p < up+10)) {
        ++ubin;
      }
      if ( (p > m-10) && (p < m+10)) {
        ++mbin;
      }
      if ( (p > lp-10) && (p < lp+10)) {
        ++lbin;
      }
    }
    //cout << "lbin:  " << lbin << ", mbin:  " << mbin << ", ubin:  " << ubin << endl;
    if (ubin > mbin) {
      ++fails;
      //      cerr << "triangle:  Did not conform to distribution- upper half greater than mid." << endl;
      //      cerr << "lbin:  " << lbin << ", mbin:  " << mbin << ", ubin:  " << ubin << endl;
      //      exit (1);
    }
    if (lbin > mbin) {
      ++fails;
      ///      cerr << "triangle:  Did not conform to distribution- lower half greater than mid." << endl;
      //      cerr << "lbin:  " << lbin << ", mbin:  " << mbin << ", ubin:  " << ubin << endl;
      //      exit (1);
    }
  }
  if (fails > 10) {
    cerr << "triangle:  Failed distribution test." << endl;
    exit (1);
  }
}

void testTriangle(unsigned seed)
{
  triangleImpl(seed,100,N);
  triangleImpl(seed,10000,N);
  triangleImpl(seed,(unsigned)-1,N);
  cout << "triangle test passed." << endl;
}

// Not the greatest test in the world- might want to improve this later.
void testExponential(unsigned seed)
{
  Rand r(1,seed);

  unsigned lc = 0, hc = 0;
  for (int i = 0; i != N; ++i) {
    unsigned p = r.exponential(0,1000,4);
    cout << "p:  " << p << endl;
    if (p < 100) ++lc;
    if (p > 1900) ++hc;
  }
  cout << "lc:  " << lc << ", hc:  " << hc << endl;
  if (hc >= lc) {
    cerr << "exponential:  Failed distribution test." << endl;
    exit (1);
  }
  if (!hc || !lc) {
    cerr << "exponential:  Bad counts." << endl;
  }
  cout << "exponential test passed." << endl;
}

void testNormal(unsigned seed)
{
  Rand r(1,seed);

  for (int i = 0; i != N; ++i) {
    unsigned p = r.normal(0,100,32.0);
    cout << "p:  " << p << endl;   
  }

}
