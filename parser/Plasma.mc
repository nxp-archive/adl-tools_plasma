
#include <assert.h>
#include <iostream>
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
  RegisterNewWhileStatement("on");
  RegisterNewBlockStatement("alt");
  RegisterNewForStatement("afor");
  RegisterNewClosureStatement("port");
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

  // Do we have a placement statement?  If so, proc will store the expression
  // which specifies the processor.  If not, this will be nil, so the evaluation
  // will produce an empty string.
   Ptree *proc = nil,*tproc,*tbody;
  if (Ptree::Match(nexpr,"[ on ( %? ) %? ]",&tproc,&tbody)) {
    nexpr = tbody;
    proc = Ptree::qMake("(`tproc`)(),");
  }

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
                                         "plasma::THandle `thname` = plasma::pSpawn(`proc` `nfname`,sizeof(`tstype`),&`tsname`).first;\n"));
    } else {
      elist = lappend(elist,Ptree::qMake("`tstype` `tsname` = {`arglist.c_str()`};\n"
                                         "plasma::THandle `thname` = plasma::pSpawn(`proc` `nfname`,&`tsname`);\n"));
    }
  } else {
    // No arguments, so no need to create the structure.
    InsertBeforeToplevel(env,Ptree::qMake("void `nfname`(void *);\n"));
    AppendAfterToplevel(env,Ptree::qMake("void `nfname`(void *) {\n`TranslateExpression(env,nexpr)`\n}\n"));
    elist = lappend(elist,Ptree::qMake("plasma::THandle `thname` = plasma::pSpawn(`proc` `nfname`,0);\n"));
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

bool isPtrType(TypeInfo &t)
{
  return (t.IsPointerToMember() || t.IsPointerType());
}

// Translates a member call.  This is designed to handle calls to spawn() from Processor
// objects.  This function is invoked because we state that Plasma is a metaclass for Processor.
Ptree* Plasma::TranslateMemberCall(Environment* env, Ptree* object,
                                   Ptree* op, Ptree* member, Ptree* arglist)
{
  // For future compatibility, if the member isn't "spawn", pass it on through.
  if (!member->Eq("spawn")) {
    return Class::TranslateMemberCall(env,object,op,member,arglist);
  } else {
    // Was this a pointer call?  If so, dereference it.
    Ptree *obj = object;
    if (op->Eq("->")) {
      obj = Ptree::qMake("*(`object`)");
    }
    return TranslateFunctionCall(env,obj,arglist);
  }
}

// Translates a spawn "function call".  It expects a single argument which must be a function
// call or member invocation.  The arguments are evaluated immediately, then a thread is
// launched of that function call.  The return value is a ResChannel object which contains the
// result of the function.  Trying to read the result before it's finished will block the thread.
Ptree* Plasma::TranslateFunctionCall(Environment* env,Ptree* spawnobj, Ptree* preargs)
{
  // If the object is just "spawn" then we consider this to be the function call invocationm, e.g.
  // the spawn operator.  Otherwise, we consider this to be the spawn pseudo-method call.
  Ptree *proc = nil;
  if (!spawnobj->Eq("spawn")) {
    proc = Ptree::qMake("(`spawnobj`)()");
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
  if (!makeSpawnFunc(env,objclass,t,targs,ufunc,lfunc,sfunc,(proc != nil))) {
    return nil;
  }

  // Finally, replace spawn operator w/call to spawn function.
  Ptree *start = Ptree::qMake("`sfunc`(");
  Ptree *cur = start;

  bool fptr = isPtrType(t);

  // If we have a specified processor, pass as argument.
  if (proc) {
    cur = lappend(cur,proc);
    if (objptr || uargs || fptr) {
      cur = lappend(cur,Ptree::Make(","));
    }
  }

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

Ptree *procName()
{
  return Ptree::Make("proc");
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
                           Ptree *ufunc,Ptree *lfunc,Ptree *sfunc,bool proc)
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
  Ptree *rtemplate = Ptree::qMake("plasma::Result<`rt.MakePtree(0)`>");
  start = Ptree::qMake("inline `rtemplate` `sfunc`(");
  cur = start;

  // If we have a processor, we need it as an argument.
  if (proc) {
    cur = lappend(cur,Ptree::qMake("plasma::Proc *`procName()`"));
    if (objclass || numargs || fptr) {
      cur = lappend(cur,Ptree::qMake(","));
    }
  }

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
                                 "return `rtemplate`(plasma::pSpawn("));
  if (proc) {
    cur = lappend(cur,Ptree::qMake("`procName()`,"));
  }
  cur = lappend(cur,Ptree::qMake("`lfunc`,sizeof(`targs`),&args));\n}\n"));
  
  InsertBeforeToplevel(env,start);

  return true;
}

