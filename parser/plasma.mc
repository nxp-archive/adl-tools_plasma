
#include <assert.h>
#include <iostream>
#include <vector>
#include "opencxx/mop.h"
#include "opencxx/ptree-core.h"
#include "opencxx/walker.h"

using namespace std;

struct ArgPair {
  Ptree *_arg;
  bool   _ref;
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
    _penv(e->GetOuterEnvironment()), 
    _benv(e->GetBottom())
  {}
  virtual Ptree* TranslateVariable(Ptree*);
  void reset(Ptree *tst,Ptree *tsn) { _tsname = tsn; _tstype = tst; _args = nil; _argnames.clear(); };

  Ptree   *args() const { return _args; };
  const ArgVect &argnames() const { return _argnames; };

private:
  Ptree       *_tsname;  // Name of thread structure.
  Ptree       *_tstype;  // Type of thread structure.

  Environment *_penv;    // Parent scope.
  Environment *_benv;    // Bottom (global) scope.
  Ptree       *_args;
  ArgVect      _argnames;
};

// Look for the variable in all environments from the current up to,
// but not including, the parent environment.  If not found, we consider
// this to be a needed argument and add it to the _args list.  Note:  We
// ignore things which are declared in the outermost scope.  This eliminates
// globals and functions.
Ptree *VarWalker::TranslateVariable(Ptree *exp)
{
  Environment *e = env;
  Bind *b;
  bool found = false;
  while (e != _penv) {
    if (!e->LookupTop(exp,b)) {
      e = e->GetOuterEnvironment();
    } else {
      found = true;
      break;
    }
  }
  if (!found) {
    // Not found, so search up to, but not including, the global scope.
    while (e != _benv) {
      if (!e->LookupTop(exp,b)) {
        e = e->GetOuterEnvironment();
      } else {
        found = true;
        break;
      }
    }
    if (found && !b->IsType()) {
      TypeInfo t;
      b->GetType(t,e);
      // You can't have pointers to references, so if it's a reference, we treat it
      // as is.
      if (t.IsReferenceType()) {
        _argnames.push_back(ArgPair(exp,true));
        _args = Ptree::Cons(t.MakePtree(Ptree::qMake("`exp`")),_args);
        return Ptree::qMake("((`_tstype`*)`_tsname`)->`exp`");
      } else {
        _argnames.push_back(ArgPair(exp,false));
        _args = Ptree::Cons(t.MakePtree(Ptree::qMake("*`exp`")),_args);
        return Ptree::qMake("*(((`_tstype`*)`_tsname`)->`exp`)");
      }
    }
  }
  return exp;
}

class Plasma : public Class {
public:
  Ptree* TranslateUserPlain(Environment*,Ptree*, Ptree*);
  static bool Initialize();

private:
  void makeThreadStruct(Environment *env,Ptree *type,Ptree *args);
};

bool Plasma::Initialize()
{
  SetMetaclassForPlain("Plasma");
  RegisterNewBlockStatement("par");
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
  l->SetCdr(Ptree::List(x));
  return l->Cdr();
}

Ptree* Plasma::TranslateUserPlain(Environment* env,Ptree* keyword, Ptree* rest)
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
    Ptree *tstype = Ptree::GenSym();
    Ptree *tsname = Ptree::GenSym();
    Ptree *thname = Ptree::GenSym();
    thnames = Ptree::Cons(thname,thnames);
    vw->reset(tstype,tsname);
    Ptree *expr = vw->Translate(*i);
    Ptree *args = vw->args();    
    makeThreadStruct(env,tstype,args);
    Ptree *nfname = Ptree::GenSym();
    const string &arglist = argList(vw->argnames());
    InsertBeforeToplevel(env,Ptree::qMake("void `nfname`(void *`tsname`);\n"));
    AppendAfterToplevel(env,Ptree::qMake("void `nfname`(void *`tsname`) {\n`TranslateExpression(env,expr)`\n}\n"));
    cur = lappend(cur,Ptree::qMake("`tstype` `tsname` = {`arglist.c_str()`};\n"
                                         "Thread *`thname` = pSpawn(`nfname`,&`tsname`);\n"));
  }

  // Now iterate over created threads, creating wait statements.
  for (PtreeIter i = thnames; !i.Empty(); ++i) {
    cur = lappend(cur,Ptree::qMake("pWait(`*i`);\n"));
  }

  cur->SetCdr(Ptree::List(Ptree::Make("// End par block.\n")));

  return start;
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
