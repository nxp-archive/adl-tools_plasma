//@beginlicenses@
//@license{Stefan Seefeld}{2004}@
//
//  Permission to use, copy, distribute and modify this software and its  
//  documentation for any purpose is hereby granted without fee, provided that
//  the above copyright notice appears in all copies and that both that copyright
//  notice and this permission notice appear in supporting documentation.
// 
//  Stefan Seefeld make(s) no representations about the suitability of this
//  software for any purpose. It is provided "as is" without express or implied
//  warranty.
//  
//  Copyright (C) 2004 Stefan Seefeld
//
//@endlicenses@

#ifndef guard_opencxx_parser_GC_h
#define guard_opencxx_parser_GC_h

#include <cstdlib>
#include <gc/gc_cpp.h>
#include <gc/gc_allocator.h>

namespace Opencxx
{

typedef gc LightObject;
typedef gc_cleanup Object;

inline
int GcCount()
{
# ifdef GC_DISABLED
  return 0;
# else
  return GC_gc_no;
# endif
}

}

#endif
