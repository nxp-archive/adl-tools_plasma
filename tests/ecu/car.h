//
// car.h:  Accelerator, etc..
//

#ifndef _CAR_H_
#define _CAR_H_

void accelerator(int32 simtime, Chan &acc_query, Chan &acc_val);
void starter(Chan &to_starter, Chan &started);

#endif
