//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
//
// angle.pa
//

#ifndef _ANGLE_H_
#define _ANGLE_H_

int32 curr_angle(int32 time);
void angle_manager(int32 num_cylinders, Chan &teeth, Chans &ask, Chans &wakeups);

#endif

