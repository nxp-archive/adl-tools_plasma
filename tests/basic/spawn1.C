//
// C++-only sample implementation of a result thread.
//
#include <iostream>

#include <plasma-interface.h>

using namespace std;
using namespace plasma;

template<class R>
class Result {  
public:
  Result(pair<THandle,void *> a) : _result((R*)a.second),_t(a.first) {}; 
  R value() const {
    if (!pDone(_t)) {
      pWait(_t);
    }
    return *_result;
  }
private:
  R       *_result;
  THandle  _t;

};

double foo(double a,double b)
{
  for (int i = 0; i != 100000000; ++i) {
    int xx = 10;
  }
  return a*a + b*b;
}

struct TmpArgs {
  double _result;
  double a;
  double b;
};

void tmp(void *args)
{
  ((TmpArgs*)args)->_result = foo(((TmpArgs*)args)->a,((TmpArgs*)args)->b);
}

Result<double> spawn( double(*fun)(double ,double),double a1,double a2)
{
  TmpArgs ta = {0,a1,a2};
  return Result<double>(pSpawn(tmp,sizeof(TmpArgs),&ta));
}

int pMain(int argc,const char *argv[])
{ 
  Result<double> result1 = spawn(foo,1.1,2.2);
  Result<double> result2 = spawn(foo,2.7,9.8);
  Result<double> result3 = spawn(foo,100.45,3.59);
  cout << "Result is:  " << result1.value() << ", " << result2.value() << ", " << result3.value() << endl;
  return 0;
}
