
#include "Wrapper.h"

using namespace std;

void Wrapper::TranslateClass(Environment* env)
{
    Member member;
    int i = 0;
    while(NthMember(i++, member)) {
      if(member.IsFunction() && wrapMember(env,member)){
        Member wrapper = member;
        Ptree* org_name = newMemberName(member.Name());
        member.SetName(org_name);
        ChangeMember(member);
        makeWrapper(env, wrapper, org_name);
        AppendMember(wrapper, Class::Public);
      }
    }
}

bool Wrapper::wrapMember(Environment *env,Member &member) const
{
  return (member.IsPublic() && !member.IsConstructor() && !member.IsDestructor());
}

void Wrapper::TranslateMemberFunction(Environment* env, Member& member)
{
  if(wrapMember(env,member)) {
    member.SetName(newMemberName(member.Name()));
  }
}

Ptree* Wrapper::newMemberName(Ptree* name)
{
    return Ptree::qMake("_orig_`name`");
}

void Wrapper::makeWrapper(Environment *env,Member& member, Ptree* orig_name)
{
  pair<Ptree*,Ptree*> body = makeWrapperBody(env, member, orig_name);
  member.SetFunctionBody(Ptree::qMake("{ `body.first` `body.second`}\n"));
}

pair<Ptree*,Ptree*> Wrapper::makeWrapperBody(Environment *env,Member& member, Ptree* orig_name)
{
  TypeInfo t;

  Ptree *call_stmt = nil, *ret_stmt = nil;
  Ptree *call_expr = exprToCallOriginal(env, member, orig_name);
  if (!call_expr) {
    return make_pair((Ptree*)nil,(Ptree*)nil);
  }
  member.Signature(t);
  t.Dereference();		// get the return type

  if((t.IsBuiltInType() & VoidType) || t.IsNoReturnType()) {
    call_stmt = Ptree::qMake("`call_expr`;\n");
  } else {
    Ptree* rvar = Ptree::Make("_rvalue");
    Ptree* rvar_decl = t.MakePtree(rvar);
    call_stmt = Ptree::qMake("`rvar_decl` = `call_expr`;\n");
    ret_stmt = Ptree::qMake("return `rvar`;\n");
  }
  return make_pair(call_stmt,ret_stmt);
}

// This checks to see if the function has any elipsis.  If so, it substitutes
// a name of "ap" for "..." and it returns as a second element the first argument
// (which is usually a format specifier argument).  Otherwise, the second element is nil.
Ptree *Wrapper::exprToCallOriginal(Environment *env,Member& member, Ptree* orig_name)
{
  Ptree *args = member.Arguments();
  Ptree *last = args->Last();
  if (last && last->Car()->Eq("...")) {
    ErrorMessage(env,"Wrapping of varargs functions is not supported.",nil,args);
    return nil;
  }
  return Ptree::qMake("`orig_name`(`member.Arguments()`)");
}
