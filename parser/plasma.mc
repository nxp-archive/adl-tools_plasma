
#include <assert.h>
#include <iostream>
#include <vector>
#include "opencxx/mop.h"
#include "opencxx/ptree-core.h"
#include "opencxx/walker.h"

# include <ext/hash_set>
using namespace __gnu_cxx;

struct str_equal {
  size_t operator()(const char*x,const char *y) const { return !strcmp(x,y); }
};

typedef hash_set<const char *,hash<const char *>,str_equal > StrHash;

using namespace std;

struct ArgPair {
  Ptree *_arg;
  bool   _ref; // true -> value or reference parm, false -> pointer.
  ArgPair(Ptree *a,bool r) : _arg(a), _ref(r) {};
};

typedef vector<ArgPair> ArgVect;

// This walks over an expression.  For each variable declaration, if it's
// not declared in any scope under the parent scope, it's recorded as a
// needed argument.
class VarWalker : public Walker {
public:
  VarWalker(Parser *p,Environment *e) : 
    Walker(p,e),
    _tsname(0),
    _tstype(0),
    _penv(e->GetOuterEnvironment()), 
    _benv(e->GetBottom()),
    _args(0),
    _inguard(false)
  {}

  virtual Ptree* TranslateVariable(Ptree*);

  void handleGuard(bool hg) { _inguard = hg; };
  void setnames(Ptree *tst,Ptree *tsn) { 
    _tsname = tsn; 
    _tstype = tst;
  };
  void reset() { 
    _inguard = false;
    _args = nil; 
    _argnames.clear(); 
    _vhash.clear();
  };

  Ptree   *args() const { return _args; };
  const ArgVect &argnames() const { return _argnames; };

  void saveGuardEnv() { _genv = env; };

  bool guardEnvEmpty() const { return _genv->IsEmpty(); };

private:
  Ptree *recordVariable(Ptree *exp,Environment *e,Bind *b,bool found_inguard);

  Ptree       *_tsname;  // Name of thread structure.
  Ptree       *_tstype;  // Type of thread structure.

  Environment *_penv;    // Parent scope.
  Environment *_benv;    // Bottom (global) scope.
  Environment *_genv;    // Guard scope (declarations in the condition).
  Ptree       *_args;
  ArgVect      _argnames;
  StrHash      _vhash;
  bool         _inguard;
};

// Look for the variable in all environments from the current up to,
// but not including, the parent environment.  If not found, we consider
// this to be a needed argument and add it to the _args list.  Note:  We
// ignore things which are declared in the outermost scope.  This eliminates
// globals and functions.
Ptree *VarWalker::TranslateVariable(Ptree *exp)
{
  if (!_inguard) {
    //    cout << "Examining variable:  ";
    //    exp->Display2(cout);
    //    cout << endl;

    Environment *e = env;
    Bind *b;
    bool found = false;
    bool found_inguard = false;

    // This searches for a variable up to, but not including, the parent scope.
    while (e != _penv) {
      if (!e->LookupTop(exp,b)) {
        e = e->GetOuterEnvironment();
      } else {
        if (_genv && _genv->LookupTop(exp,b)) {
          // Special case- was the variable declared in the guard environment?
          // If so, then we make it a value parameter.
          found_inguard = true;
        }
        found = true;
        break;
      }
    }

    if (!found || found_inguard) {

      // Not found, so search up to, but not including, the global scope.
      if (!found_inguard) {
        while (e != _benv) {
          if (!e->LookupTop(exp,b)) {
            e = e->GetOuterEnvironment();
          } else {
            found = true;
            break;
          }
        }
      }
            
      if (found && !b->IsType()) {
        return recordVariable(exp,e,b,found_inguard);
      }
    }
  }
  return exp;
}

