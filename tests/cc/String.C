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

#include <iostream>

#include "gc/gc.h"

#include "String.h"
#include "Tokens.h"

using namespace std;

String::String(Tokens &tk) :
  _ptr((char*)tk._str.p),
  _len(tk._str.l)
{
}

ostream &operator<< (ostream &o,const String &s)
{
  o.write(s._ptr,s._len);
  return o;
}

String String::append(String x) const
{
  char *s = (char*)GC_malloc(_len+x._len);
  memcpy(s,_ptr,_len);
  memcpy(s+_len,x._ptr,x._len);
  return String(s,_len+x._len);
}
