
#include <assert.h>
#include <iostream>
#include <vector>
#include "opencxx/mop.h"
#include "opencxx/ptree-core.h"

#include "VarWalker.h"
#include "Plasma.h"

using namespace std;

bool Plasma::Initialize()
{
  SetMetaclassForPlain("Plasma");
  RegisterNewBlockStatement("par");
  RegisterNewForStatement("pfor");
  RegisterNewBlockStatement("alt");
  RegisterNewForStatement("afor");
  RegisterNewForStatement("port");
  RegisterMetaclass("pSpawner","Plasma");
  return TRUE;
}

// The argument list was created in reverse order, so that's why
// we iterate through it in reverse.
string argList(const ArgVect &av)
{
  string s;
  for(ArgVect::const_reverse_iterator i = av.rbegin(); i != av.rend(); ++i){
    if (!s.empty()) {
      s += ",";
    }
    if (!i->_ref) {
      s += "&";
    }
    s += i->_arg->ToString();
  }
  return s;
}

// Given a list and an object, appends the object to the list and returns
// a pointer to the new end-of-list object.
inline Ptree *lappend(Ptree *l,Ptree *x)
{
  return Ptree::Last(Ptree::Snoc(l,x));
}

// This figures out what kind of user statement we have and dispatches to the
// appropriate function.
Ptree* Plasma::TranslateUserPlain(Environment* env,Ptree* keyword, Ptree* rest)
{
  if (keyword->Eq("par")) {
    return TranslatePar(env,keyword,rest);
  }
  else if (keyword->Eq("pfor")) {
    return TranslatePfor(env,keyword,rest);
  }
  else if (keyword->Eq("alt")) {
    return TranslateAlt(env,keyword,rest);
  }
  else if (keyword->Eq("afor")) {
    return TranslateAfor(env,keyword,rest);
  }

  ErrorMessage(env, "unknown user statement encountered",nil,keyword);
  return nil;
}

// This translates plain par statements.  The semantics are that each expression
// is launched as a separate thread.  All threads must finish before the par block
// is completed and execution continues.
Ptree* Plasma::TranslatePar(Environment* env,Ptree* keyword, Ptree* rest)
{
  Ptree *body;

  if(!Ptree::Match(rest, "[ %? ]", &body)){
    ErrorMessage(env, "invalid par statement", nil, keyword);
    return nil;
  }

  // Ignore the braces.
  body = body->Cadr();

  Ptree *start = Ptree::List(Ptree::qMake("//Begin par block\n"));
  Ptree *cur = start;

  // This walker will be used to walk each expression that's an element of
  // the par block.  It just records what variables are used that occur in the
  // parent scope.
  VarWalker *vw = new VarWalker(env->GetWalker()->GetParser(),env);

  Ptree *thnames = nil;

  // Iterate over constituent statements, creating spawn statements.
  for(PtreeIter i = body; !i.Empty(); ++i) {
    vw->reset();
    convertToThread(cur,thnames,*i,vw,env,false);
  }

  // Now iterate over created threads, creating wait statements.
  for (PtreeIter i = thnames; !i.Empty(); ++i) {
    cur = lappend(cur,Ptree::qMake("pWait(`*i`);\n"));
  }

  cur->SetCdr(Ptree::List(Ptree::Make("// End par block.\n")));

  return start;
}

