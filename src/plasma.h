//
// This provides additional plasma functionality and
// must be explicitly included in a user's program.
//

#ifndef _PLASMA_H_
#define _PLASMA_H_

#include <stdarg.h>
#include <vector>
#include <list>

#include "ChanSupport.h"

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
  template <class Data,class Base = SingleConsumerChannel >
  pTMutex class Channel : public Base, public SingleDataChannelBase {
  public:
    typedef Data value_type;

    Channel() {};
    void write(const Data &d);
    Data read() { return read_internal(false); };
    Data get() { return read_internal(true); };

  private:
    Data read_internal(bool clear_ready);
    
    Data _data;
  };

  // This is similiar to Channel in that it stores a single item, but if a read
  // blocks, it forces the processor to go busy.  This can be used for when
  // waiting on a resource holds up a task, e.g. a processor waiting on a load
  // can't do something else.
  // The user can specify a timeslice value in the constructor, or specify 0 to
  // mean no timeslice.
  // Note:  This is not protected by mutex code b/c if we're in busy-okay mode,
  // we don't have preemption.
  template <class Data,class Base = SingleConsumerChannel >
  class BusyChan : public Base, public SingleDataChannelBase {
  public:
    typedef Data value_type;

    BusyChan(ptime_t ts = 0) : _timeslice(ts) {};
    void write(const Data &d);
    Data read() { return read_internal(false); };
    Data get() { return read_internal(true); };

  private:
    Data read_internal(bool clear_ready);

    Data       _data;
    ptime_t    _timeslice;
  };

  // Queued channel class:  The class may store either an arbitrary number of
  // objects or a fixed number.  If a fixed number, a write will block if the
  // channel is full.  This is designed for multiple producers to feed data to
  // a single consumer.  If size is 0, then no max size exists.
  template <typename Data,typename Base = SingleConsumerChannel,
            typename Container = std::list<Data,traceable_allocator<Data> > >
  pTMutex class QueueChan : public Base, public MultiProducerChannel {
    typedef Container Store;
  public:
    typedef Data value_type;

    QueueChan(int size = 0) : _maxsize(size), _size(0) {};
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

  private:
    void check_size() const { assert(!_maxsize || _size <= _maxsize); }
    Data read_internal(bool clear_ready);

    unsigned   _maxsize;   // Max size.  If 0, no fixed size.
    unsigned   _size;      // Current size of queue.
    Store      _store;
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
    typedef Data value_type;

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
  template <typename Data,typename Base = SingleConsumerClockChannel,
            typename Container = std::list<std::pair<Data,ptime_t>,traceable_allocator<std::pair<Data,ptime_t> > > >
  pTMutex class ClockChan : Base, public MultiProducerChannel {
    typedef std::pair<Data,ptime_t> DP;
    typedef Container Store;
  public:
    typedef Data value_type;

    ClockChan(ptime_t p,ptime_t s = 0,int size = 1) : Base(p,s,size) {};
    void write(const Data &d);
    bool ready() const { return current_data() && Base::is_phi(); };

    void clear_ready() { pLock(); if (!empty()) { Base::decr_size(); _store.pop_back(); } pUnlock(); };
    Data read() { return read_internal(false); };
    Data get() { return read_internal(true); };

    using Base::maxsize;
    using Base::setMaxSize;
    using Base::full;
    using Base::size;
    using Base::empty;
    using Base::clear_notify;
    using Base::set_notify;

  private:
    bool current_data() const;
    Data read_internal(bool clear_ready);
    ptime_t curr_data_time() const { return _store.back().second; };
    ptime_t curr_time() const { return pTime(); };

    Store      _store;
  };

  //////////////////////////////////////////////////////////////////////////////
  //
  // Implementation.
  //
  //////////////////////////////////////////////////////////////////////////////

  /////////////// Channel ///////////////

  template <class Data,class Base>
  Data Channel<Data,Base>::read_internal(bool clearready)
  {
    // We'll be consuming data, so if we have a waiting
    // writer, it's valid to wake it up.
    if (have_writers() && clearready) {
      pAddReady(next_writer());
    }
    while (!ready()) {
      // We don't have data, so the reader must block until the writer adds
      // data.  This is in a loop in order to handle the multi-consumer
      // situation, where a consumer may be awakened, but finds that another
      // consumer already got to the data first.
      Base::set_notify(pCurThread(),HandleType());
      pSleep();
      Base::clear_notify();
    }
    if (clearready) {
      set_ready(false);
    }
    // Reactivate a waiting writer if one appeared while we were asleep.
    if (have_writers() && clearready) {
      pAddReady(next_writer());
    }    
    return _data;
  }

  template <class Data,class Base>
  void Channel<Data,Base>::write(const Data &d) 
  { 
    // If we have a waiting reader, wake it up.
    if (Base::have_reader()) {
      pWake(Base::notify_reader());
    }
    while (ready()) {
      // We already have data, so the write must block
      // until the reader consumes the data.
      set_writenotify(pCurThread());
      pSleep();
    }
    // Store data and set ready.
    _data = d;
    set_ready(true);
    // Reactivate a reader if one appeared while we were asleep.
    if (Base::have_reader()) {
      pWake(Base::notify_reader());
    }    
  };

  /////////////// BusyChan ///////////////

  template <class Data,class Base>
  Data BusyChan<Data,Base>::read_internal(bool clearready)
  {
    // We'll be consuming data, so if we have a waiting
    // writer, it's valid to wake it up.
    if (have_writers() && clearready) {
      pAddReady(next_writer());
    }
    if (!ready()) {
      // We don't have data, so the reader must
      // block until the writer adds data.
      Base::set_notify(pCurThread(),HandleType());
      pBusySleep(_timeslice);
    }
    if (clearready) {
      set_ready(false);
    }
    // Reactivate a waiting writer if one appeared while we were asleep.
    if (have_writers() && clearready) {
      pAddReady(next_writer());
    }    
    return _data;
  }

  template <class Data,class Base>
  void BusyChan<Data,Base>::write(const Data &d) 
  { 
    // If we have a waiting reader, wake it up.
    if (Base::have_reader()) {
      pBusyWake(Base::notify_reader());
    }
    while (ready()) {
      // We already have data, so the write must block
      // until the reader consumes the data.
      set_writenotify(pCurThread());
      pSleep();
    }
    // Store data and set ready.
    _data = d;
    set_ready(true);
    // Reactivate a reader if one appeared while we were asleep.
    if (Base::have_reader()) {
      pBusyWake(Base::notify_reader());
    }    
  };

  /////////////// QueueChan ///////////////

  template <typename Data,typename Base,typename Container>
  Data QueueChan<Data,Base,Container>::read_internal(bool clearready)
  {
    // If there's a waiting writer (queue is full) and we're
    // going to remove an item, then unblock the next writer here.
    if (have_writers() && clearready) {
      pAddReady(next_writer());
    }
    // If no data- sleep until we get some.
    if (!ready()) {
      Base::set_notify(pCurThread(),HandleType());
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

  template <typename Data,typename Base,typename Container>
  void QueueChan<Data,Base,Container>::write(const Data &d) 
  { 
    // If we have a waiting reader, wake it up.
    if (Base::have_reader()) {
      pWake(Base::notify_reader());
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
    if (Base::have_reader()) {
      pWake(Base::notify_reader());
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

  template <typename Data,typename Base,typename Container>
  bool ClockChan<Data,Base,Container>::current_data() const
  {
    return (!empty() && curr_data_time() < curr_time());
  }

  template <typename Data,typename Base,typename Container>
  Data ClockChan<Data,Base,Container>::read_internal(bool clearready)
  {
    // If there's a waiting writer (queue is full) and we're
    // going to remove an item, then unblock the next writer here.
    if (ready() && have_writers() && clearready) {
      pAddReady(next_writer());
    }
    // If no data- sleep until we get some.
    while (!ready()) {
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

  template <typename Data,typename Base,typename Container>
  void ClockChan<Data,Base,Container>::write(const Data &d) 
  { 
    // If we have a waiting reader, wake it up.
    if (Base::have_reader()) {
      Base::delayed_wakeup(current_data());
    }
    // Sleep if queue is full.  This is a loop so that if a waiting
    // writer is awakened and then another thread jumps in and writess
    // data to again fill the queue, the thread will again sleep.
    while (Base::full()) {
      set_writenotify(pCurThread());
      pSleep();
    }
    // Add data element w/time of next clock cycle.
    _store.push_front(std::make_pair(d,curr_time()));
    Base::incr_size();
    Base::check_size();
    // Reactivate a reader if one appeared while we were asleep.
    if (Base::have_reader()) {
      Base::delayed_wakeup(current_data());
    }
  };

}

#endif
