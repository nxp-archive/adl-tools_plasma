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

// We can't use the plasma.h channel b/c it uses Plasma constructs.
// So, we declare it here in C++.
template <class Data>
class Channel {
public:
  Channel() : _ready(false), _readt(0), _writet(0) {};
  void write(const Data &d);
  bool ready() const { return _ready; };
  void clear_ready() { _ready = false; };
  Data read() { return read_internal(false); };
  Data get() { return read_internal(true); };

  // These are marked as non-mutex b/c they are used by alt, which already
  // does the locking.
  void set_notify(THandle t) { assert(!_readt); _readt = t; };
  THandle clear_notify() { THandle t = _readt; _readt = 0; return t; };
private:
  void set_writenotify(THandle t) { assert(!_writet); _writet = t; };
  THandle clear_writenotify() { THandle t = _writet; _writet = 0; return t; };
  Data read_internal(bool clear_ready);

  Data       _data;
  bool       _ready;
  THandle    _readt;
  THandle    _writet;
};

  template <class Data>
  Data Channel<Data>::read_internal(bool clearready)
  {
    pLock();
    if (!_ready) {
      set_notify(pCurThread());
      pSleep();
    }
    if (_writet) {
      pAddReady(clear_writenotify());
    }
    Data temp = _data;
    if (clearready) {
      clear_ready();
    }
    pUnlock();
    return temp;
  }

  template <class Data>
  void Channel<Data>::write(const Data &d) 
  { 
    pLock();
    if (_ready) {
      set_writenotify(pCurThread());
      pSleep();
    }
    _data = d;
    _ready = true;
    if (_readt) {
      pWake(clear_notify());
    }
    pUnlock();
  };


void producer(void *);
void consumer(void *);

typedef Channel<int> IntChan;

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

  Thread *p1 = pSpawn(producer,&parg1,-1);
  Thread *p2 = pSpawn(producer,&parg2,-1);
  Thread *p3 = pSpawn(producer,&parg3,-1);
  Thread *p4 = pSpawn(producer,&parg4,-1);
  Thread *c  = pSpawn(consumer,channels,-1);
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
    int j;
    while (true) {
      j = 0;
      pLock();
      for ( ; j != NumChan; ++j) {
        if (channels[j].ready()) {
          goto DataReady;
        } else {
          channels[j].set_notify(pCurThread());
        }
      }
      pSleep();
    }
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
