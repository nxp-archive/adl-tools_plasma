//
// Various channels used for communication.
//

#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_

#include "plasma.h"
#include "Tokens.h"

struct TokChan : public plasma::Channel<Tokens> {};

#endif
