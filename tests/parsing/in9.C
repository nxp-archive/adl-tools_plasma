//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//

// This tests our ability to have a nested template functions.

define(reg foo2) {

  template <typename T> 
    void A(T a,T b)
    {
      ;
    }

}

