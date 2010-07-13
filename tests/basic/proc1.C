//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
 
#include <assert.h>
#include <stdio.h>
#include <iostream>

#include "Interface.h"

using namespace std;
using namespace plasma;

struct PArg {
  double  val;
  int     d;

  PArg(double a,int x) : val(a), d(x) {};
};

void func(void *);
void delay(double x,int d = 10,int w = 10000000);

int pMain(int argc,const char *argv[])
{ 
  Processor p1,p2;

  PArg parg1(1,10);
  PArg parg2(2,10);
  PArg parg3(3,10);
  PArg parg4(4,20);
  PArg parg5(5,10);

  Thread *t1 = pSpawn(func,&parg1,-1);
  Thread *t2 = pSpawn(p1(),func,&parg2,-1);
  Thread *t3 = pSpawn(p1(),func,&parg3,-1);
  Thread *t4 = pSpawn(p2(),func,&parg4,-1);
  Thread *t5 = pSpawn(p2(),func,&parg5,-1);
  pWait(t1);
  pWait(t2);
  pWait(t3);
  pWait(t4);
  pWait(t5);
  printf ("Done.\n");
  return 0;
}

void delay(double x,int d,int w)
{
  for (int i = 0; i != d; ++i) {
    int j = 0;
    for (int i = 0; i != w; ++i) {
      j += 1;
    }
    printf ("In block %.1f.\n",x);
  }
}

void func(void *a) 
{
  double val = ((PArg*)a)->val;
  int d = ((PArg*)a)->d;

  delay(val,d);
  printf ("Done with block %.1f.\n",val);
}
