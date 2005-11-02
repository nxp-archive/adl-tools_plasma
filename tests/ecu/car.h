//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
//
// car.h:  Accelerator, etc..
//

#ifndef _CAR_H_
#define _CAR_H_

void accelerator(int32 simtime, Chan &acc_query, Chan &acc_val);
void starter(Chan &to_starter, Chan &started);

#endif
