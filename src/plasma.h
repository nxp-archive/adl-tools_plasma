//
// This provides additional plasma functionality and
// must be explicitly included in a user's program.
//

#ifndef _PLSMAA_H_
#define _PLSMAA_H_

namespace plasma {

  // Basic channel class:  Stores only a single piece of data, so 
  // a second write before a read will block.  This is designed
  // for a single producer to feed data to a single consumer.
  template <class Data>
  class Channel {
  public:
    Channel() : _ready(false), _t(0) {};
    void write(const Data &d);
    void set_notify(Thread *t,int h) { assert(!_t); _t = t; _h = h; };
    Thread *clear_notify() { Thread *t = _t; _t = 0; return t; };
    bool ready() const { return _ready; };
    void clear_ready() { _ready = false; };
    Data read() { return read_internal(false); };
    int get() { return read_internal(true); };
  private:
    Data read_internal(bool clear_ready);

    Data       _data;
    bool    _ready;
    Thread *_t;
    int     _h;
  };

  template <class Data>
  Data Channel<Data>::read_internal(bool clearready)
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

  template <class Data>
  void Channel<Data>::write(const Data &d) 
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

}

#endif
