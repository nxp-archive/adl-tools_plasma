//
// This provides additional plasma functionality and
// must be explicitly included in a user's program.
//

#ifndef _PLASMA_H_
#define _PLASMA_H_

#include <stdarg.h>
#include <assert.h>
#include <vector>
#include <list>

namespace plasma {

  // Basic mutex I/O routines.  These lock the processor before performing output.
  int mprintf(const char *format, ... );
  int mfprintf(FILE *,const char *format, ...);
  int mvprintf(const char *format, va_list ap);
  int mvfprintf(FILE *,const char *format,va_list ap);

  // This duplicates a string, returning a GC-allocated char*.  There is then
  // no need to delete this string.
  char *gc_strdup(const char *);
  char *gc_strdup(const std::string &s);

  // Basic channel class:  Stores only a single piece of data, so 
  // a second write before a read will block.  This is generally
  // used bya single producer to go to a single consumer, but it is
  // possible to have multiple producers.
  template <class Data>
  pTMutex class Channel {
    typedef std::vector<THandle,traceable_allocator<THandle> > Writers;
  public:
    Channel() : _ready(false), _readt(0) {};
    void write(const Data &d);
    bool ready() const { return _ready; };
    void clear_ready() { _ready = false; };
    Data read() { return read_internal(false); };
    Data get() { return read_internal(true); };

    // These are marked as non-mutex b/c they are used by alt, which already
    // does the locking.
    pNoMutex void set_notify(THandle t,HandleType h) { assert(!_readt); _readt = t; _h = h; };
    pNoMutex THandle clear_notify() { THandle t = _readt; _readt = 0; return t; };
  private:
    void set_writenotify(THandle t) { _writers.push_back(t); };
    bool have_writers() const { return !_writers.empty(); };
    THandle next_writer() { THandle t = _writers.back(); _writers.pop_back(); return t; };
    Data read_internal(bool clear_ready);

    Data       _data;
    bool       _ready;
    THandle    _readt;
    Writers    _writers;
    HandleType _h;
  };

  // Queued channel class:  The class may store either an arbitrary number of
  // objects or a fixed number.  If a fixed number, a write will block if the
  // channel is full.  This is designed for multiple producers to feed data to
  // a single consumer.  If size is 0, then no max size exists.
  template <typename Data,typename Container = std::list<Data,traceable_allocator<Data> > >
  pTMutex class QueueChan {
    typedef Container Store;
    typedef std::vector<THandle,traceable_allocator<THandle>> Writers;
  public:
    QueueChan(int size = 0) : _maxsize(size), _size(0), _readt(0) {};
    void write(const Data &d);
    bool ready() const { return !empty(); };
    bool full() const { return _maxsize && _size >= _maxsize; };
    bool empty() const { return _size == 0; };
    int size() const { return _size; };
    pNoMutex int maxsize() const { return _maxsize; };
    void setMaxSize(int ms) { _maxsize = ms; }
    void clear_ready() { pLock(); if (_size) { --_size; _store.pop_back(); } pUnlock(); };
    Data read() { return read_internal(false); };
    Data get() { return read_internal(true); };

    pNoMutex void set_notify(THandle t,HandleType h) { assert(!_readt); _readt = t; _h = h; };
    pNoMutex THandle clear_notify() { THandle t = _readt; _readt = 0; return t; };
  private:
    void set_writenotify(THandle t) { _writers.push_back(t); };
    bool have_writers() const { return !_writers.empty(); };
    void check_size() const { assert(!_maxsize || _size <= _maxsize); }
    THandle next_writer() { THandle t = _writers.back(); _writers.pop_back(); return t; };
    Data read_internal(bool clear_ready);

    unsigned   _maxsize;   // Max size.  If 0, no fixed size.
    unsigned   _size;      // Current size of queue.
    Store      _store;
    THandle    _readt;     // A blocked read thread.
    Writers    _writers;   // One or more blocked writers.
    HandleType _h;
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
    typedef Result<Data> Base;
  public:
    ResChan(const Result<Data> &r) : Result<Data>(r), _rt(0), _read(false) {};
    
    bool ready() const { return Base::done() && !_read; };
    Data read() { return Base::value(); };
    Data get() { _read = true; return Base::value(); };
    void clear_ready() { _read = false; };

    pNoMutex void set_notify(THandle t,HandleType h);
    pNoMutex THandle clear_notify();
  private:
    THandle _rt;
    bool    _read;
  };

