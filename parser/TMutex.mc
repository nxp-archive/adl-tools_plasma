//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
//
// Mutex wrapper class for templates.  Unfortunately, OpenC++
// doesn't allow a metaclass to handle both a template and a
// non-template at once.  So, this handles templates and is just
// a thin wrapper around the non-template version.
//

#include "Mutex.h"

class TMutex : public Mutex {
public:
  static bool Initialize();
  virtual bool AcceptTemplate();
};

bool TMutex::AcceptTemplate() 
{ 
  return true; 
}

bool TMutex::Initialize()
{
  RegisterMetaclass("pTMutex","TMutex");
  RegisterNewMemberModifier(NoMutexStr);
  return true;
}
