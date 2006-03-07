//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//

// This tests our ability to have a nested function.

define(reg foo1) {
  size = 10;
  define(stuff bar) {
    size = 20;
  }
}

define(reg foo2) {

  void A(int a,int signbit)
    {
      ;
    }

  uint32 B(int a,int signbit)
    {
      ;
    }

  bits<32> C(int a,int signbit)
    {
      ;
    }

  const bits<32> D(int a,int signbit)
    {
      ;
    }

  uint32 E(bits<32> field,bits<32> x,bits<32> y)
    {
      int i = 1;
    }

  void F(bits<32> field,bits<32> x,bits<32> y)
    {
      int i = 1;
    }

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

