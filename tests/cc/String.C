//
// Very simple string class- a string is a pointer
// and a length.  This allows one of these classes
// to point to data in a read-only mmapped file.
//

#include <iostream>

#include "String.h"
#include "Tokens.h"

using namespace std;

String::String(Tokens &tk) :
  _ptr((char*)tk._str.p),
  _len(tk._str.l)
{
  assert(tk._tok == IDENTIFIER || tk._tok == STRING_LITERAL);
}

ostream &operator<< (ostream &o,const String &s)
{
  o.write(s._ptr,s._len);
  return o;
}