Ptree *VarWalker::recordVariable(Ptree *exp,Environment *e,Bind *b,bool found_inguard)
{
  TypeInfo t;
  b->GetType(t,e);
  const char *name = exp->ToString();
  //cout << "Found variable " << name << endl;
  // You can't have pointers to references, so if it's a reference, we treat it
  // as is.  We also want to treat any loop variables as copy-by-value so that
  // we won't clobber it.
  if (t.IsReferenceType() || t.IsArray() || found_inguard) {
    if (!_vhash.count(name)) {
      _vhash.insert(name);
      _argnames.push_back(ArgPair(exp,true));
      if (t.IsArray()) {
        t.Dereference();
        _args = Ptree::Cons(t.MakePtree(Ptree::qMake("*`exp`")),_args);
      } else {
        _args = Ptree::Cons(t.MakePtree(Ptree::qMake("`exp`")),_args);
      }
    }
    return Ptree::qMake("(((`_tstype`*)`_tsname`)->`exp`)");
  } else {
    if (!_vhash.count(name)) {
      _vhash.insert(name);
      _argnames.push_back(ArgPair(exp,false));
      _args = Ptree::Cons(t.MakePtree(Ptree::qMake("*`exp`")),_args);
    }
    return Ptree::qMake("(*(((`_tstype`*)`_tsname`)->`exp`))");
  }
}

class Plasma : public Class {
public:
  Ptree* TranslateUserPlain(Environment*,Ptree*, Ptree*);
  static bool Initialize();

private:
  Ptree* TranslatePar(Environment* env,Ptree* keyword, Ptree* rest);
  Ptree* TranslatePfor(Environment* env,Ptree* keyword, Ptree* rest);
  Ptree* TranslateAlt(Environment* env,Ptree* keyword, Ptree* rest);
  void makeThreadStruct(Environment *env,Ptree *type,Ptree *args);
  void convertToThread(Ptree* &elist,Ptree* &thnames,Ptree *expr,VarWalker *vw,
                       Environment *env,bool heapalloc);
};

