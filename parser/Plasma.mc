
#include <assert.h>
#include <iostream>
#include "opencxx-bk/mop.h"
#include "opencxx-bk/ptree-core.h"

#include "VarWalker.h"
#include "Plasma.h"
#include "LdLeaf.h"

using namespace std;

// A line directive has to start at the beginning of a line, so
// we always include a newline, then dump out the text.
int LdLeaf::Write(ostream &out, int indent)
{
  char* ptr = data.leaf.position;
  int len = data.leaf.length;
  out << '\n';
  out.write(ptr,len);
  return 2;
}

// Given an environment and a ptree item, returns a filename and line number.
// This will only work correctly if you give it a chunk of code from the original
// program- it dives down until it finds the first leaf, then it uses that to get
// the information.
Ptree *getLineNumber(Environment *env,Ptree *expr,int &line)
{
  while (expr && !expr->IsLeaf()) {
    expr = expr->Car();
  }
  return env->GetLineNumber(expr,line);
}

// Returns a line directive with the program location pointed to by expr.
// Note:  Expr must come from the original program for this to work correctly.
Ptree *lineDirective(Environment *env,Ptree *expr)
{
  int line;
  Ptree *fn = getLineNumber(env,expr,line);
  Ptree *p = Ptree::qMake("# `line` \"`fn`\"\n");
  char *str = p->ToString();
  return new LdLeaf(str,strlen(str));
}

// It should be valid to do this b/c there should only be one instance
// of the Plasma object.
void Plasma::InitializeInstance(Ptree*, Ptree*)
{
  VarWalker::setPlasma(this);
}

bool Plasma::Initialize()
{
  SetMetaclassForPlain("Plasma");
  RegisterNewBlockStatement("par");
  RegisterNewForStatement("pfor");
  RegisterNewWhileStatement("on");
  RegisterNewBlockStatement("alt");
  RegisterNewBlockStatement("prialt");
  RegisterNewForStatement("afor");
  RegisterNewForStatement("priafor");
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
  else if (keyword->Eq("prialt")) {
    return TranslatePriAlt(env,keyword,rest);
  }
  else if (keyword->Eq("afor") || keyword->Eq("priafor")) {
    return TranslateAfor(env,keyword,rest);
  }

  ErrorMessage(env, "unknown user statement encountered",nil,keyword);
  return nil;
}