// Translates a pfor statement.  The semantics of this are that the body is
// dispatched as a thread for each loop iteration.
Ptree* Plasma::TranslatePfor(Environment* env,Ptree* keyword, Ptree* rest)
{
  Ptree *s1, *s2, *s3, *body;
  if(!Ptree::Match(rest, "[ ( %? %? ; %? ) %? ]", &s1,&s2,&s3,&body)) {
    ErrorMessage(env, "invalid pfor statement", nil, keyword);
    return nil;
  }

  // This walker will be used to walk each expression that's an element of
  // the par block.  It just records what variables are used that occur in the
  // parent scope.
  VarWalker *vw = new VarWalker(env->GetWalker()->GetParser(),env);

  Ptree *thnames = nil;

  Ptree *start = Ptree::List(Ptree::qMake("//Begin pfor block\n"));
  Ptree *cur = start;

  // Handling the guard means that we want to record any declarations and
  // we want to record as call-by-value any variables mentioned here.
  vw->handleGuard(true);
  vw->Translate(s1);
  vw->saveGuardEnv();
  if (vw->guardEnvEmpty()) {
    ErrorMessage(env,"pfor statement declares no loop variables.",nil,keyword);
  }

  // We heap allocate our temporary structures because otherwise the
  // next loop iteration would overwrite it, before the thread even started.
  // Turn off guard handling.
  vw->handleGuard(false);
  convertToThread(cur,thnames,body,vw,env,true);

  Ptree *thn = thnames->First();

  Ptree *tvname = Ptree::GenSym();
  Ptree *iname = Ptree::GenSym();

  return Ptree::qMake("plasma::TVect `tvname`;\n"
                      "for (`s1` `s2` ; `s3`) { `start` `tvname`.push_back(`thn`); }\n"
                      "for (plasma::TVect::iterator `iname` = `tvname`.begin(); `iname` != `tvname`.end(); ++`iname`) { pWait(*`iname`); }\n"
                      );
}

// This converts an expression into a thread invocation of a function containing that
// expression.
// elist:     Expression list to append to.  Must not be null.
// thnames:   List of thread names.
// expr:      Expression to make into a thread.
// vw:        VarWalker class which stores parameter usage information.
// env:       Current environment.
// onthread:  Copy thread structure to space allocated at beginning of thread.
void Plasma::convertToThread(Ptree* &elist,Ptree* &thnames,Ptree *expr,VarWalker *vw,
                             Environment *env,bool onthread)
{
  Ptree *tstype = Ptree::GenSym();       // Symbol for thread argument structure type.
  Ptree *tsname = Ptree::GenSym();       // Symbol for thread argument structure instance name.
  Ptree *nfname = Ptree::GenSym();
  Ptree *thname = Ptree::GenSym();       // Symbol for thread handle name.

  assert(elist);

  vw->setnames(tstype,tsname);              
  Ptree *nexpr = vw->Translate(expr);    // Translate expression, scanning for variables.
  Ptree *args = vw->args();

  thnames = Ptree::Cons(thname,thnames); // Add on to list of thread names.

  if (args) {
    // Arguments exist- we have to create a structure in which
    // to pass them.
    makeThreadStruct(env,tstype,args);     // Create the argument structure.
    const string &arglist = argList(vw->argnames());
    // We insert a prototype before, and the actual function after, the current location
    // because we want to handle the case where the thread calls the current function recursively.
    InsertBeforeToplevel(env,Ptree::qMake("void `nfname`(void *`tsname`);\n"));
    AppendAfterToplevel(env,Ptree::qMake("void `nfname`(void *`tsname`) {\n`TranslateExpression(env,nexpr)`\n}\n"));
    if (onthread) {
      elist = lappend(elist,Ptree::qMake("`tstype` `tsname` = {`arglist.c_str()`};\n"
                                         "plasma::THandle `thname` = plasma::pSpawn(`nfname`,sizeof(`tstype`),&`tsname`).first;\n"));
    } else {
      elist = lappend(elist,Ptree::qMake("`tstype` `tsname` = {`arglist.c_str()`};\n"
                                         "plasma::THandle `thname` = plasma::pSpawn(`nfname`,&`tsname`);\n"));
    }
  } else {
    // No arguments, so no need to create the structure.
    InsertBeforeToplevel(env,Ptree::qMake("void `nfname`(void *);\n"));
    AppendAfterToplevel(env,Ptree::qMake("void `nfname`(void *) {\n`TranslateExpression(env,nexpr)`\n}\n"));
    elist = lappend(elist,Ptree::qMake("plasma::THandle `thname` = plasma::pSpawn(`nfname`,0);\n"));
  }
}

// This constructs and inserts a thread structure only if we don't have one that's large enough.
void Plasma::makeThreadStruct(Environment *env,Ptree *type,Ptree *args)
{
    Ptree *front = Ptree::List(Ptree::qMake("struct `type` {\n"));
    Ptree *cur = front;
    for(PtreeIter i = args; !i.Empty(); i++){
      cur = lappend(cur,Ptree::qMake("  `*i`;\n"));
    }
    cur = lappend(cur,Ptree::qMake("};\n"));
    InsertBeforeToplevel(env,front);
}

