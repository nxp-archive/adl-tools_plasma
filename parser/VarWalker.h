//
// The VarWalker class traverses a Ptree, looking for variable
// instances.  It records these so that the user can figure out
// what variables are used within an expression.
//

#ifndef _VARWALKER_H_
#define _VARWALKER_H_

#include <vector>

#include "opencxx/walker.h"
# include <ext/hash_set>
using namespace __gnu_cxx;

struct str_equal {
  size_t operator()(const char*x,const char *y) const { return !strcmp(x,y); }
};

typedef hash_set<const char *,hash<const char *>,str_equal > StrHash;

struct ArgPair {
  Ptree   *_arg;
  bool     _ref; // true -> value or reference parm, false -> pointer.
  TypeInfo _type;
  ArgPair(Ptree *a,bool r,TypeInfo t) : _arg(a), _ref(r), _type(t) {};
};

typedef std::vector<ArgPair> ArgVect;

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
  virtual ~VarWalker() {};

  virtual Ptree* TranslateVariable(Ptree*);
  virtual Ptree* TranslateUserPlain(Ptree*);

  void handleGuard(bool hg) { _inguard = hg; };
  void setnames(Ptree *tst,Ptree *tsn) { 
    _tsname = tsn; 
    _tstype = tst;
    _tsvars.insert(tsn->ToString());
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
  Ptree *TranslateUserBlock(Ptree *s);
  Ptree *TranslateUserFor(Ptree *s);

  Ptree *recordVariable(Ptree *exp,Environment *e,Bind *b,bool found_inguard);

  Ptree       *_tsname;  // Name of thread structure.
  Ptree       *_tstype;  // Type of thread structure.

  Environment *_penv;    // Parent scope.
  Environment *_benv;    // Bottom (global) scope.
  Environment *_genv;    // Guard scope (declarations in the condition).
  Ptree       *_args;    // Stores members of the future thread structure.
  ArgVect      _argnames;// Stores argument information.
  StrHash      _vhash;   // Tracks variables we've seen already.
  bool         _inguard;

  static StrHash _tsvars; // Stores all thread-structure variables used so far.
};

#endif