void dumpEnv(Environment *env)
{
  int i = 0;
  cerr << "-------------" << endl;
  while (env) {
    cerr << i << ":  " << env << ":  ";
    Environment *nenv = env->GetOuterEnvironment();
    if (!nenv) {
      cerr << "<outer>" << endl;
    } else {
      env->Dump();
    }
    env = nenv;
    ++i;
  }
  cerr << "-------------" << endl;
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

  //  dumpEnv(env);

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

  // Bit of a hack here- if we have a on-block, we need to remove the outer
  // brackets so that convertToThread will see it.  Otherwise, we have to keep
  // the braces, so that it will be handled as a single expression and be
  // properly translated.
  //  if (body->Second()->Ca_ar()->Eq("on")) {
  if (Ptree::Match(body,"[ { [ [ on %_ ] ] } ]")) {
    body = body->Cadr()->Car();
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

  // Is this an empty statement?  If so, skip.
  if (!expr->Car()) {
    return;
  }

  // Do we have a placement statement?  If so, proc will store the expression
  // which specifies the processor.  If not, this will be nil, so the evaluation
  // will produce an empty string.
  Ptree *proc = nil,*pri = nil,*tproc = nil,*tpri = nil,*tbody = nil,*onarg = nil;
  if (Ptree::Match(expr,"[ on ( %? ) %? ]",&onarg,&tbody)) {
    // Check to see if we have two arguments.  If so, first is the
    // processor, second is the priority.  Otherwise, priority defaults
    // to -1, which means priority of current thread.
    if (Ptree::Match(onarg,"[ %? , %? ]",&tproc,&tpri)) {
      pri = tpri;
    } else {
      tproc = onarg;
    }
    expr = tbody;
    proc = Ptree::qMake("(`tproc`)(),");
  }
  if (!pri) {
    pri = Ptree::Make("-1");
  }

  // We must do this after checking for the on-statement, since "on" is not valid C++
  // and the translation step would thus return nil in error.
  vw->setnames(tstype,tsname);              
  Ptree *nexpr = vw->Translate(expr);    // Translate expression, scanning for variables.
  Ptree *args = vw->args();

  // Register this variable so that nested par blocks will see the parm structure.
  Class *tsclass = new Class(env,tstype);
  env->RecordPointerVariable(tsname->ToString(),tsclass);

  thnames = Ptree::Cons(thname,thnames); // Add on to list of thread names.

  // We need to insert anything in the bottom-most environment to ensure
  // that it gets added.  This is a problem if we come across Plasma code in
  // a class declaration.
  Environment *benv = env->GetBottom();

  if (args) {
    // Arguments exist- we have to create a structure in which
    // to pass them.
    makeThreadStruct(benv,tstype,args,vw->argnames());     // Create the argument structure.
    const string &arglist = argList(vw->argnames());
    // We insert a prototype before, and the actual function after, the current location
    // because we want to handle the case where the thread calls the current function recursively.
    InsertBeforeToplevel(benv,Ptree::qMake("void `nfname`(void *`tsname`);\n"));
    AppendAfterToplevel(benv,Ptree::List(Ptree::qMake("void `nfname`(void *`tsname`) {\n"),
                                         lineDirective(env,expr),
                                         Ptree::qMake("`TranslateExpression(env,nexpr)`\n}\n")));
    if (onthread) {
      elist = lappend(elist,Ptree::qMake("`tstype` `tsname` = {`arglist.c_str()`};\n"
                                         "plasma::THandle `thname` = plasma::pSpawn(`proc` `nfname`,sizeof(`tstype`),&`tsname`,`pri`).first;\n"));
    } else {
      elist = lappend(elist,Ptree::qMake("`tstype` `tsname` = {`arglist.c_str()`};\n"
                                         "plasma::THandle `thname` = plasma::pSpawn(`proc` `nfname`,&`tsname`,`pri`);\n"));
    }
  } else {
    // No arguments, so no need to create the structure.
    InsertBeforeToplevel(benv,Ptree::qMake("void `nfname`(void *);\n"));
    AppendAfterToplevel(benv,Ptree::List(Ptree::qMake("void `nfname`(void *) {\n"),
                                         lineDirective(env,expr),
                                         Ptree::qMake("`TranslateExpression(env,nexpr)`\n}\n")));
    elist = lappend(elist,Ptree::qMake("plasma::THandle `thname` = plasma::pSpawn(`proc` `nfname`,0,`pri`);\n"));
  }
}

// This constructs and inserts a thread structure only if we don't have one that's large enough.
void Plasma::makeThreadStruct(Environment *env,Ptree *type,Ptree *args,const ArgVect &av)
{
    Ptree *front = Ptree::List(Ptree::qMake("struct `type` {\n"));
    Ptree *cur = front;
    for(PtreeIter i = args; !i.Empty(); i++){
      cur = lappend(cur,Ptree::qMake("  `*i`;\n"));
    }
    cur = lappend(cur,Ptree::qMake("};\n"));

    // Insert any needed forward defines first.
    for (ArgVect::const_iterator i = av.begin(); i != av.end(); ++i) {
      if (i->_fwd) {
        InsertBeforeToplevel(env,Ptree::qMake("`i->_fwd`;\n"));
      }
    }
    
    // Insert thread structure.
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

  // Default priority is the same priority as the calling thread.
  Ptree *priority = Ptree::Make("-1");

  // Extract arguments.
  switch (args->Length()) {
  case 1:
    // No action taken- we will use the current priority.
    args = args->First();
    break;
  case 3:
    // Extract priority from second argument (which is 3rd element in
    // list due to the comma).
    priority = args->Third();
    args = args->First();
    break;
  default: {
    ErrorMessage(env,"The spawn operator requires one or two argument.",nil,spawnobj);
    return nil;
  }
  }

  // User function and its arguments.
  Ptree *ufunc,*uargs;

  // Try to extract a function call.
  if (!Ptree::Match(args,"[ %? ( %? ) ]",&ufunc,&uargs)) {
    Ptree *uop,*uptr;
    // For some reason, member pointers parse differently than member calls.
    // So we convert it here to the same form:  obj <op> member.
    if (Ptree::Match(args,"[ %? %? [ %? ( %? ) ] ]",&ufunc,&uop,&uptr,&uargs)) {
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

  Environment *benv = env->GetBottom();

  // Create temporary argument storage structure.
  if (!makeSpawnStruct(benv,objclass,t,targs,ufunc)) {
    return nil;
  }

  // Create spawn function- this will take the function and its arguments
  // and launch a thread, returning a result structure w/a pointer to the
  // result data.
  if (!makeSpawnFunc(benv,objclass,t,targs,ufunc,lfunc,sfunc,(proc != nil))) {
    return nil;
  }

  // Finally, replace spawn operator w/call to spawn function.
  Ptree *start = Ptree::qMake("`sfunc`(");
  Ptree *cur = start;

  bool fptr = isPtrType(t);

  // If we have a specified processor, pass as argument.
  if (proc) {
    cur = lappend(cur,Ptree::qMake("`proc`,"));
  }

  // If we have an explicit object, pass as argument.
  if (objptr) {
    cur = lappend(cur,Ptree::qMake("`objptr`,"));
  }

  // If we have a pointer to a member or function, pass as argument.
  if (fptr) {
    cur = lappend(cur,Ptree::qMake("`ufunc`,"));
  }

  // Pass the priority.
  cur = lappend(cur,priority);
  if (uargs) {
    cur = lappend(cur,Ptree::Make(","));
  }

  // Finally, pass function arguments.
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

Ptree *priName()
{
  return Ptree::Make("priority");
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
  if (rt.IsBuiltInType() == VoidType) {
    // If we get a void function, we treat it as an integer.
    cur = lappend(cur,Ptree::qMake("int `resultName()`;\n"));
  } else {
    cur = lappend(cur,Ptree::qMake("`rt.MakePtree(resultName())`;\n"));
  }

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
  AppendAfterToplevel(env,start);
  return true;
}

// Creates the launcher function and spawn function for a spawn operator.
bool Plasma::makeSpawnFunc(Environment *env,Class *objclass,TypeInfo t,Ptree *targs,
                           Ptree *ufunc,Ptree *lfunc,Ptree *sfunc,bool proc)
{
  TypeInfo origt = t;
  bool fptr = false;
  if (isPtrType(t)) {
    fptr = true;
    t.Dereference();
  }

  // If the return type is void, set it to an integer, for now.
  bool isvoid = false;
  TypeInfo rt;
  t.Dereference(rt);
  if (rt.IsBuiltInType() == VoidType) {
    rt.SetInt();
    isvoid = true;
  }

  // Create the launcher function which calls the user function w/proper args.
  Ptree *start = Ptree::qMake("inline void `lfunc`(void *args)\n{\n");
  Ptree *cur = start;

  // When we call the user function, we either want to same the result (normal
  // case), or simply set it to 0- the latter is only applicable for void
  // functions.  Sinc they do not return a value, we fake it by considering it
  // to be an integer return which always has a value of 0.
  if (isvoid) {
    cur = lappend(cur,Ptree::qMake("  ((`targs`*)args)->`resultName()` = 0;\n  "));
  } else {
    cur = lappend(cur,Ptree::qMake("  ((`targs`*)args)->`resultName()` = "));
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
  AppendAfterToplevel(env,start);

  // Create the spawn function which launches the thread and returns
  // the result template.
  Ptree *rtemplate = Ptree::qMake("plasma::Result<`rt.MakePtree(0)`>");
  start = Ptree::qMake("`rtemplate` `sfunc`(");
  cur = start;

  // If we have a processor, we need it as an argument.
  if (proc) {
    cur = lappend(cur,Ptree::qMake("plasma::Proc *`procName()`,"));
  }

  // If we have an explicit object, we need it as an argument.
  if (objclass) {
    cur = lappend(cur,Ptree::qMake("`objclass->Name()` *`className()`,"));
  }

  // If we have a pointer to the function or method, it is an argument.
  if (fptr) {
    cur = lappend(cur,Ptree::qMake("`origt.MakePtree(ptrName())`,"));
  }

  // The priority of the thread.
  cur = lappend(cur,Ptree::qMake("int `priName()`"));
  if (numargs) {
    cur = lappend(cur,Ptree::qMake(","));
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

  cur = lappend(cur,Ptree::Make(")"));

  // Add forward declaration of object, if necessary.
  if (objclass) {
    InsertBeforeToplevel(env,Ptree::qMake("class `objclass->Name()`;\n"));
  }

  // Add forward declaration- copy list so that we only get the decl.
  Ptree *fwddecl = start;
  InsertBeforeToplevel(env,Ptree::qMake("`fwddecl`;\n"));

  start = Ptree::qMake("inline `Ptree::CopyList(fwddecl)` ");
  cur = start;

  // Initialization of the argument passing structure.
  Ptree *emptyarg = Ptree::Make("()");
  cur = lappend(cur,Ptree::qMake("\n{\n"
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
  cur = lappend(cur,Ptree::qMake("`lfunc`,sizeof(`targs`),&args,`priName()`));\n}\n"));
  
  AppendAfterToplevel(env,start);

  return true;
}

// Translate an alt or prialt block.
Ptree* Plasma::TranslateAlt(Environment* env,Ptree* keyword, Ptree* rest,bool reverse)
{
  PortList pv;
  Ptree *defaultblock = nil;

  if (!parseAltBody(env,rest,pv,defaultblock,reverse)) {
    return false;
  }

  return generateAltBlock(env,pv,defaultblock);
}

// Translate an alt block.
Ptree* Plasma::TranslateAlt(Environment* env,Ptree* keyword, Ptree* rest)
{
  return TranslateAlt(env,keyword,rest,true);
}

// Translate a prialt block.
Ptree* Plasma::TranslatePriAlt(Environment* env,Ptree* keyword, Ptree* rest)
{
  return TranslateAlt(env,keyword,rest,false);
}

// Translate an afor block.  This is similar to an alt block, except that
// we loop over the port statements.
Ptree* Plasma::TranslateAfor(Environment* env,Ptree* keyword, Ptree* rest)
{
  PortList pv;
  Ptree *defaultblock = nil;

  if (!parseAforBody(env,rest,pv,defaultblock)) {
    return false;
  }

  return generateAltBlock(env,pv,defaultblock);
}

// Does a depth-first of origpv, placing a flattened
// list of ports into pv.
void flatten(PortList &pv,const PortList &origpv)
{
  for (PortList::const_iterator i = origpv.begin(); i != origpv.end(); ++i) {
    if (i->isport()) {
      pv.push_back(&(i->port()));
    } else {
      flatten(pv,i->list());
    }
  }
}

// Main generation function for alt/afor blocks.  Given a list of
// ports, constructs the necessary alt structure.
Ptree *Plasma::generateAltBlock(Environment *env,const PortList &origpv,Ptree *defaultblock)
{
  Ptree *label  = Ptree::GenSym();
  Ptree *handle = Ptree::GenSym();
  Ptree *sindex = Ptree::GenSym();
  Ptree *loop  = Ptree::GenSym();

  PortList pv;
  flatten(pv,origpv);

  // We put this inside of a try block if we don't have a default block,
  // since we want to clear the notifications if an exception occurs.
  Ptree *start = (!defaultblock) ? 
    Ptree::List(Ptree::Make("try {\n")) :
    Ptree::List(Ptree::Make("{\n"));
  Ptree *cur = start;

  cur = lappend(cur,Ptree::qMake("plasma::pLock();\n"));

  // Start of main loop.
  cur = lappend(cur,Ptree::qMake("`loop`:\n"));

  // For each afor entry, write out initial statement loop statement.
  // Create auxiliary stack if needed.
  bool needsindex = false;
  bool haveloops = false;
  for (PortList::const_iterator i = pv.begin(); i != pv.end(); ++i) {
    const Port &p = i->port();
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

  // Create index variable.
  cur = lappend(cur,Ptree::qMake("int `handle`;\n"));

  // If needed, create index variable for use w/auxiliary stacks.
  if (needsindex) {
    cur = lappend(cur,Ptree::qMake("int `sindex`;\n"));
  }

  // For each entry, write out ready-query logic.
  int index = 0;
  for (PortList::const_iterator iter = pv.begin(); iter != pv.end(); ++iter,++index) {
    const Port &p = iter->port();
    cur = lappend(cur,Ptree::qMake("`handle` = `index`;\n"));
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
    cur = lappend(cur,Ptree::qMake("`handle` = `(int)(pv.size())`;\n"));
  } else {
    // This generated code is only reached if no ports were ready.
    // In that case, we set each channel to notify us when it has data.
    int index = 0;
    for (PortList::const_iterator iter = pv.begin(); iter != pv.end(); ++iter,++index) {
      const Port &p = iter->port();
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

    // Next, we sleep.  When we wake up, we get the handle of the channel
    // which awoke us.
    cur = lappend(cur,Ptree::qMake("plasma::pSleep();\n"
                                   "plasma::pLock();\n"));

    // Clear all notification, since we have a value.
    for (PortList::const_iterator iter = pv.begin(); iter != pv.end(); ++iter) {
      const Port &p = iter->port();
      if (p.isloop()) {
        cur = lappend(cur,Ptree::qMake("for (`p.s1` `p.s2` ; `p.s3`) {\n"
                                       "(`p.chan`) `p.op` clear_notify();\n"
                                       "}\n"));        
      } else {
        cur = lappend(cur,Ptree::qMake("(`p.chan`) `p.op` clear_notify();\n"));
      }
    }
  }

  // End of main loop, if we have no default block.  If we have a default block,
  // then we fall through.
  if (!defaultblock) {
    cur = lappend(cur,Ptree::qMake("goto `loop`;\n"));
  }

  if ( (cur = generateAltBody(env,cur,label,handle,pv,defaultblock)) == nil) {
    return nil;
  }

  cur = lappend(cur,Ptree::Make("}\n"));

  // Exception cleanup code if no default block.
  if (!defaultblock) {
    cur = lappend(cur,Ptree::Make("catch (...) {\n"));
    for (PortList::const_iterator i = pv.begin(); i != pv.end(); ++i) {
      const Port &p = i->port();
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
Ptree *Plasma::generateAltBody(Environment *env,Ptree *cur,Ptree *label,Ptree *handle,
                               const PortList &pv,Ptree *defaultblock)
{
  // Unlock processors- may be redundant for some cases, but is needed
  // for default blocks and non-standard channels.
  // Jump to the code for the relevant handle.
  cur = lappend(cur,Ptree::qMake("`label`:\n"
                                 "switch(`handle`) {\n"));

  // Handling code.  Each value should be a valid declaration.
  // The second statement represents the channel to be queried.
  int index = 0;
  for (PortList::const_iterator iter = pv.begin(); iter != pv.end(); ++iter,++index) {
    const Port &p = iter->port();
    cur = lappend(cur,Ptree::qMake("case `index`: {\n"));
    if (p.val) {
      if (p.val->Length() > 1) {
        ErrorMessage(env,"Invalid port statement:  Only one declaration is allowed.",nil,p.val);
        return nil;
      }
      // User specified a type, so use it directly.
      cur = lappend(cur,Ptree::qMake("`p.val` = (`p.chan`) `p.op` get();\n"
                                     " plasma::pUnlock();\n"));
      // Add it to the environment so that we can translate the body and it
      // will recognize it.
      Class *pclass = new Class(env,p.val->Ca_ar());
      env->RecordVariable(p.val->Car()->Second()->Car()->ToString(),pclass);
    } else {
      // Simple case- no value.  We still have to call get in order to drain the value,
      // but we don't assign it to anything.
      cur = lappend(cur,Ptree::qMake("(`p.chan`) `p.op` get();\n"));
    }
    cur = lappend(cur,Ptree::qMake("{\n"));
    cur = lappend(cur,lineDirective(env,p.body));
    cur = lappend(cur,Ptree::qMake("`TranslateExpression(env,p.body)`\n"
                                   "} } break;\n"));
  }

  // Write the default block code, if present.
  if (defaultblock) {
    cur = lappend(cur,Ptree::qMake("case `(int)(pv.size())`: {\n"
                                   " plasma::pUnlock();\n"));
    cur = lappend(cur,lineDirective(env,defaultblock));
    cur = lappend(cur,Ptree::qMake("`TranslateExpression(env,defaultblock)`\n"
                                   "} break;\n"));
  }

  // End of switch statement.
  cur = lappend(cur,Ptree::Make("}\n"));

  return cur;
}

// Parses an alt or prialt body.
bool Plasma::parseAltBody(Environment *env,Ptree *rest,PortList &pv,
                          Ptree* &defaultblock,bool reverse)
{
  Ptree *body;
  if(!Ptree::Match(rest, "[ %? ]", &body)){
    ErrorMessage(env, "invalid alt statement", nil, rest);
    return false;
  }

  // Ignore the braces.
  body = body->Cadr();

  pv.push_back(PortNode());

  // Parse the body of the alt.
  if (!parseAltBlock(env,body,false,pv.back().list(),defaultblock)) {
    return false;
  }

  if (reverse) {
    // Now reverse the elements just to jack with people:  They shouldn't
    // be depending upon it being in a particular order- prialt should be used for that.
    pv.back().list().reverse();
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
bool Plasma::parseAforBody(Environment *env,Ptree *rest,PortList &pv,Ptree* &defaultblock)
{
  Ptree *s1, *s2, *s3, *body;

  if(!Ptree::Match(rest, "[ ( %? %? ; %? ) %? ]", &s1,&s2,&s3,&body)) {
    ErrorMessage(env, "invalid afor statement", nil, rest);
    return false;
  }

  // Ignore the braces.
  body = body->Cadr();

  pv.push_back(PortNode());

  if (!parseAltBlock(env,body,true,pv.back().list(),defaultblock)) {
    return false;
  }

  if ((int)pv.back().list().size() > 1) {
    ErrorMessage(env,"more than one port statement in an afor block is currently not supported",nil,rest);
    return false;
  }

  Port &p = pv.back().list().back().port();
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
bool Plasma::parseAltBlock(Environment *env,Ptree *body,bool isloop,PortList &pv,Ptree* &defaultblock)
{
  // Walk through the statements in the alt block.  They may be
  // either a port, alt, afor, or default block.
  for (PtreeIter i = body; !i.Empty(); ++i) {
    Ptree *pchan,*pop,*pval,*pbody;
    if (Ptree::Match(*i,"[ %? %? port ( %? ) %? ]",&pchan,&pop,&pval,&pbody)) {
      pv.push_back(new Port(isloop,pchan,pop,pval,pbody));
    }
    else if ((*i)->Car()->Eq("alt")) {
      parseAltBody(env,(*i)->Cdr(),pv,defaultblock,true);
    }
    else if ((*i)->Car()->Eq("prialt")) {
      parseAltBody(env,(*i)->Cdr(),pv,defaultblock,false);
    }
    else if ((*i)->Car()->Eq("afor") || (*i)->Car()->Eq("priafor")) {
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
