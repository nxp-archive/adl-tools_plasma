//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//



define(reg foo1) {
  size = 10;
  define(stuff bar) {
    size = 20;
  }
}

// Example of a helper function defined inline with the rest of of the ADL spec.
// Note that this is at the top-leve, outside of any define or defmodify.
template <size_t Size>
bitset<Size> signExtend(bitset<Size> a,int signbit)
{
  assert (signbit < a.size());
  if (a.test(signbit)) {
    for (int i = a.size()-1; i >= signbit; --i) {
      a.set(i);
    }
  }
  return a;
}

define(reg foo2) {
  size = 30;
  define(stuff bar) {
    size = 20;
  }
}

define(reg foo3) {
  size = 30;
  define(stuff bar) {
    size = 20;
  }
}