// Translate an alt block.
Ptree* Plasma::TranslateAlt(Environment* env,Ptree* keyword, Ptree* rest)
{
  Ptree *body;

  if(!Ptree::Match(rest, "[ %? ]", &body)){
    ErrorMessage(env, "invalid alt statement", nil, keyword);
    return nil;
  }

  // Ignore the braces.
  body = body->Cadr();
  
  PortVect pv;
  Ptree *defaultblock = nil;

  if (!parseAltBlock(env,body,pv,defaultblock)) {
    return nil;
  }

  Ptree *iname = Ptree::GenSym();  
  Ptree *label = Ptree::GenSym();

  // We put this inside of a try block if we don't have a default block,
  // since we want to clear the notifications if an exception occurs.
  Ptree *start = (!defaultblock) ? 
    Ptree::List(Ptree::Make("try {\n")) :
    Ptree::List(Ptree::Make("{\n"));
  Ptree *cur = start;

  cur = lappend(cur,Ptree::qMake("plasma::pLock();\n"
                                 "int `iname`;\n"));

  // The first part of the alt block queries each channel to see
  // if it has any data ready.
  for (int index = 0; index != (int)pv.size(); ++index) {
    const Port &port = pv[index];
    cur = lappend(cur,Ptree::qMake("`iname` = `index`;\n"
                                   "if ( (`port.chan`).ready() ) { goto `label`; }\n"));
  }

  // If we have a default block, this becomes the last index in our
  // case statement.  We jump directly there if we reach this point.
  if (defaultblock) {
    cur = lappend(cur,Ptree::qMake("`iname` = `(int)(pv.size())`;\n"));
  } else {

    // This generated code is only reached if no ports were ready.
    // In that case, we set each channel to notify us when it has data.
    for (int index = 0; index != (int)pv.size(); ++index) {
      const Port &port = pv[index];

      cur = lappend(cur,Ptree::qMake("(`port.chan`).set_notify(plasma::pCurThread(),`index`);\n"));
    }

    // Next, we sleep.  When we wake up, we get the handle of the channel
    // which awoke us.
    cur = lappend(cur,Ptree::qMake("`iname` = plasma::pSleep();\n"
                                   "plasma::pLock();\n"));

    // Clear all notification, since we have a value.
    for (unsigned index = 0; index != pv.size(); ++index) {
      const Port &port = pv[index];
      cur = lappend(cur,Ptree::qMake("(`port.chan`).clear_notify();\n"));
    }

  }

  if ( (cur = generateAltBody(env,cur,label,iname,pv,defaultblock)) == nil) {
    return nil;
  }

  // Finish try block with exception cleanup code.
  cur = lappend(cur,Ptree::Make("}\n"));

  // Exception cleanup code if no default block.
  if (!defaultblock) {
    cur = lappend(cur,Ptree::Make("catch (...) {\n"));
    for (unsigned index = 0; index != pv.size(); ++index) {
      const Port &port = pv[index];
      cur = lappend(cur,Ptree::qMake("(`port.chan`).clear_notify();\n"));
    }
    cur = lappend(cur,Ptree::Make("}\n"));
  }

  return start;
}