// Translate an alt block.
Ptree* Plasma::TranslateAlt(Environment* env,Ptree* keyword, Ptree* rest)
{
  PortVect pv;
  Ptree *defaultblock = nil;

  if (!parseAltBody(env,rest,pv,defaultblock)) {
    return false;
  }

  return generateAltBlock(env,pv,defaultblock);
}

// Translate an afor block.  This is similar to an alt block, except that
// we loop over the port statements.
Ptree* Plasma::TranslateAfor(Environment* env,Ptree* keyword, Ptree* rest)
{
  PortVect pv;
  Ptree *defaultblock = nil;

  if (!parseAforBody(env,rest,pv,defaultblock)) {
    return false;
  }

  return generateAltBlock(env,pv,defaultblock);
}

// Main generation function for alt/afor blocks.  Given a list of
// ports, constructs the necessary alt structure.
Ptree *Plasma::generateAltBlock(Environment *env,const PortVect &pv,Ptree *defaultblock)
{
  Ptree *label  = Ptree::GenSym();
  Ptree *handle = Ptree::GenSym();
  Ptree *sindex = Ptree::GenSym();
  Ptree *uflag  = Ptree::GenSym();

  // We put this inside of a try block if we don't have a default block,
  // since we want to clear the notifications if an exception occurs.
  Ptree *start = (!defaultblock) ? 
    Ptree::List(Ptree::Make("try {\n")) :
    Ptree::List(Ptree::Make("{\n"));
  Ptree *cur = start;

  cur = lappend(cur,Ptree::qMake("plasma::pLock();\n"
                                 "plasma::HandleType `handle`(0,0);\n"));

  // For each afor entry, write out initial statement loop statement.
  // Create auxiliary stack if needed.
  bool needsindex = false;
  bool haveloops = false;
  for (PortVect::const_iterator i = pv.begin(); i != pv.end(); ++i) {
    const Port &p = *i;
    if (p.isloop()) {
      haveloops = true;
      cur = lappend(cur,Ptree::qMake("`p.s1`\n"));
      // Create a stack of target types, and store indices to those values,
      // only if we don't have a built-in type as an index variable.
      if (p.needstack() && !defaultblock) {
        TypeInfo t = p.indextype;
        cur = lappend(cur,Ptree::qMake("std::vector<`t.MakePtree(0)`> `p.stack`;\n"));
        needsindex = true;
      }
    }
  }

  // If we have any loops then we need an update flag.
  if (haveloops && !defaultblock) {
    cur = lappend(cur,Ptree::qMake("bool `uflag` = false;\n"));
  }

  // If needed, create index variable for use w/auxiliary stacks.
  if (needsindex) {
    cur = lappend(cur,Ptree::qMake("int `sindex` = 0;\n"));
  }

  // For each entry, write out ready-query logic.
  for (int index = 0; index != (int)pv.size(); ++index) {
    const Port &p = pv[index];
    cur = lappend(cur,Ptree::qMake("`handle`.first = `index`;\n"));
    if (p.isloop()) {
      // Afor code.
      cur = lappend(cur,Ptree::qMake("for ( ; `p.s2` ; `p.s3`) {\n"
                                     "if ( (`p.chan`) `p.op` ready() ) { goto `label`; }\n"
                                     "}\n"));
    } else {
      // Alt code.
      cur = lappend(cur,Ptree::qMake("if ( (`p.chan`) `p.op` ready() ) { goto `label`; }\n"));
    }
  }

  // If we have a default block, this becomes the last index in our
  // case statement.  We jump directly there if we reach this point.
  if (defaultblock) {
    cur = lappend(cur,Ptree::qMake("`handle`.first = `(int)(pv.size())`;\n"));
  } else {
    // This generated code is only reached if no ports were ready.
    // In that case, we set each channel to notify us when it has data.
    for (int index = 0; index != (int)pv.size(); ++index) {
      const Port &p = pv[index];
      if (p.isloop()) {
        // If we have a non-builtin type as an index variable, we store each index
        // variable in a vector and store the corresponding index.  When we wake up,
        // we fetch that value and then use it.
        if (p.needstack()) {
          cur = lappend(cur,Ptree::qMake("`sindex` = 0;\n"
                                         "for (`p.s1` `p.s2` ; `p.s3`) {\n"
                                         "(`p.chan`) `p.op` set_notify(plasma::pCurThread(),plasma::HandleType(`index`,`sindex`++));\n"
                                         "`p.stack`.push_back(`p.loopvar`);\n"
                                         "}\n"));
        } else {
          cur = lappend(cur,Ptree::qMake("for (`p.s1` `p.s2` ; `p.s3`) {\n"
                                         "(`p.chan`) `p.op` set_notify(plasma::pCurThread(),plasma::HandleType(`index`,(int)`p.loopvar`));\n"
                                         "}\n"));
        }        
      } else {
        cur = lappend(cur,Ptree::qMake("(`p.chan`) `p.op` set_notify(plasma::pCurThread(),plasma::HandleType(`index`,0));\n"));
      }
    }

    // If we have any loops, then toggle the update flag to truee here- when we
    // wake up and jump to the proper case spot, this will tell us to update the
    // loop variable from the second element of the handle.
    if (haveloops && !defaultblock) {
      cur = lappend(cur,Ptree::qMake("`uflag` = true;\n"));
    }

    // Next, we sleep.  When we wake up, we get the handle of the channel
    // which awoke us.
    cur = lappend(cur,Ptree::qMake("`handle` = plasma::pSleep();\n"
                                   "plasma::pLock();\n"));

    // Clear all notification, since we have a value.
    for (unsigned index = 0; index != pv.size(); ++index) {
      const Port &p = pv[index];
      if (p.isloop()) {
        cur = lappend(cur,Ptree::qMake("for (`p.s1` `p.s2` ; `p.s3`) {\n"
                                       "(`p.chan`) `p.op` clear_notify();\n"
                                       "}\n"));        
      } else {
        cur = lappend(cur,Ptree::qMake("(`p.chan`) `p.op` clear_notify();\n"));
      }
    }
  }

  if ( (cur = generateAltBody(env,cur,label,handle,uflag,pv,defaultblock)) == nil) {
    return nil;
  }

  cur = lappend(cur,Ptree::Make("}\n"));

  // Exception cleanup code if no default block.
  if (!defaultblock) {
    cur = lappend(cur,Ptree::Make("catch (...) {\n"));
    for (PortVect::const_iterator i = pv.begin(); i != pv.end(); ++i) {
      const Port &p = *i;
      if (p.isloop()) {
        cur = lappend(cur,Ptree::qMake("for (`p.s1` `p.s2` ; `p.s3`) {\n"
                                       "  (`p.chan`) `p.op` clear_notify();\n"
                                       "}\n"));
      } else {
        cur = lappend(cur,Ptree::qMake("(`p.chan`) `p.op` clear_notify();\n"));
      }
    }
    cur = lappend(cur,Ptree::qMake("}\n"));
  }

  return start;

}

