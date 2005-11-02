//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
//
// Mutex wrapper class:  Any class preceded by pMutex will
// have all public member functions wrapped with mutex code.
// A public member may be preceded by the pNoMutex modifier
// to disable the generation of mutex code.
//

#ifndef _MUTEX_H_
#define _MUTEX_H_

#include "Wrapper.h"

#define NoMutexStr "pNoMutex"

// Wraps public member functions with mutex code that are not constructors
// or a destructor and do not have pNoMutex as a member modifier.
class Mutex : public Wrapper {
public:
  static bool Initialize();
protected:
  virtual std::pair<Opencxx::Ptree*,Opencxx::Ptree*> makeWrapperBody(Opencxx::Environment *env,
                                                                     Opencxx::Member& member, 
                                                                     Opencxx::Ptree* name,
                                                                     Opencxx::Ptree *variadic);
  virtual bool wrapMember(Opencxx::Environment *env,Opencxx::Member &member) const;
};

#endif
