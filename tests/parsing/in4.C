//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//

define (stuff bar) {
  readhook = {
    int i = 1;
  };
  writehook = func(intbv<32> val) {
    int i = 1;
  };
}
