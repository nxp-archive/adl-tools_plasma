//
// This provides additional plasma functionality and
// must be explicitly included in a user's program.
//

#ifndef _PLSMAA_H_
#define _PLSMAA_H_

#include <assert.h>
#include <vector>
#include <list>

namespace plasma {

  // Basic channel class:  Stores only a single piece of data, so 
  // a second write before a read will block.  This is designed
  // for a single producer to feed data to a single consumer.
  template <class Data>
  pTMutex class Channel {
  public:
    Channel() : _ready(false), _readt(0), _writet(0) {};
    void write(const Data &d);
    bool ready() const { return _ready; };
    void clear_ready() { _ready = false; };
    Data read() { return read_internal(false); };
    Data get() { return read_internal(true); };

    // These are marked as non-mutex b/c they are used by alt, which already
    // does the locking.
    pNoMutex void set_notify(THandle t,int h) { assert(!_readt); _readt = t; _h = h; };
    pNoMutex THandle clear_notify() { THandle t = _readt; _readt = 0; return t; };
  private:
    void set_writenotify(THandle t) { assert(!_writet); _writet = t; };
    THandle clear_writenotify() { THandle t = _writet; _writet = 0; return t; };
    Data read_internal(bool clear_ready);

    Data    _data;
    bool    _ready;
    THandle _readt;
    THandle _writet;
    int     _h;
  };

  // Queued channel class:  The class may store either an arbitrary number of
  // objects or a fixed number.  If a fixed number, a write will block if the
  // channel is full.  This is designed for multiple producers to feed data to
  // a single consumer.
  template <class Data>
  pTMutex class QueueChan {
    typedef std::list<Data>      Store;
    typedef std::vector<THandle> Writers;
  public:
    QueueChan(int size = -1) : _maxsize(size), _size(0), _readt(0) {};
    void write(const Data &d);
    bool ready() const { return !_store.empty(); };
    int size() const { return _size; };
    pNoMutex int maxsize() const { return _maxsize; };
    void clear_ready() { pLock(); --_size; _store.pop_back(); pUnlock(); };
    Data read() { return read_internal(false); };
    Data get() { return read_internal(true); };

    pNoMutex void set_notify(THandle t,int h) { assert(!_readt); _readt = t; _h = h; };
    pNoMutex THandle clear_notify() { THandle t = _readt; _readt = 0; return t; };
  private:
    void set_writenotify(THandle t) { _writers.push_back(t); };
    THandle next_writer() { THandle t = _writers.back(); _writers.pop_back(); return t; };
    Data read_internal(bool clear_ready);

    int     _maxsize;   // Max size.  If -1, no fixed size.
    int     _size;      // Current size of queue.
    Store   _store;
    THandle _readt;     // A blocked read thread.
    Writers _writers;   // One or more blocked writers.
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
  pTMutex class ResChan : private Result<Data> {
  public:
    ResChan(const Result<Data> &r) : Result<Data>(r), _rt(0), _read(false) {};
    
    bool ready() const { return done() && !_read; };
    Data read() { return value(); };
    Data get() { _read = true; return value(); };
    void clear_ready() { _read = false; };

    pNoMutex void set_notify(THandle t,int h);
    pNoMutex THandle clear_notify();
  private:
    THandle _rt;
    bool    _read;
  };

  //////////////////////////////////////////////////////////////////////////////
  //
  // Implementation.
  //
  //////////////////////////////////////////////////////////////////////////////

  /////////////// Channel ///////////////

  template <class Data>
  Data Channel<Data>::read_internal(bool clearready)
  {
    if (!_ready) {
      set_notify(pCurThread(),0);
      pSleep();
    }
    if (_writet) {
      pAddReady(clear_writenotify());
    }
    if (clearready) {
      clear_ready();
    }
    return _data;
  }

  template <class Data>
  void Channel<Data>::write(const Data &d) 
  { 
    if (_ready) {
      set_writenotify(pCurThread());
      pSleep();
    }
    _data = d;
    _ready = true;
    if (_readt) {
      pWake(clear_notify(),_h);
    }
  };

  /////////////// QueueChan ///////////////

  template <class Data>
  Data QueueChan<Data>::read_internal(bool clearready)
  {
    // If no data- sleep until we get some.
    if (!ready()) {
      set_notify(pCurThread(),0);
      pSleep();
    }
    // If there's a waiting writer (queue is full) and we're
    // going to remove an item, then unblock the next writer here.
    if (_writers.size() && clearready) {
      pAddReady(next_writer());
    }
    Data temp = _store.back();
    if (clearready) {
      clear_ready();
    }
    return temp;
  }

  template <class Data>
  void QueueChan<Data>::write(const Data &d) 
  { 
    // Sleep if queue is full.  This is a loop so that if a waiting
    // writer is awakened and then another thread jumps in and writess
    // data to again fill the queue, the thread will again sleep.
    while (_maxsize >= 0 && _size >= _maxsize) {
      set_writenotify(pCurThread());
      pSleep();
    }
    _store.push_front(d);
    ++_size;
    assert(_maxsize < 0 || _size <= _maxsize);
    // Wake a sleeping reader.
    if (_readt) {
      pWake(clear_notify(),_h);
    }
  };

  /////////////// ResChan ///////////////

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
