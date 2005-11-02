//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
//
// Quantity manager class: Use this to model a resource.  Various threads can
// make requests of the resource and will be busy until the request is
// satisfied.  You instantiate this class with a timeslice parameter; this sets
// the length of one request.  In another words, a quantity with a value of 10
// means that 1 request would take 10 time units.  For example, if modeling a
// bus, a read might take 10 clock cycles.
// 
// Once instantiated, a thread calls request with an amount.  At a minimum, this
// will be satisifed in amount*timeslice time units.  The thread's processor is
// busy for this time, but is timesliced by the timeslice of the quantity.  If
// there is more than one thread making a request, then the quantity class
// iterates over the requesting threads, decrementing their remaining amounts in
// a round-robin fashion.
//

#ifndef _QUANTITY_H_
#define _QUANTITY_H_

#include "plasma.h"

namespace plasma {

  class Quantity {
  public:
    Quantity(int ts);

    // Request 'amount' of the resource.
    void request(int amount);

    friend void work(Quantity *);
  private:
    typedef BusyChan<int> RetChan;
    // Note:  List is garbage collected so that you can erase from
    // the list and the iterator is still valid.
    typedef std::list<std::pair<RetChan,int>,gc_allocator<std::pair<RetChan,int> > > Returns;

    Returns::iterator register_request(int);

    Channel<Returns::iterator> _request;
    Returns _returns;

    ptime_t _ts;
    Processor _p;
  };

  inline Quantity::Quantity(int ts) : _ts(ts) 
  { 
    // Can't use the spawn operator b/c of namespace issues.
    // Fix this when OpenC++ improves.
    pSpawn(_p(),(void(*)(void *))work,(void *)this,-1);
  }

  inline void Quantity::request(int amount)
  {
    Returns::iterator iter = register_request(amount);
    _request.write(iter);
    iter->first.get();
  }

  inline Quantity::Returns::iterator Quantity::register_request(int amount)
  {
    _returns.push_front(std::make_pair(RetChan(_ts),amount));
    return _returns.begin();
  }

  inline void work(Quantity *q)
  {
    while (true) {
      // Block if nothing to do.
      Quantity::Returns::iterator iter = q->_request.get();
      // Now we loop, slowly using up the required quantities.
      while (true) {
        pDelay(q->_ts);
        // Find next item to service.
        do {
          ++iter;
          if (iter == q->_returns.end()) {
            iter = q->_returns.begin();
          }
        } while (!iter->second);
        if (!(--iter->second)) {
          // Done with this item:  Wake it up.
          iter->first.write(0);
          // Remove from list- the receiver's copy of
          // the iterator is still valid b/c of gc.
          q->_returns.erase(iter);
          // If nothing in list, exit from this loop so that
          // we'll sleep until something comes along.
          if (q->_returns.empty()) {
            break;
          } else {
            // Else- start at beginning b/c we can't increment the iterator.
            iter = q->_returns.begin();
          }
        }
        // Check for any new requests.
        alt {
          q->_request.port() { ; }
          { ; }
        }
      }
    }
  }
  
}

#endif