// Translate an afor block.  This is similar to an alt block, except that
// we loop over the port statements.
Ptree* Plasma::TranslateAfor(Environment* env,Ptree* keyword, Ptree* rest)
{
  Ptree *s1, *s2, *s3, *body;

  if(!Ptree::Match(rest, "[ ( %? %? ; %? ) %? ]", &s1,&s2,&s3,&body)) {
    ErrorMessage(env, "invalid afor statement", nil, keyword);
    return nil;
  }

  // Ignore the braces.
  body = body->Cadr();
  
  PortVect pv;
  Ptree *defaultblock = nil;

  if (!parseAltBlock(env,body,pv,defaultblock)) {
    return nil;
  }

  if (pv.size() > 1) {
    ErrorMessage(env,"more than one port statement in an afor block is currently not supported",nil,keyword);
    return nil;
  }

  // We have to extract the iterator from the first statement in the afor loop.
  // This means that it somewhat limits the generality of the for-loop style statement.
  // We may have to address this issue later.
  VarWalker *vw = new VarWalker(env->GetWalker()->GetParser(),env);
  if (!parseAforCondition(vw,env,s1,s3)) {
    return nil;
  }

  const ArgVect &argnames = vw->argnames();
  // We just take the first declaration and assume that's the index variable for afor.
  Ptree *index = argnames.front()._arg;
  TypeInfo indextype = argnames.front()._type;
  // This controls whether we can put the value into the channel's handle or if
  // we need an auxiliary stack.
  bool needstack = true;
  if (int bt = indextype.IsBuiltInType()) {
    if (bt & (CharType | IntType | ShortType | LongType | BooleanType)) {
      needstack = false;
    }
  }

  Ptree *iname  = Ptree::GenSym();
  Ptree *sindex = Ptree::GenSym();
  Ptree *stack  = Ptree::GenSym();
  Ptree *label  = Ptree::GenSym();

  const Port &port = pv.front();

  // The first part of the afor block queries each channel to see
  // if it has any data ready.  It's placed within the general afor
  // for-loop.
  Ptree *start = (!defaultblock) ? 
    Ptree::List(Ptree::Make("try {\n")) :
    Ptree::List(Ptree::Make("{\n"));
  Ptree *cur = start;
  cur = lappend(cur,Ptree::qMake("plasma::pLock();\n"
                                 "int `iname` = 0;\n"
                                 "`s1`\n"));

  // Create a stack of target types, and store indices to those values,
  // only if we don't have a built-in type as an index variable.
  if (needstack && !defaultblock) {
    cur = lappend(cur,Ptree::qMake("std::vector<`indextype.MakePtree(0)`> `stack`;\n"
                                   "int `sindex` = 0;\n"));
  }

  cur = lappend(cur,Ptree::qMake("for ( ; `s2` ; `s3`) {\n"
                                 "if ( (`port.chan`).ready() ) { goto `label`; }\n"
                                 "}\n"));
  
  // If we have a default block, we'll reach it if nothing is ready.
  // We'll then jump past the sleep logic.
  if (defaultblock) {
    cur = lappend(cur,Ptree::qMake("`iname` = 1;\n"
                                   "goto `label`;\n"));
  } else {
    // This generated code is only reached if no ports were ready.
    // In that case, we set each channel to notify us when it has data.
    // Next, we sleep.  When we wake up, we get the handle of the channel
    // which woke us.
    // If we have a non-builtin type as an index variable, we store each index
    // variable in a vector and store the corresponding index.  When we wake up,
    // we fetch that value and then use it.
    if (needstack) {
      cur = lappend(cur,Ptree::qMake("for (`s1` `s2` ; `s3`) {\n"
                                     "(`port.chan`).set_notify(plasma::pCurThread(),`sindex`++);\n"
                                     "`stack`.push_back(`index`);\n"
                                     "}\n"
                                     "`index` = `stack`[plasma::pSleep()];\n"
                                     "plasma::pLock();\n"));
    } else {
      cur = lappend(cur,Ptree::qMake("for (`s1` `s2` ; `s3`) {\n"
                                     "(`port.chan`).set_notify(plasma::pCurThread(),`index`);\n"
                                     "}\n"
                                     "`index` = ((`indextype.MakePtree(0)`) plasma::pSleep());\n"
                                     "plasma::pLock();\n"));
    }

    // Clear all notification, since we have a value.
    cur = lappend(cur,Ptree::qMake("for (`s1` `s2` ; `s3`) {\n"
                                   "(`port.chan`).clear_notify();\n"
                                   "}\n"));
  }

  if ( (cur = generateAltBody(env,cur,label,iname,pv,defaultblock)) == nil) {
    return nil;
  }

  cur = lappend(cur,Ptree::Make("}\n"));

  // Exception cleanup code if no default block.
  if (!defaultblock) {
    cur = lappend(cur,Ptree::qMake("catch (...) {\n"
                                   "for (`s1` `s2` ; `s3`) {\n"
                                   "(`port.chan`).clear_notify();\n"
                                   "}\n"
                                   "}\n"));
  }

  return start;
}

