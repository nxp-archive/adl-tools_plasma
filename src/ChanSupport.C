//
// Various support classes for the channels.  Non-templated stuff should
// be placed in here.
//

#include "Interface.h"
#include "ChanSupport.h"

using namespace std;

namespace plasma {

  /////////////// Timeout ///////////////

  void timeout(void *a)
  {
    Timeout *to = (Timeout *)a;
    pDelay(to->delay());
    to->_ready = true;
    pWake(make_pair(to->reset(),to->_h));
  }

  // This will sleep if we're not ready and will clear
  // the ready state, "consuming" the timeout.
  int Timeout::get() 
  { 
    pLock();
    if (!ready()) {
      pUnlock();
      set_notify(pCurThread(),HandleType());
      pLock();
    }
    _ready = false;
    pUnlock();
    return 0; 
  };

  THandle Timeout::clear_notify()
  {
    if (_writet) {
      pTerminate(_writet);
      _writet = 0;
    }
    return reset();
  }

  THandle Timeout::reset() 
  { 
    THandle t = _readt; 
    _readt = 0;
    return t; 
  };

  void Timeout::set_notify(plasma::THandle t,plasma::HandleType h)
  {
    _ready = false;
    _readt = t;
    _h = h;
    assert(!_writet);
    _writet = pSpawn(timeout,this,-1);
  }

  /////////////// ClockChan ///////////////

  ClockChanImpl::ClockChanImpl(ptime_t p,ptime_t s,unsigned ms) : 
    _period(p), _skew(s), _maxsize(ms), _size(0)
  {}

  // Returns true if we're on a clock edge, given a clock period.
  bool ClockChanImpl::is_phi() const
  {
    return (pTime() % _period) == _skew;
  }

  // Returns the time of the next clock cycle.  We compute the time of the 
  // last edge, then add on the skew (or another cycle if no skew was specified)
  // to get the next edge.
  ptime_t ClockChanImpl::next_phi() const
  {
    return ( (((pTime()+_period)-_skew) / _period )*_period)+_skew;
  }

  //
  // Single-consumer support class.
  //

  SingleConsumerClockChannel::SingleConsumerClockChannel(ptime_t p,ptime_t s,unsigned ms) : 
    ClockChanImpl(p,s,ms), _waket(0), _delay(0), _readt(0)
  {}

  TPair SingleConsumerClockChannel::reset() 
  { 
    THandle t = _readt; 
    _readt = 0; 
    _waket = 0;
    return make_pair(t,_h);
  }

  void SingleConsumerClockChannel::set_notify(THandle t,HandleType h) 
  { 
    assert(!_readt); 
    _readt = t; 
    _h = h; 
    // If we have data, start the waker.  We don't need to check whether
    // the data is current b/c we wouldn't be here if it weren't.
    if (!empty()) {
      start_waker();
    }
  }

  void SingleConsumerClockChannel::cancel_waker()
  {
    if (_waket) {
      pTerminate(_waket);
      _waket = 0;
    }
  }

  void SingleConsumerClockChannel::clear_notify() 
  { 
    _readt = 0;
    cancel_waker(); 
  };

  void sc_delayed_waker(void *a)
  {
    SingleConsumerClockChannel *cc = (SingleConsumerClockChannel *)a;
    pDelay(cc->delay());
    pWake(cc->reset());
  }

  // Start a wake-up thread only if one doesn't already exist.
  void SingleConsumerClockChannel::start_waker()
  {
    if (!_waket) {
      _delay = next_phi() - pTime();
      assert(!_waket);
      _waket = pSpawn(sc_delayed_waker,this,-1);
    }
  }

  void SingleConsumerClockChannel::delayed_wakeup(bool current_data)
  {
    if (is_phi() && current_data) {
      // We're on a clock edge- wake up thread.
      // Cancel a waker thread if it exists.
      cancel_waker();
      pWake(reset());
    } else {
      // Not on a clock edge- schedule a thread to
      // wake up the reader at the correct time.
      start_waker();
    }
  }

  void SingleConsumerClockChannel::delayed_reader_wakeup(bool have_data)
  {
    // If we're not empty, and we're here, then it's because we're not on a clock
    // edge- in that case we'll start a waker thread.
    // If we are empty, then we're ready for notification when we do get data.
    set_notify(pCurThread(),HandleType());
    pSleep();
  }

  //
  // Multi-consumer support class.
  //

  MultiConsumerClockChannel::MultiConsumerClockChannel(ptime_t p,ptime_t s,unsigned ms) : 
    ClockChanImpl(p,s,ms)
  {}

  // Removes a consumer from the known set.
  TPair MultiConsumerClockChannel::reset(MClkInfo::iterator iter) 
  { 
    TPair t = iter->second._tinfo;
    _cons.erase(iter);
    return t;
  }

  // Adds a consumer to the known set and sets up a waker thread if
  // we have data.
  void MultiConsumerClockChannel::set_notify(THandle t,HandleType h) 
  { 
    pair<MClkInfo::iterator,bool> ip = _cons.insert(std::make_pair(t,MClk(t,h)));
    // If we have data, start the waker.  We don't need to check whether
    // the data is current b/c we wouldn't be here if it weren't.
    if (!empty()) {
      start_waker(ip.first);
    }
  }

  // This only cancels the waking thread if the current thread is
  // equal to the pending reader thread.
  void MultiConsumerClockChannel::clear_notify() 
  { 
    MClkInfo::iterator iter = _cons.find(pCurThread());
    cancel_waker(iter);
    _cons.erase(iter);
  }

  // Cancel a waker if the iterator is valid (not end()) and a waker
  // exists for the thread.  Note:  This does not remove the consumer.
  // You have to call reset to do that.
  void MultiConsumerClockChannel::cancel_waker(MClkInfo::iterator iter)
  {
    if (iter != _cons.end() && iter->second._waker) {
      pTerminate(iter->second._waker);
      iter->second._waker = 0;
    }
  }

  struct WakeArgs {
    int                        _delay;
    MClkInfo::iterator         _iter;
    MultiConsumerClockChannel *_mc;

    WakeArgs(MultiConsumerClockChannel *mc) : _mc(mc) {};
  };

  void mc_delayed_waker(void *a)
  {
    WakeArgs &args = *((WakeArgs*)a);
    MultiConsumerClockChannel &mc = *(args._mc);
    pDelay(args._delay);
    pWake(mc.reset(args._iter));
  }

  // Start a wake-up thread only if one doesn't already exist for the
  // specified thread.
  void MultiConsumerClockChannel::start_waker(MClkInfo::iterator iter)
  {
    if (iter != _cons.end() && !iter->second._waker) {
      WakeArgs args(this);
      args._delay = next_phi() - pTime();
      args._iter = iter;
      iter->second._waker = pSpawn(mc_delayed_waker,sizeof(WakeArgs),&args,-1).first;
    }
  }

  void MultiConsumerClockChannel::delayed_wakeup(bool current_data)
  {
    if (is_phi() && current_data) {
      // We're on a clock edge- wake up thread.
      // Cancel a waker thread if it exists.
      cancel_waker(_cons.begin());
      pWake(reset(_cons.begin()));
    } else {
      // Not on a clock edge- schedule a thread to
      // wake up the reader at the correct time.
      start_waker(_cons.begin());
    }
  }

  void MultiConsumerClockChannel::delayed_reader_wakeup(bool have_data)
  {
    // If we're not empty, and we're here, then it's because we're not on a clock
    // edge- in that case we'll start a waker thread.
    // If we are empty, then we're ready for notification when we do get data.
    set_notify(pCurThread(),HandleType());
    pSleep();
    clear_notify();
  }

}
