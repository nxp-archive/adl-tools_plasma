//
// Mutex wrapper class:  Any class preceded by pMutex will
// have all public member functions wrapped with mutex code.
// A public member may be preceded by the pNoMutex modifier
// to disable the generation of mutex code.
//

#include "Mutex.h"

using namespace std;

bool Mutex::Initialize()
{
  RegisterMetaclass("pMutex","Mutex");
  RegisterNewMemberModifier(NoMutexStr);
  return true;
}

bool isNoMutex(Ptree *mlist)
{
  while (mlist != nil) {
    if (mlist->Eq(NoMutexStr)) {
      return true;
    }
    mlist = mlist->Car();
  }
  return false;
}

bool Mutex::wrapMember(Environment *env,Member &member) const
{
  return (Wrapper::wrapMember(env,member) && 
          !isNoMutex(member.GetUserMemberModifier()));
}

// I write std::pair here b/c OpenC++ gets confused if I don't- it doesn't
// understand that the declaration is the same as the definition b/c of the
// "using" statement above.
std::pair<Ptree*,Ptree*> Mutex::makeWrapperBody(Environment *env,Member& member, Ptree* name,Ptree *variadic)
{
    pair<Ptree*,Ptree*> body = Wrapper::makeWrapperBody(env, member, name, variadic);
    return make_pair(Ptree::qMake("try {\n"
                                  "  plasma::pLock();\n"
                                  "  `body.first`\n"
                                  "  plasma::pUnlock();\n"),
                     Ptree::qMake("`body.second`\n"
                                  "}\n"
                                  "catch (...) {\n"
                                  "  pUnlock();\n"
                                  "  throw;\n"
                                  "}\n"));
}

