//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
//
// Various channels used for communication.
//

#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_

#include "plasma.h"
#include "Tokens.h"

struct TokChan : public plasma::Channel<Tokens> {};

#endif

