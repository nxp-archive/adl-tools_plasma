//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//



define(reg foo1) {
  size = 10;
  write = func (intbv<20> v,unsigned q) {
    size = 20;
  };
}
