//
// Very simple string class- a string is a pointer
// and a length.  This allows one of these classes
// to point to data in a read-only mmapped file.
//

#ifndef _STRING_H_
#define _STRING_H_

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
  const char *_ptr;
  unsigned _len;
};

#endif
