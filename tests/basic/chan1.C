
#include <assert.h>
#include <iostream>

#include "plasma-interface.h"

using namespace std;

void producer(void *);
void consumer(void *);

class Channel {
public:
  Channel() : _ready(false), _t(0) {};
  void write(int d);
  void set_notify(Thread *t,int h) { assert(!_t); _t = t; _h = h; };
  Thread *clear_notify() { Thread *t = _t; _t = 0; return t; };
  bool ready() const { return _ready; };
  void clear_ready() { _ready = false; };
  int read() { return read_internal(false); };
  int get() { return read_internal(true); };
private:
  int read_internal(bool clear_ready);

  bool    _ready;
  int     _data;
  Thread *_t;
  int     _h;
};

int Channel::read_internal(bool clearready)
{
  pLock();
  if (!_ready) {
    set_notify(pCurThread(),0);
    pSleep();
  }
  if (_t) {
    pAddReady(clear_notify());
  }
  int temp = _data;
  if (clearready) {
    clear_ready();
  }
  pUnlock();
  return temp;
}

void Channel::write(int d) 
{ 
  pLock();
  if (_ready) {
    set_notify(pCurThread(),0);
    pSleep();
  }
  _data = d;
  _ready = true;
  if (_t) {
    pWake(clear_notify(),_h);
  }
  pUnlock();
};

struct PArg {
  int      val;
  Channel &chan;

  PArg(int a,Channel &c) : val(a), chan(c) {};
};

const int NumChan = 4;

int pMain(int argc,const char *argv[])
{ 
  Channel channels[NumChan];
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
  Channel &chan = ((PArg*)a)->chan;
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
  Channel *channels = ((Channel*)a);
  for (int negcount = 0; negcount < NumChan; ) {
    int j = 0, end = 0;
    pLock();
    for ( ; j != NumChan; ++j) {
      if (channels[j].ready()) {
        goto DataReady;
      } else {
        channels[j].set_notify(pCurThread(),j);
        end = j+1;
      }
    }
    j = pSleep();
    pLock();
  DataReady:
    for ( int i = 0; i != end; ++i) {
      channels[i].clear_notify();
    }
    int v = channels[j].get();
    printf ("Data read from channel %d:  %d\n",j,v);
    if (v < 0) ++negcount;
  }
  printf ("Consumer done.\n");
}