Ptree *Plasma::generateAltBody(Environment *env,Ptree *cur,Ptree *label,Ptree *iname,
                               const PortVect &pv,Ptree *defaultblock)
{
  // Unlock processors- may be redundant for some cases, but is needed
  // for default blocks and non-standard channels.
  // Jump to the code for the relevant handle.
  cur = lappend(cur,Ptree::qMake("`label`:\n"
                                 "plasma::pUnlock();\n"
                                 "switch(`iname`) {\n"));

  // Handling code.  Each value should be a valid declaration.
  // The second statement represents the channel to be queried.
  for (int index = 0; index != (int)pv.size(); ++index) {
    const Port &port = pv[index];
    if (port.val) {
      Ptree *val,*cv,*type;
      if (Ptree::Match(port.val,"[%? %? %? ;]",&cv,&type,&val)) {
        // User specified a type, so use it directly.
        cur = lappend(cur,Ptree::qMake("case `index`: {\n"
                                       "`cv``type``val` = (`port.chan`).get();\n"
                                       "{\n"
                                       "`port.body`\n"
                                       "}\n"
                                       "} break;\n"));
      } else {
        ErrorMessage(env,"Invalid value declaration for port statement",nil,port.val);
        return nil;
      }
    } else {
      // Simple case- no value, so just insert code.
      cur = lappend(cur,Ptree::qMake("case `index`: {\n"
                                     "`port.body`\n"
                                     "} break;\n"));      
    }
  }

  // Write the default block code, if present.
  if (defaultblock) {
    cur = lappend(cur,Ptree::qMake("case `(int)(pv.size())`: {\n"
                                    "`defaultblock`\n"
                                    "} break;\n"));
  }

  // End of switch statement.
  cur = lappend(cur,Ptree::Make("}\n"));

  return cur;
}

// This first parses the first expression in the afor statement, recording
// any declarations.  It then parses the second statement- any variables used
// there are recorded.  The first variable used in the third statement is
// then considered to be the loop index that we use in the afor block.
bool Plasma::parseAforCondition(VarWalker *vw,Environment *env,Ptree *s1,Ptree *s3)
{
  vw->handleGuard(true);
  vw->Translate(s1);
  vw->saveGuardEnv();
  if (vw->guardEnvEmpty()) {
    ErrorMessage(env,"afor statement declares no loop variables.",nil,s1);
    return nil;
  }
  vw->handleGuard(false);
  vw->Translate(s3);

  if (vw->argnames().empty()) {
    ErrorMessage(env,"afor statement does not use its iterator variable in its condition expression.",nil,s3);
    return false;
  }
  return true;
}

bool Plasma::parseAltBlock(Environment *env,Ptree *body,PortVect &pv,Ptree* &defaultblock)
{
  // Walk through the statements in the alt block.  Each should be a 
  // case statement or a default statement.
  for (PtreeIter i = body; !i.Empty(); ++i) {
    Ptree *pval,*pchan,*pnil,*pbody;
    if (Ptree::Match(*i,"[ port ( %? %? ; %? ) %? ]",&pval,&pchan,&pnil,&pbody)) {
      if (pnil != nil) {
        ErrorMessage(env,"bad port statement:  Nothing may appear after second semicolon.",nil,*i);
      }
      pv.push_back(Port(pchan,pval,pbody));
    }
    else if (Ptree::Match(*i,"[ { %* } ]")) {
      defaultblock = *i;
    } else {
      ErrorMessage(env, "bad alt statement:  Only port blocks or a default block are allowed within the alt block.",nil,*i);
      return false;
    }
  }

  if (pv.empty()) {
    ErrorMessage(env,"bad alt statement:  You must have at least one port or default block.",nil,body);
    return false;
  }
  return true;
}

bool isPtrType(TypeInfo &t)
{
  return (t.IsPointerToMember() || t.IsPointerType());
}

