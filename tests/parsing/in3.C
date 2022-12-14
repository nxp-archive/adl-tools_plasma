//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//

define(reg foo1) {
  """
  Let the branch target effective address (BTEA) be calculated as follows:

  * For bcctrl[l], let BTEA be 32 Os concatencated with the contents of bits
    32:61 of the Count Register concatenated with 0b00.

  The BO field of the instruction specified the condition or conditions that
  must be met in order for the branch to be taken, as defined in `Branch
  Instructions`_.  The sum BI+32 specified the bit of the Condition Register
  that is to be used.
  """;

  i = 10;
  size = 10;
  define(stuff bar) {
    size = 20;
  }
}
