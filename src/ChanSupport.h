//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
//
// Various support classes for the channels.  Non-templated stuff should
// be placed in here.
//

#ifndef _CHANSUPPORT_H_
#define _CHANSUPPORT_H_

#include <deque>
#include <assert.h>
#include <ext/hash_set>
#include <ext/hash_map>

namespace plasma {

  // Base class for channels that allow multiple producers.  An actual
  // channel should inherit from this class and implement write, read, and get.
  class MultiProducerChannel {
    typedef std::deque<THandle,traceable_allocator<THandle> > Writers;
  public:

    bool multiple_producers_allowed() const { return true; };
  protected:
    void set_writenotify(THandle t) { _writers.push_back(t); };
    void push_writer(THandle t) { _writers.push_front(t); };
    bool have_writers() const { return !_writers.empty(); };
    THandle next_writer() { THandle t = _writers.front(); _writers.pop_front(); return t; };

    Writers    _writers;
  };

  // Base class for a simple channel that allows only one consumer but
  // multiple producers.
  class SingleConsumerChannel {
  public:
    SingleConsumerChannel() : _readt(0) {};

    // These are marked as non-mutex b/c they are used by alt, which already
    // does the locking.
    void set_notify(THandle t) { assert(!_readt); _readt = t; };
    void clear_notify() { _readt = 0; };

    bool multiple_consumers_allowed() const { return false; };
  protected:
    // Do we have a waiting reader?
    bool have_reader() const { return _readt; };
    THandle notify_reader() { THandle t = _readt; _readt = 0; return t; };

  private:
    THandle    _readt;
  };

  struct HashTHandle {
    size_t operator()( THandle t ) const { return (size_t)t; };
  };

  typedef __gnu_cxx::hash_set<THandle,HashTHandle> ConInfo;

  // Base class for a simple channel that allows multiple consumers and multiple
  // producers.  Note:  The order in which multiple consumers will be satisfied
  // is non-deterministic.
  class MultiConsumerChannel {

  public:
    MultiConsumerChannel(){};

    // These are marked as non-mutex b/c they are used by alt, which already
    // does the locking.
    void set_notify(THandle t) { _cons.insert(t); };
    // This clears the notification for the current thread- should only be called by
    // reader threads.
    void clear_notify() { _cons.erase(_cons.find(pCurThread())); };

    bool multiple_consumers_allowed() const { return true; };
  protected:
    // Do we have a waiting reader?
    bool have_reader() const { return !_cons.empty(); };
    // This gets the next reader in a non-deterministic fashion.
    THandle notify_reader() { THandle t = *(_cons.begin()); _cons.erase(_cons.begin()); return t; };

  private:
    ConInfo _cons;
  };

  // Base class for single-data item channels.  This must be first on the list
  // of inherited classes b/c we want the this pointer to be the same as the
  // main class's.
  class SingleDataChannelBase : public MultiProducerChannel {
  public:
    SingleDataChannelBase() : _source_channel(this), _ready(false) {};
    SingleDataChannelBase(const SingleDataChannelBase &x) :
      _source_channel(this),
      _ready(x._ready)
    {}    
    bool ready() const { return _ready; };
    bool full() const { return _ready; };
    void clear_ready() { pLock(); _ready = false; pUnlock(); };

    void *get_source_channel() const { return _source_channel; };
    void set_source_channel(void *sc) { _source_channel = sc; };
  protected:
    void set_ready(bool r) { _ready = r; };

  private:
    void      *_source_channel;  // For tracking connectivity.

    bool       _ready;   // Data is ready flag.
  };

  //
  // Timeout channel:  When used in an alt block, will set ready after a specified
  // amount of simulation time.
  //
  class Timeout {
  public:
    Timeout(ptime_t d) : _ready(false), _delay(d), _readt(0), _writet(0) {};

    ptime_t delay() const { return _delay; };
    void setDelay(ptime_t d) { _delay = d; };

    bool ready() const { return _ready; };
    int get();