// Translates a spawn "function call".  It expects a single argument which must be a function
// call or member invocation.  The arguments are evaluated immediately, then a thread is
// launched of that function call.  The return value is a ResChannel object which contains the
// result of the function.  Trying to read the result before it's finished will block the thread.
Ptree* Plasma::TranslateFunctionCall(Environment* env,Ptree* spawnobj, Ptree* preargs)
{
  if (!spawnobj->Eq("spawn")) {
    ErrorMessage(env,"bad spawn class- make sure that you haven't used the pSpawner keyword erroneously.",nil,spawnobj);
    return nil;
  }

  Ptree *args = TranslateArguments(env,preargs)->Cadr();

  if (args->Length() > 1) {
    ErrorMessage(env,"The spawn operator accepts only one argument.",nil,spawnobj);
    return nil;
  }

  // User function and its arguments.
  Ptree *ufunc,*uargs;

  // Try to extract a function call.
  if (!Ptree::Match(args,"[[ %? ( %? ) ]]",&ufunc,&uargs)) {
    Ptree *uop,*uptr;
    // For some reason, member pointers parse differently than member calls.
    // So we convert it here to the same form:  obj <op> member.
    if (Ptree::Match(args,"[[ %? %? [ %? ( %? ) ] ]]",&ufunc,&uop,&uptr,&uargs)) {
      ufunc = Ptree::List(ufunc,uop,uptr);
    } else {
      ErrorMessage(env,"Invalid spawn call- argument must be a function call.",nil,args);
      return nil;
    }
  }

  // Will store object's class if a member call.
  Class *objclass = nil;

  // Will store object pointer expression if a member call.
  Ptree *objptr = nil;

  // Environment of the invoked type.  The main environment if a function, otherwise
  // environment of the class defining the member.
  Environment *callenv = env;

  // Function or member call's type.
  TypeInfo t;

  // If we're dealing with a member call, setup stuff appropriately.
  if (!checkForMemberCall(env,objclass,callenv,ufunc,objptr)) {
    return nil;;
  }

  // The argument type must be a function call.  Its return type will indicate
  // the type of the result channel object.
  if (!callenv->Lookup(ufunc,t)) {
    ErrorMessage(env,"Could not lookup spawn argument.",nil,ufunc);
    return nil;
  }

  // Make sure that spawn's argument is some sort of a subroutine call.
  if (!(t.IsFunction() || t.IsPointerToMember())) {
    TypeInfo tt;
    t.Dereference(tt);
    if (!tt.IsFunction()) {
      ErrorMessage(env,"Spawn argument must be a function call, function pointer, or method call.",nil,ufunc);
      return nil;
    }
  }

  Ptree *targs  = Ptree::GenSym();
  Ptree *sfunc  = Ptree::GenSym();
  Ptree *lfunc  = Ptree::GenSym();

  // Create temporary argument storage structure.
  if (!makeSpawnStruct(env,objclass,t,targs,ufunc)) {
    return nil;
  }

  // Create spawn function- this will take the function and its arguments
  // and launch a thread, returning a result structure w/a pointer to the
  // result data.
  if (!makeSpawnFunc(env,objclass,t,targs,ufunc,lfunc,sfunc)) {
    return nil;
  }

  // Finally, replace spawn operator w/call to spawn function.
  Ptree *start = Ptree::qMake("`sfunc`(");
  Ptree *cur = start;

  bool fptr = isPtrType(t);

  // If we have an explicit object, pass as argument.
  if (objptr) {
    cur = lappend(cur,objptr);
    if (uargs || fptr) {
      cur = lappend(cur,Ptree::Make(","));
    }
  }

  // If we have a pointer to a member or function, pass as argument.
  if (fptr) {
    cur = lappend(cur,ufunc);
    if (uargs) {
      cur = lappend(cur,Ptree::Make(","));
    }
  }

  cur = lappend(cur,Ptree::qMake("`uargs`)"));
  return start;
}

