//
// This class wraps all public member functions with the code
// generated by MakeWrapperBody.  It is derived from the WrapperClass
// example of OpenC++.
//

#ifndef _WRAPPER_H_
#define _WRAPPER_H_

#include <utility>

#include "opencxx/mop2.h"

using namespace Opencxx;

// Wraps public member functions.  Derive from this and override
// MakeWrapperBody() to generate the desired wrapper code.  It 
// returns a pair, where the first element is the invocation of the
// original function and the second element is the return statement,
// if there is one.
class Wrapper : public TemplateClass {
public:
  void TranslateClass(Environment* env);
  void TranslateMemberFunction(Environment* env, Member& member);
  
  // This is overridden so that the default is to not handle templates.
  // However, since we derive from the correct class, you can override
  // this function in order to handle templates.
  virtual bool AcceptTemplate() { return false; };
protected:
  // Overload this to change the name used for the original method.
  // The default is to prepend "_orig_".
  virtual Ptree* newMemberName(Ptree* name);
  // Overload this to define what to wrap.  By default all public
  // methods other than constructors and the destructor are wrapped.
  virtual bool wrapMember(Environment *env,Member &member) const;
  // Overload this to define the wrapper function.  Return (nil,nil) on error.
  virtual std::pair<Ptree*,Ptree*> makeWrapperBody(Environment *env,
                                                                     Member& member, Ptree* org_name,
                                                                     Ptree *variadic);

private:
  void setupOrigMember(Environment *env,Member &member,Ptree *name);
  void makeWrapper(Environment *env,Member& member, Ptree* org_name,Ptree *variadic);
  Ptree *exprToCallOriginal(Environment *env,Member& member, Ptree* org_name);
  Ptree *isVariadic(Environment *env,Member &member);
  Ptree *variadicArgs(Ptree *args);
};

#endif