    // These are marked as non-mutex b/c they are used by alt, which already
    // does the locking.
    void set_notify(THandle t);
    THandle clear_notify();

    bool multiple_producers_allowed() const { return false; };
    bool multiple_consumers_allowed() const { return false; };    
    void *get_source_channel() const { return 0; };
    void set_source_channel(void *) { };
  private:
    friend void timeout(void *a);
    THandle reset();

    bool       _ready;
    ptime_t    _delay;
    THandle    _readt;
    THandle    _writet;
  };

  // Non-templated implementation class used by ClockChan- do not use this
  // directly.
  class ClockChanImpl {
  public:
    ClockChanImpl(ptime_t p,ptime_t s,unsigned maxsize);
    ClockChanImpl(const ClockChanImpl &);

    bool is_phi() const;
    ptime_t next_phi() const;

    bool empty() const { return _size == 0; };
    unsigned size() const { return _size; };

    unsigned period() const{ return _period; };

    unsigned maxsize() const { return _maxsize; };
    void setMaxSize(unsigned ms) { _maxsize = ms; }
    // We're full if we're >= the max size.
    bool full() const { return _size >= allowed_size(); };

    bool interlocked() const { return (_maxsize == 0); };

    void set_size(unsigned s) { _size = s; };
    void incr_size() { ++_size; };
    void decr_size() { --_size; };
    void check_size() const { assert(_maxsize <= 0 || _size <= _maxsize); }

    void *get_source_channel() const { return _source_channel; };
    void set_source_channel(void *sc) { _source_channel = sc; };    
  private:
    unsigned allowed_size() const { return (!_maxsize) ? 1 : _maxsize; };

    ptime_t    _period;    // Clock period.
    ptime_t    _skew;      // Skew- offset from clock edge.
    unsigned   _maxsize;   // Max size.  If -1, no fixed size.  0 means a writer blocks
                           // until a reader consumes the data.
    unsigned   _size;      // Number of items in channel.

    void      *_source_channel;    // For connectivity tracking.
  };

  // Base class used for single-consumer clocked channels.
  class SingleConsumerClockChannel : public ClockChanImpl {
  public:
    SingleConsumerClockChannel(ptime_t p,ptime_t s,unsigned maxsize);

    bool have_reader() const { return _readt; };
    void set_notify(THandle t);
    void clear_notify();
    void delayed_wakeup(bool current_data);
    void delayed_reader_wakeup();
    void start_waker();
    THandle reset();
    void cancel_waker();
    int delay() const { return _delay; };

    bool multiple_consumers_allowed() const { return false; };    
  private:
    THandle    _waket;     // Thread which wakes reader at the correct time.
    int        _delay;     // Stores delay time for waker.
    THandle    _readt;     // Read thread.
  };

  struct MClk {
    THandle    _tinfo;
    THandle    _waker;

    MClk(THandle t) : _tinfo(t), _waker(0) {};
  };
  typedef __gnu_cxx::hash_map<THandle,MClk,HashTHandle> MClkInfo;

  // Base class used for multi-consumer clocked channels.
  // If _broadcast is true, we will deliver a wakeup to all
  // registered consumers, rather than just one.
  class MultiConsumerClockChannel : public ClockChanImpl {
  public:
    MultiConsumerClockChannel(ptime_t p,ptime_t s,unsigned maxsize);

    void set_broadcast() { _broadcast = true; };
    bool have_reader() const { return !_cons.empty(); };
    void set_notify(THandle t);
    void clear_notify();
    void cancel_waker(MClkInfo::iterator iter);
    // This wakes up the next available reader.
    void delayed_wakeup(bool current_data);
    void delayed_reader_wakeup();
    void start_waker(MClkInfo::iterator);
    THandle reset(MClkInfo::iterator);

    bool multiple_consumers_allowed() const { return true; };    
  private:
    bool        _broadcast;
    MClkInfo    _cons;
  };

}

#endif
