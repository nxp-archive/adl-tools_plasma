
#include "Wrapper.h"
#include "LdLeaf.h"

using namespace std;

void Wrapper::TranslateClass(Environment* env)
{
    Member member;
    int i = 0;
    while(NthMember(i++, member)) {
      if(member.IsFunction() && wrapMember(env,member)){
        Ptree* org_name = newMemberName(member.Name());
        // If function takes a va-list, create variadic wrapper, too.
        // This isn't perfect, but it gets around the issue of dealing
        // directly with variadic functions- we'd have to go through the
        // body and convert it to handle a va_list.
        if (Ptree *lastfixed = isVariadic(env,member)) {
          Member vwrapper = member;
          vwrapper.SetArgumentList(variadicArgs(member.ArgumentList()));
          makeWrapper(env, vwrapper, org_name, lastfixed);
          AppendMember(vwrapper, Class::Public);
        }
        // Normal creation code.
        Member wrapper = member;
        setupOrigMember(env,member,org_name);
        ChangeMember(member);
        makeWrapper(env, wrapper, org_name, nil);
        AppendMember(wrapper, Class::Public);
      }
    }
}

void Wrapper::setupOrigMember(Environment *env,Member &member,Ptree *org_name)
{
  Ptree *ofb = member.FunctionBody();
  member.SetName(org_name);
  // We only insert the line directive if there is a function body- there might
  // not be one if we're processing a declaration and this isn't an inlined function.
  if (ofb) {
    member.SetFunctionBody(Ptree::List(lineDirective(env,ofb),ofb));
  }
}

// If the function is variadic (has ...) then we can't handle it and
// produce an error.  If it takes a va_list as a last argument, we return
// the next to last argument.
Ptree *Wrapper::isVariadic(Environment *env,Member &member)
{
  Ptree *args = member.ArgumentList();
  Ptree *last = args->Last();
  if (last && last->Car()->Eq("...")) {
    // Can't handle variadic functions directly.
    ErrorMessage(env,"Variadic members may not be directly wrapped.\n"
                 "Instead, create a method which takes a va_list-\n"
                 "both variadic and va_list wrappers will be created.\n",nil,member.Name());
    return nil;
  } else if (last && last->Ca_ar()->Eq("va_list")) {
    // Function takes a va_list, so we can create a variadic wrapper, as well
    // as the va_list version.
    Ptree *a = member.Arguments();
    int length = a->Length();
    if (length < 3) {
      ErrorMessage("Found a varargs function with no initial non-ellipses parameter.",nil,member.Name());
      return nil;
    }
    return a->Nth(length-3);
  }
  // Not variadic at all.
  return nil;
}

// Return arguments w/last argument replaced by "...";
Ptree *Wrapper::variadicArgs(Ptree *args)
{
  if (!args) {
    return nil;
  } else if (!args->Cdr()) {
    return Ptree::List(Ptree::Make("..."));
  } else {
    return Ptree::List(args->Car(),variadicArgs(args->Cdr()));
  }
}

bool Wrapper::wrapMember(Environment *env,Member &member) const
{
  return (member.IsPublic() && !member.IsConstructor() && !member.IsDestructor());
}

void Wrapper::TranslateMemberFunction(Environment* env, Member& member)
{
  if(wrapMember(env,member)) {
    setupOrigMember(env,member,newMemberName(member.Name()));
  }
}

Ptree* Wrapper::newMemberName(Ptree* name)
{
    return Ptree::qMake("_orig_`name`");
}

void Wrapper::makeWrapper(Environment *env,Member& member, Ptree* orig_name,Ptree *variadic)
{
  pair<Ptree*,Ptree*> body = makeWrapperBody(env, member, orig_name,variadic);
  member.SetFunctionBody(Ptree::qMake("{ `body.first` `body.second`}\n"));
}

pair<Ptree*,Ptree*> Wrapper::makeWrapperBody(Environment *env,Member& member, 
                                             Ptree* orig_name, Ptree *variadic)
{
  TypeInfo t;

  Ptree *call_stmt = nil, *ret_stmt = nil;
  Ptree *call_expr = exprToCallOriginal(env, member, orig_name);
  if (!call_expr) {
    return make_pair((Ptree*)nil,(Ptree*)nil);
  }

  // Get the return type.
  member.Signature(t);
  t.Dereference();

  if((t.IsBuiltInType() & VoidType) || t.IsNoReturnType()) {
    call_stmt = Ptree::qMake("`call_expr`;\n");
  } else {
    Ptree* rvar = Ptree::Make("_rvalue");
    Ptree* rvar_decl = t.MakePtree(rvar);
    call_stmt = Ptree::qMake("`rvar_decl` = `call_expr`;\n");
    ret_stmt = Ptree::qMake("return `rvar`;\n");
  }

  // If set, create variadic handling code.
  if (variadic) {
    Ptree *orig_stmt = call_stmt;
    call_stmt = Ptree::qMake("va_list ap;\n"
                                    "__builtin_va_start(ap,`variadic`);\n"
                                    "`orig_stmt`"
                                    "__builtin_va_end(ap);\n");    
  }

  return make_pair(call_stmt,ret_stmt);
}

Ptree *Wrapper::exprToCallOriginal(Environment *env,Member& member, Ptree* orig_name)
{
  return Ptree::qMake("`orig_name`(`member.Arguments()`)");
}
