
#include <assert.h>
#include <iostream>

#include "plasma-interface.h"
#include "plasma.h"

using namespace std;
using namespace plasma;

void producer(void *);
void consumer(void *);

typedef plasma::Channel<int> IntChan;

struct PArg {
  int      val;
  IntChan &chan;

  PArg(int a,IntChan &c) : val(a), chan(c) {};
};

const int NumChan = 4;

int pMain(int argc,const char *argv[])
{ 
  IntChan channels[NumChan];
  PArg parg1(10,channels[0]);
  PArg parg2(100,channels[1]);
  PArg parg3(1000,channels[2]);
  PArg parg4(10000,channels[3]);

  Thread *p1 = pSpawn(producer,&parg1);
  Thread *p2 = pSpawn(producer,&parg2);
  Thread *p3 = pSpawn(producer,&parg3);
  Thread *p4 = pSpawn(producer,&parg4);
  Thread *c  = pSpawn(consumer,channels);
  pWait(p1);
  pWait(p2);
  pWait(p3);
  pWait(p4);
  pWait(c);
  printf ("Done.\n");
  return 0;
}

void producer(void *a) 
{
  IntChan &chan = ((PArg*)a)->chan;
  int val = ((PArg*)a)->val;

  for (int i = 0; i != 10; ++i) {
    chan.write(val+i);
    for (int j = 0; j != 100000; ++j) {
      int x = x + 1;
    }
  }
  chan.write(-1);
  printf ("Producer done.\n");
}

void consumer(void *a)
{
  IntChan *channels = ((IntChan*)a);
  for (int negcount = 0; negcount < NumChan; ) {
    int j = 0;
    pLock();
    for ( ; j != NumChan; ++j) {
      if (channels[j].ready()) {
        goto DataReady;
      } else {
        channels[j].set_notify(pCurThread(),j);
      }
    }
    j = pSleep();
    pLock();
  DataReady:
    for ( int i = 0; i != NumChan; ++i) {
      channels[i].clear_notify();
    }
    int v = channels[j].get();
    printf ("Data read from channel %d:  %d\n",j,v);
    if (v < 0) ++negcount;
  }
  printf ("Consumer done.\n");
}
