//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//

void outer {

  int i = 2;
  i = i + 20;

  int k = func(int j) {
    int j = 10;
    int k = 20;
    inner(i);
    define (reg foo) {
      readhook = func() {
        int j = 10;
      };
    }
  };

  void stuff();

  inner(i);

}