  // Clocked channel class: This is a queued channel which only allows reading
  // every n time units.  This is useful for simulating a clocked design.
  template <typename Data,typename Container = std::list<std::pair<Data,ptime_t>,traceable_allocator<std::pair<Data,ptime_t> > > >
  pTMutex class ClockChan : ClockChanImpl {
    typedef std::pair<Data,ptime_t> DP;
    typedef Container Store;
    typedef std::vector<THandle,traceable_allocator<THandle> > Writers;
  public:
    ClockChan(ptime_t p,int size = 1) : ClockChanImpl(p), _maxsize(size) {};
    void write(const Data &d);
    bool ready() const { return current_data() && is_phi(); };

    pNoMutex int maxsize() const { return _maxsize; };
    void setMaxSize(int ms) { _maxsize = ms; }
    bool full() const { return _maxsize && _size >= _maxsize; };

    void clear_ready() { pLock(); if (!empty()) { --_size; _store.pop_back(); } pUnlock(); };
    Data read() { return read_internal(false); };
    Data get() { return read_internal(true); };

    using ClockChanImpl::size;
    using ClockChanImpl::empty;
    using ClockChanImpl::clear_notify;
    using ClockChanImpl::set_notify;

  private:
    bool current_data() const;
    void set_writenotify(THandle t) { _writers.push_back(t); };
    bool have_writers() const { return !_writers.empty(); };
    void check_size() const { assert(!_maxsize || _size <= _maxsize); }
    THandle next_writer() { THandle t = _writers.back(); _writers.pop_back(); return t; };
    Data read_internal(bool clear_ready);
    ptime_t curr_data_time() const { return _store.back().second; };
    ptime_t curr_time() const { return pTime(); };

    unsigned   _maxsize;   // Max size.  If 0, no fixed size.
    Store      _store;
    Writers    _writers;   // One or more blocked writers.
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
    // We'll be consuming data, so if we have a waiting
    // writer, it's valid to wake it up.
    if (have_writers() && clearready) {
      pAddReady(next_writer());
    }
    if (!_ready) {
      // We don't have data, so the reader must
      // block until the writer adds data.
      set_notify(pCurThread(),HandleType());
      pSleep();
    }
    if (clearready) {
      clear_ready();
    }
    // Reactivate a waiting writer if one appeared while we were asleep.
    if (have_writers() && clearready) {
      pAddReady(next_writer());
    }    
    return _data;
  }

  template <class Data>
  void Channel<Data>::write(const Data &d) 
  { 
    // If we have a waiting reader, wake it up.
    if (_readt) {
      pWake(clear_notify(),_h);
    }
    while (_ready) {
      // We already have data, so the write must block
      // until the reader consumes the data.
      set_writenotify(pCurThread());
      pSleep();
    }
    // Store data and set ready.
    _data = d;
    _ready = true;
    // Reactivate a reader if one appeared while we were asleep.
    if (_readt) {
      pWake(clear_notify(),_h);
    }    
  };

  /////////////// QueueChan ///////////////

  template <typename Data,typename Container>
  Data QueueChan<Data,Container>::read_internal(bool clearready)
  {
    // If there's a waiting writer (queue is full) and we're
    // going to remove an item, then unblock the next writer here.
    if (have_writers() && clearready) {
      pAddReady(next_writer());
    }
    // If no data- sleep until we get some.
    if (!ready()) {
      set_notify(pCurThread(),HandleType());
      pSleep();
    }
    Data temp = _store.back();
    if (clearready) {
      clear_ready();
    }
    if (have_writers() && clearready) {
      pAddReady(next_writer());
    }
    return temp;
  }

  template <typename Data,typename Container>
  void QueueChan<Data,Container>::write(const Data &d) 
  { 
    // If we have a waiting reader, wake it up.
    if (_readt) {
      pWake(clear_notify(),_h);
    }
    // Sleep if queue is full.  This is a loop so that if a waiting
    // writer is awakened and then another thread jumps in and writess
    // data to again fill the queue, the thread will again sleep.
    while (full()) {
      set_writenotify(pCurThread());
      pSleep();
    }
    _store.push_front(d);
    ++_size;
    check_size();
    // Reactivate a reader if one appeared while we were asleep.
    if (_readt) {
      pWake(clear_notify(),_h);
    }
  };

  /////////////// ResChan ///////////////

  template <class Data>
  void ResChan<Data>::set_notify(THandle t,HandleType h) 
  { 
    assert(!_rt); 
    _rt = t; 
    pAddWaiter(Base::thread(),_rt); 
    pSetHandle(Base::thread(),h); 
  };

  template <class Data>
  THandle ResChan<Data>::clear_notify() 
  { 
    THandle t = _rt; 
    pClearWaiter(Base::thread(),_rt); 
    _rt = 0; 
    return t; 
  };

  /////////////// ClockChan ///////////////

  template <typename Data,typename Container>
  bool ClockChan<Data,Container>::current_data() const
  {
    return (!empty() && curr_data_time() < curr_time());
  }

  template <typename Data,typename Container>
  Data ClockChan<Data,Container>::read_internal(bool clearready)
  {
    // If there's a waiting writer (queue is full) and we're
    // going to remove an item, then unblock the next writer here.
    bool r = ready();
    if (r && have_writers() && clearready) {
      pAddReady(next_writer());
    }
    // If no data- sleep until we get some.
    if (!r) {
      delayed_reader_wakeup(!empty());
    }
    Data temp = _store.back().first;
    if (clearready) {
      clear_ready();
    }
    if (have_writers() && clearready) {
      pAddReady(next_writer());
    }
    return temp;
  }

  template <typename Data,typename Container>
  void ClockChan<Data,Container>::write(const Data &d) 
  { 
    // If we have a waiting reader, wake it up.
    if (_readt) {
      delayed_wakeup(current_data());
    }
    // Sleep if queue is full.  This is a loop so that if a waiting
    // writer is awakened and then another thread jumps in and writess
    // data to again fill the queue, the thread will again sleep.
    while (full()) {
      set_writenotify(pCurThread());
      pSleep();
    }
    // Add data element w/time of next clock cycle.
    _store.push_front(std::make_pair(d,curr_time()));
    ++_size;
    check_size();
    // Reactivate a reader if one appeared while we were asleep.
    if (_readt) {
      delayed_wakeup(current_data());
    }
  };

}

#endif
