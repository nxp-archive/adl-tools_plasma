//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
//
// C++-only sample implementation of a result thread.
//
#include <iostream>

#include <Interface.h>

using namespace std;
using namespace plasma;

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

Result<double> spawn(double a1,double a2)
{
  TmpArgs ta = {double(),a1,a2};
  return Result<double>(pSpawn(tmp,sizeof(TmpArgs),&ta,-1));
}

int pMain(int argc,const char *argv[])
{ 
  Result<double> result1 = spawn(1.1,2.2);
  Result<double> result2 = spawn(2.7,9.8);
  Result<double> result3 = spawn(100.45,3.59);
  cout << "Result is:  " << result1.value() << ", " << result2.value() << ", " << result3.value() << endl;
  return 0;
}
