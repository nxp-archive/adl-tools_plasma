//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
//
// engine.h:  Engine model.
//

#ifndef _ENGINE_H_
#define _ENGINE_H_

void engine(FuelChans &inject, Chans &spark);
void flywheel(Chan &from_flywheel, Chan &to_starter);

#endif