bool Plasma::checkForMemberCall(Environment *env,Class* &objclass,
                                 Environment* &callenv,Ptree* &ufunc,
                                Ptree* &objptr)
{
  // Are we dealing with a pointer or reference to a member?
  bool memptr = false;
  bool explicitobj = true;
  bool deref = false;
  Ptree *object, *member;
  if (Ptree::Match(ufunc,"[ %? . %? ]",&object,&member)) {
    // Member call with reference of object.
    objptr = Ptree::qMake("&(`object`)");
  } else if (Ptree::Match(ufunc,"[ %? -> %? ]",&object,&member)) {
    // Member call with pointer to object.
    objptr = object;
    deref = true;
  } else if (Ptree::Match(ufunc,"[ %? .* %? ]",&object,&member)) {
    // Pointer to member w/reference object.
    objptr = Ptree::qMake("&(`object`)");
    memptr = true;
  } else if (Ptree::Match(ufunc,"[ %? ->* %? ]",&object,&member)) {
    // Pointer to member w/pointer object.
    objptr = object;
    deref = true;
    memptr = true;
  } else if (Ptree::Match(ufunc,"[ %? :: %? ]",&object,&member)) {
    // Call to static- handle as a function.
    object = nil;
    member = nil;
    objptr = nil;
  } else {
    // Not a member call with an explicit object.
    object = nil;
    member = nil;
    objptr = nil;
    explicitobj = false;
  }

  // Explicit object was found- find its environment.
  if (object) {
    TypeInfo pt;
    if (!env->Lookup(object,pt)) {
      ErrorMessage(env,"Could not lookup member call class.",nil,object);
      return false;
    }
    if (deref) {
      pt.Dereference();
    }
    if (!(objclass = pt.ClassMetaobject())) {
      ErrorMessage("Could not find class's type.",nil,object);
      return false;
    }
    ufunc = member;
    callenv = (memptr) ? callenv : objclass->GetEnvironment();
  }

  // Is this an implicit member call?  If so, we need to add the this pointer as
  // an argument.
  if (!explicitobj) {
    if (Environment *menv = env->IsMember(ufunc)) {
      // Is a member- get type.
      if (!(objclass = menv->LookupThis())) {
        ErrorMessage(env,"Could not find member function's class.",nil,ufunc);
        return false;
      }
      callenv = menv;
      objptr = Ptree::Make("this");
    }
  }

  return true;
}

Ptree *className()
{
  return Ptree::Make("_class");
}

Ptree *ptrName()
{
  return Ptree::Make("_ptr");
}

Ptree *resultName()
{
  return Ptree::Make("_result");
}

Ptree *argName(int i)
{
  char buf[10];
  sprintf(buf,"_arg%d",i);
  return Ptree::Make(buf);
}

// Creates a structure for storing a spawned thread's result and arguments.
bool Plasma::makeSpawnStruct(Environment *env,Class *objclass,TypeInfo t,Ptree *targs,Ptree *ufunc)
{
  Ptree *start = Ptree::qMake("struct `targs` {\n");
  Ptree *cur = start;

  TypeInfo origt = t;
  // If we have a function or member pointer, note that fact
  // and dereference so that we can query for argument and
  // return information.
  bool fptr = isPtrType(t);
  if (fptr) {
    t.Dereference();
  }

  // Note:  The result ***must*** go first because we point the Result object to
  // the beginning of the block in order to get the function result.
  TypeInfo rt;
  t.Dereference(rt);
  if (t.IsNoReturnType()) {
    ErrorMessage(env,"Spawn's argument must return a value- void functions are not allowed.",nil,ufunc);
    return false;
  }
  cur = lappend(cur,Ptree::qMake("`rt.MakePtree(resultName())`;\n"));

  TypeInfo at;
  int numargs = t.NumOfArguments();
  assert(numargs >= 0);
  for (int i = 0; i != numargs; ++i) {
    t.NthArgument(i,at);
    cur = lappend(cur,Ptree::qMake("`at.MakePtree(argName(i))`;\n"));
  }

  // If we have a object class pointer, create pointer to store it.
  if (objclass) {
    cur = lappend(cur,Ptree::qMake("`objclass->Name()` *`className()`;\n"));
  }

  // If we're dealing with a member pointer, store it.
  if (fptr) {
    cur = lappend(cur,Ptree::qMake("`origt.MakePtree(ptrName())`;\n"));
  }
  
  cur = lappend(cur,Ptree::Make("};\n"));
  InsertBeforeToplevel(env,start);
  return true;
}

