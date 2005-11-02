//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
//
// Miscellaneous common types and info.
//

#ifndef _TYPES_H_
#define _TYPES_H_

#include <vector>
#include <string>

// Size of the 'int' type.
const unsigned IntSize = 4;

// Size of the 'char' type.
const unsigned CharSize = 1;

// The machine's word size.  Note that making this different
// from INT_SIZE may cause serious problems.
const unsigned WordSize = 4;

// Not really used, but it's here.
const unsigned DoubleSize = 8;

// This is a strange multiplier that needs to be used in the allocation
// of global variables for the GNU Assembler.  Not sure exactly what it
// represents.
const unsigned WeirdMultiplier = 4;

typedef std::vector<int> IntVect;

#endif

