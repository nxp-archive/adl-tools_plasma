//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
//
// Very simple string class- a string is a pointer
// and a length.  This allows one of these classes
// to point to data in a read-only mmapped file.
//

#ifndef _CC_STRING_H_
#define _CC_STRING_H_

#include <cstring>
#include <assert.h>
#include <iosfwd>

struct Tokens;

class String {
public:
  String() : _ptr(0), _len(0) {};
  String(const char *p,int n) : _ptr(p), _len(n) {};
  String(Tokens &tk);
  String(const String &x) : _ptr(x._ptr), _len(x._len) {};
  // This will allocate a new string of the given size
  // using the garbage collector.
  String(int n);

  bool operator==(String x) const;
  bool operator!=(String x) const;

  // This will allocate a new string, containing contents
  // + the argument string.  The original contents are
  // not modified.
  String append(String x) const; 

  bool empty() const { return !_len; };
  unsigned size() const { return _len; };
  const char *data() const { return _ptr; };
  char operator[](unsigned n) { assert(n < _len); return _ptr[n]; };

  friend std::ostream &operator<< (std::ostream &,const String &);
private:
  friend struct StringHash;

  const char *_ptr;
  unsigned _len;
};

struct StringHash {
  size_t operator()(String x) const {
    unsigned long __h = 0;
    for (unsigned i = 0; i != x._len; ++i)
      __h = 5*__h + x._ptr[i];
    return size_t(__h);
  };
};

inline bool String::operator==(String x) const
{
  if (_len != x._len) {
    return false;
  } else {
      return !std::strncmp(_ptr,x._ptr,_len);
  }
}

inline bool String::operator!=(String x) const
{
  return !(*this == x);
}

#endif