bool Plasma::Initialize()
{
  SetMetaclassForPlain("Plasma");
  RegisterNewBlockStatement("par");
  RegisterNewForStatement("pfor");
  RegisterNewBlockStatement("alt");
  RegisterNewClosureStatement("port");
  RegisterNewBlockStatement("otherwise");
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

Ptree *lappend(Ptree *l,Ptree *x)
{
  if (!l) {
    return Ptree::List(x);
  } else {
    l->SetCdr(Ptree::List(x));
    return l->Cdr();
  }
}

inline bool compare(Ptree *p,const char *s)
{
  return !strncmp(p->GetPosition(),s,p->GetLength());
}

// This figures out what kind of user statement we have and dispatches to thee
// appropriate function.
Ptree* Plasma::TranslateUserPlain(Environment* env,Ptree* keyword, Ptree* rest)
{
  if (compare(keyword,"par")) {
    return TranslatePar(env,keyword,rest);
  }
  else if (compare(keyword,"pfor")) {
    return TranslatePfor(env,keyword,rest);
  }
  else if (compare(keyword,"alt")) {
    return TranslateAlt(env,keyword,rest);
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

  return Ptree::qMake("TVect `tvname`;\n"
                      "for (`s1` `s2` ; `s3`) { `start` `tvname`.push_back(`thn`); }\n"
                      "for (TVect::iterator `iname` = `tvname`.begin(); `iname` != `tvname`.end(); ++`iname`) { pWait(*`iname`); }\n"
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
                                         "Thread *`thname` = pSpawn(`nfname`,sizeof(`tstype`),&`tsname`);\n"));
    } else {
      elist = lappend(elist,Ptree::qMake("`tstype` `tsname` = {`arglist.c_str()`};\n"
                                         "Thread *`thname` = pSpawn(`nfname`,&`tsname`);\n"));
    }
  } else {
    // No arguments, so no need to create the structure.
    InsertBeforeToplevel(env,Ptree::qMake("void `nfname`(void *);\n"));
    AppendAfterToplevel(env,Ptree::qMake("void `nfname`(void *) {\n`TranslateExpression(env,nexpr)`\n}\n"));
    elist = lappend(elist,Ptree::qMake("Thread *`thname` = pSpawn(`nfname`,0);\n"));
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

struct Port {
  Ptree *chan;
  Ptree *val;
  Ptree *body;
  Port(Ptree *c,Ptree *v,Ptree *b) : chan(c), val(v), body(b) {};
};

typedef vector<Port> PortVect;

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

  // Walk through the statements in the alt block.  Each should be a 
  // case statement or a default statement.
  for (PtreeIter i = body; !i.Empty(); ++i) {
    Ptree *cguard,*cbody;    
    if (Ptree::Match(*i,"[ port ( %? ) %? ]",&cguard,&cbody)) {
      Ptree *chan,*val;
      if (Ptree::Match(cguard,"[ %? , %? ]",&chan,&val)) {
        pv.push_back(Port(chan,val,cbody));
      } 
      else if (Ptree::Match(cguard,"[ %? ]",&chan)) {
        pv.push_back(Port(chan,0,cbody));
      } 
      else {
        ErrorMessage(env, "bad port statement:  Expected either a channel and value or just a channel.",nil,*i);        
        return nil;
      }
    }
    else if (Ptree::Match(*i,"[ { %* } ]")) {
      defaultblock = *i;
    } else {
      ErrorMessage(env, "bad alt statement:  Only case or default statements are allowed within the block.",nil,*i);
      return nil;
    }
  }

  if (pv.empty()) {
    ErrorMessage(env,"bad alt statement:  You must have at least one port or default block.",nil,keyword);
    return nil;
  }

  Ptree *iname = Ptree::GenSym();  
  Ptree *label = Ptree::GenSym();
  Ptree *start = Ptree::List(Ptree::qMake("{\n"
                                     "pLock();\n"
                                     "int `iname`;\n"));
  Ptree *cur = start;

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
      cur = lappend(cur,Ptree::qMake("(`port.chan`).set_notify(pCurThread(),`index`);\n"));
    }

    // Next, we sleep.  When we wake up, we get the handle of the channel
    // which woke us.
    cur = lappend(cur,Ptree::qMake("`iname` = pSleep();\n"
                                   "pLock();\n"));

    // Clear all notification, since we have a value.
    for (unsigned index = 0; index != pv.size(); ++index) {
      const Port &port = pv[index];
      cur = lappend(cur,Ptree::qMake("(`port.chan`).clear_notify();\n"));
    }

  }

  // Jump to the code for the relevant handle.
  cur = lappend(cur,Ptree::qMake("`label`:\n"
                             "switch(`iname`) {\n"));

  Ptree *readname = Ptree::Make("read");

  for (int index = 0; index != (int)pv.size(); ++index) {
    const Port &port = pv[index];
    if (port.val) {
      // We only need to query the read member if the user wants
      // to map the read value to a variable.
      TypeInfo t;
      if (!env->Lookup(port.chan->Car(),t)) {
        ErrorMessage(env,"Channel not found.",nil,port.chan);
        return nil;
      }
      Class *mc = 0;
      if (t.IsReferenceType()) {
        t.Dereference();
      }
      if (!t.IsClass(mc)) {
        ErrorMessage(env,"Channel must be a class object.",nil,port.chan);
        return nil;
      }
      assert(mc);
      Member member;
      if (!mc->LookupMember(readname,member)) {
        ErrorMessage(env,"Bad channel:  No read() member found.",nil,port.chan);
        return nil;
      }
      if (!member.IsFunction()) {
        ErrorMessage(env,"Bad channel:  read() member is not a function.",nil,port.chan);
        return nil;
      }
      TypeInfo rt;
      member.Signature(rt);
      rt.Dereference();
      cur = lappend(cur,Ptree::qMake("case `index`: {\n"
                                     "`rt.MakePtree(port.val)` = (`port.chan`).get();\n"
                                     "{\n"
                                     "`port.body`\n"
                                     "}\n"
                                     "} break;\n"));
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

  cur = lappend(cur,Ptree::Make("}\n}\n"));

  return start;
}
