//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//


// This makes sure that we don't consider '#' characters in strings when trying to
// calculate file locations for error messages.
define(reg foo1) {
  i = 10;
  size = 10;
  define(stuff bar) {
    " This '#1' should be ignored.";
    foo+-;
  }
}