// Creates the launcher function and spawn function for a spawn operator.
bool Plasma::makeSpawnFunc(Environment *env,Class *objclass,TypeInfo t,Ptree *targs,
                           Ptree *ufunc,Ptree *lfunc,Ptree *sfunc)
{
  // Create the launcher function which calls the user function w/proper args.
  Ptree *start = Ptree::qMake("inline void `lfunc`(void *args)\n"
                              "{\n"
                              "  ((`targs`*)args)->`resultName()` = ");
  Ptree *cur = start;

  TypeInfo origt = t;
  bool fptr = false;
  if (isPtrType(t)) {
    fptr = true;
    t.Dereference();
  }

  // Create invoker line- this calls the user function.  We must decide whether we're invoking a
  // method or a function and whether we're dealing with a pointer or a fixed value.
  if (objclass) {
    if (fptr) {
      cur = lappend(cur,Ptree::qMake("(((`targs`*)args)->`className()`->*(((`targs`*)args)->`ptrName()`))("));
    } else {
      cur = lappend(cur,Ptree::qMake("((`targs`*)args)->`className()`->`ufunc`("));
    }
  } else {
    if (fptr) {
      cur = lappend(cur,Ptree::qMake("(((`targs`*)args)->`ptrName()`)("));      
    } else {
      cur = lappend(cur,Ptree::qMake("`ufunc`("));
    }
  }

  // Now write out the arguments to the user function.
  int numargs = t.NumOfArguments();
  for (int i = 0; i != numargs; ++i) {
    if (i) {
      cur = lappend(cur,Ptree::Make(","));
    }
    cur = lappend(cur,Ptree::qMake("((`targs`*)args)->`argName(i)`"));
  };
  
  cur = lappend(cur,Ptree::Make(");\n}\n"));
  InsertBeforeToplevel(env,start);

  TypeInfo rt;
  t.Dereference(rt);

  // Create the spawn function which launches the thread and returns
  // the result template.
  Ptree *rtemplate = Ptree::qMake("Result<`rt.MakePtree(0)`>");
  start = Ptree::qMake("inline `rtemplate` `sfunc`(");
  cur = start;

  // If we have an explicit object, we need it as an argument.
  if (objclass) {
    cur = lappend(cur,Ptree::qMake("`objclass->Name()` *`className()`"));
    if (numargs || fptr) {
      cur = lappend(cur,Ptree::qMake(","));
    }
  }

  // If we have a pointer to the function or method, it is an argument.
  if (fptr) {
    cur = lappend(cur,Ptree::qMake("`origt.MakePtree(ptrName())`"));
    if (numargs) {
      cur = lappend(cur,Ptree::qMake(","));
    }
  }

  // The arguments to the function to be invoked.
  TypeInfo at;
  for (int i = 0; i != numargs; ++i) {
    if (i) {
      cur = lappend(cur,Ptree::Make(","));
    }
    t.NthArgument(i,at);
    cur = lappend(cur,Ptree::qMake("`at.MakePtree(argName(i))`"));
  }

  // Initialization of the argument passing structure.
  Ptree *emptyarg = Ptree::Make("()");
  cur = lappend(cur,Ptree::qMake(")\n{\n"
                                 "`targs` args = {`rt.MakePtree(emptyarg)`"));
  for (int i = 0; i != numargs; ++i) {
    cur = lappend(cur,Ptree::qMake(",`argName(i)`"));
  }
  if (objclass) {
    cur = lappend(cur,Ptree::qMake(",`className()`"));
  }
  if (fptr) {
    cur = lappend(cur,Ptree::qMake(",`ptrName()`"));
  }

  // Finally, we spawn the launch function.
  cur = lappend(cur,Ptree::qMake("};\n"
                                 "return `rtemplate`(pSpawn(`lfunc`,sizeof(`targs`),&args));\n}\n"));
  
  InsertBeforeToplevel(env,start);

  return true;
}
