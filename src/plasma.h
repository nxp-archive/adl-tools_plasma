//
// This provides additional plasma functionality and
// must be explicitly included in a user's program.
//

#ifndef _PLSMAA_H_
#define _PLSMAA_H_

#include <assert.h>

namespace plasma {

  // Basic channel class:  Stores only a single piece of data, so 
  // a second write before a read will block.  This is designed
  // for a single producer to feed data to a single consumer.
  template <class Data>
  class Channel {
  public:
    Channel() : _ready(false), _readt(0), _writet(0) {};
    void write(const Data &d);
    void set_notify(THandle t,int h) { assert(!_readt); _readt = t; _h = h; };
    THandle clear_notify() { THandle t = _readt; _readt = 0; return t; };
    void set_writenotify(THandle t,int h) { assert(!_writet); _writet = t; _h = h; };
    THandle clear_writenotify() { THandle t = _writet; _writet = 0; return t; };
    bool ready() const { return _ready; };
    void clear_ready() { _ready = false; };
    Data read() { return read_internal(false); };
    Data get() { return read_internal(true); };
  private:
    Data read_internal(bool clear_ready);

    Data    _data;
    bool    _ready;
    THandle _readt;
    THandle _writet;
    int     _h;
  };

  // This channel is used to interface with a spawned thread.  It is a read-only
  // channel initialized by a Result object returned by the spawn operator.  In
  // other words, there is no way to write to this channel- it gets its value
  // solely from the result returned by the thread.  Since it only ever has a
  // single value, read() and get() will always return the same value.  However, a get()
  // operation will clear the _read flag so that a call to clear_ready() will be required in
  // order for ready() to return true again.  This is done so that repeated reads using an alt
  // block will work.
  template <class Data>
  class ResChan : private Result<Data> {
  public:
    ResChan(const Result<Data> &r) : Result<Data>(r), _rt(0), _read(false) {};
    
    bool ready() const { return done() && !_read; };
    Data read() { return value(); };
    Data get() { _read = true; return value(); };
    void clear_ready() { _read = false; };
    void set_notify(THandle t,int h);
    THandle clear_notify();
  private:
    THandle _rt;
    bool    _read;
  };

  //////////////////////////////////////////////////////////////////////////////
  //
  // Implementation.
  //
  //////////////////////////////////////////////////////////////////////////////

  template <class Data>
  Data Channel<Data>::read_internal(bool clearready)
  {
    pLock();
    if (!_ready) {
      set_notify(pCurThread(),0);
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
      set_writenotify(pCurThread(),0);
      pSleep();
    }
    _data = d;
    _ready = true;
    if (_readt) {
      pWake(clear_notify(),_h);
    }
    pUnlock();
  };

  template <class Data>
  void ResChan<Data>::set_notify(THandle t,int h) 
  { 
    assert(!_rt); 
    _rt = t; 
    pAddWaiter(thread(),_rt); 
    pSetHandle(thread(),h); 
  };

  template <class Data>
  THandle ResChan<Data>::clear_notify() 
  { 
    THandle t = _rt; 
    pClearWaiter(thread(),_rt); 
    _rt = 0; 
    return t; 
  };

}

#endif