// Generates the case statement for the alt action code.
Ptree *Plasma::generateAltBody(Environment *env,Ptree *cur,Ptree *label,Ptree *handle,Ptree *uflag,
                               const PortVect &pv,Ptree *defaultblock)
{
  // Unlock processors- may be redundant for some cases, but is needed
  // for default blocks and non-standard channels.
  // Jump to the code for the relevant handle.
  cur = lappend(cur,Ptree::qMake("`label`:\n"
                                 "plasma::pUnlock();\n"
                                 "switch(`handle`.first) {\n"));

  // Handling code.  Each value should be a valid declaration.
  // The second statement represents the channel to be queried.
  for (int index = 0; index != (int)pv.size(); ++index) {
    const Port &p = pv[index];
    if (p.val) {
      if (p.val->Length() > 1) {
        ErrorMessage(env,"Invalid port statement:  Only one declaration is allowed.",nil,p.val);
        return nil;
      }
      // User specified a type, so use it directly.
      cur = lappend(cur,Ptree::qMake("case `index`: {\n"));
      // If we have a loop, we copy the handle's second item to the loopvar so
      // that it can be used by the body of the code.
      if (p.isloop() && !defaultblock) {
        cur = lappend(cur,Ptree::qMake("if (`uflag`) {\n"));
        if (p.needstack()) {
          cur = lappend(cur,Ptree::qMake("`p.loopvar` = `p.stack`[`handle`.second];\n"));
        } else {
          TypeInfo t = p.indextype;
          cur = lappend(cur,Ptree::qMake("`p.loopvar` = ((`t.MakePtree(0)`)`handle`.second);\n"));
        }
        cur = lappend(cur,Ptree::qMake("}\n"));
      }
      cur = lappend(cur,Ptree::qMake("`p.val` = (`p.chan`) `p.op` get();\n"
                                     "{\n"
                                     "`p.body`\n"
                                     "}\n"
                                     "} break;\n"));
    } else {
      // Simple case- no value, so just insert code.
      cur = lappend(cur,Ptree::qMake("case `index`: {\n"
                                     "`p.body`\n"
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

// Parses an alt's body.
bool Plasma::parseAltBody(Environment *env,Ptree *rest,PortVect &pv,Ptree* &defaultblock)
{
  Ptree *body;
  if(!Ptree::Match(rest, "[ %? ]", &body)){
    ErrorMessage(env, "invalid alt statement", nil, rest);
    return false;
  }

  // Ignore the braces.
  body = body->Cadr();

  // Parse the body of the alt.
  if (!parseAltBlock(env,body,false,pv,defaultblock)) {
    return false;
  }

  return true;
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

// Parses an afor block.
bool Plasma::parseAforBody(Environment *env,Ptree *rest,PortVect &pv,Ptree* &defaultblock)
{
  Ptree *s1, *s2, *s3, *body;

  if(!Ptree::Match(rest, "[ ( %? %? ; %? ) %? ]", &s1,&s2,&s3,&body)) {
    ErrorMessage(env, "invalid afor statement", nil, rest);
    return false;
  }

  int pvsize = pv.size();

  // Ignore the braces.
  body = body->Cadr();
  
  if (!parseAltBlock(env,body,true,pv,defaultblock)) {
    return false;
  }

  if ((int)pv.size() > pvsize+1) {
    ErrorMessage(env,"more than one port statement in an afor block is currently not supported",nil,rest);
    return false;
  }

  Port &p = pv.back();
  // Store afor information in channel object.
  p.s1 = s1;
  p.s2 = s2;
  p.s3 = s3;

  // We have to extract the iterator from the first statement in the afor loop.
  // This means that it somewhat limits the generality of the for-loop style statement.
  // We may have to address this issue later.
  VarWalker *vw = new VarWalker(env->GetWalker()->GetParser(),env);
  if (!parseAforCondition(vw,env,s1,s3)) {
    return false;
  }

  const ArgVect &argnames = vw->argnames();
  // We just take the first declaration and assume that's the index variable for afor.
  p.loopvar = argnames.front()._arg;
  p.indextype = argnames.front()._type;
  // This controls whether we can put the value into the channel's handle or if
  // we need an auxiliary stack.
  bool needstack = true;
  if (int bt = p.indextype.IsBuiltInType()) {
    if (bt & (CharType | IntType | ShortType | LongType | BooleanType)) {
      needstack = false;
    }
  }
  if (needstack) {
    p.stack  = Ptree::GenSym();
  }

  return true;
}

// Scans for alt, afor, or port constructs.
bool Plasma::parseAltBlock(Environment *env,Ptree *body,bool isloop,PortVect &pv,Ptree* &defaultblock)
{
  // Walk through the statements in the alt block.  They may be
  // either a port, alt, afor, or default block.
  for (PtreeIter i = body; !i.Empty(); ++i) {
    Ptree *pchan,*pop,*pval,*pbody;
    if (Ptree::Match(*i,"[ %? %? port ( %? ) %? ]",&pchan,&pop,&pval,&pbody)) {
      pv.push_back(Port(isloop,pchan,pop,pval,pbody));
    }
    else if ((*i)->Car()->Eq("alt")) {
      parseAltBody(env,(*i)->Cdr(),pv,defaultblock);
    }
    else if ((*i)->Car()->Eq("afor")) {
      parseAforBody(env,(*i)->Cdr(),pv,defaultblock);
    }
    else if (Ptree::Match(*i,"[ { %* } ]")) {
      if (defaultblock) {
        ErrorMessage(env, "bad alt/afor block:  Only one default block is allowed.",nil,*i);
        return false;
      }
      defaultblock = *i;
    } else {
      ErrorMessage(env, "bad alt/afor block:  Only port, afor, alt, or a default block are allowed within an alt/afor block.",nil,*i);
      return false;
    }
  }

  if (pv.empty()) {
    ErrorMessage(env,"bad alt/afor block:  You must have at least one port or default block.",nil,body);
    return false;
  }
  return true;
}
