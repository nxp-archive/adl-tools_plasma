
#include <assert.h>
#include <iostream>
#include <vector>

#include "opencxx/mop.h"
#include "opencxx/ptree-core.h"

#include "VarWalker.h"

using namespace std;

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
  //cout << "Found variable " << name << ", type:  " << b->GetEncodedType() << endl;
  // You can't have pointers to references, so if it's a reference, we treat it
  // as is.  We also want to treat any loop variables as copy-by-value so that
  // we won't clobber it.
  if (t.IsReferenceType() || t.IsArray() || found_inguard) {
    if (!_vhash.count(name)) {
      _vhash.insert(name);
      _argnames.push_back(ArgPair(exp,true,t));
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
      _argnames.push_back(ArgPair(exp,false,t));
      _args = Ptree::Cons(t.MakePtree(Ptree::qMake("*`exp`")),_args);
    }
    return Ptree::qMake("(*(((`_tstype`*)`_tsname`)->`exp`))");
  }
}
