
#include <assert.h>
#include <iostream>
#include <vector>

#include "opencxx/mop.h"
#include "opencxx/ptree.h"

#include "VarWalker.h"

using namespace std;

StrHash VarWalker::_tsvars;

// This derived class exists so that the double-dispatch mechanism will
// end up calling TranslateVariable.  By default, you generally just get
// a character string, which won't work when there are nested par blocks.
class TsLeaf : public Leaf {
public:
  TsLeaf(Ptree *p) : Leaf(p->GetPosition(),p->GetLength()) {};
  Ptree* Translate(Walker*);
  void Typeof(Walker*, TypeInfo&);
};

Ptree* TsLeaf::Translate(Walker* w)
{
    return w->TranslateVariable(this);
}

void TsLeaf::Typeof(Walker* w, TypeInfo& t)
{
    w->TypeofVariable(this, t);
}

// This is just a list which calls translate on its constituent items.
class TsList : public NonLeaf {
public:
  TsList(Ptree *p,Ptree *q) : NonLeaf(p,q) {};
  Ptree *Translate(Walker *);

  static Ptree* List();
  static Ptree* List(Ptree*);
  static Ptree* List(Ptree*, Ptree*);
  static Ptree* List(Ptree*, Ptree*, Ptree*);
  static Ptree* List(Ptree*, Ptree*, Ptree*, Ptree*);
};

Ptree *TsList::Translate(Walker *w)
{
  Ptree *p1 = Car();
  Ptree *p2 = w->Translate(p1);
  Ptree *q1 = Cdr();
  Ptree *q2 = w->Translate(q1);
  if (p1 == p2 && q1 == q2) {
    return this;
  } else {
    return new TsList(p2,q2);
  }
}

Ptree* TsList::List()
{
    return nil;
}

Ptree* TsList::List(Ptree* p)
{
    return new TsList(p, nil);
}

Ptree* TsList::List(Ptree* p, Ptree* q)
{
    return new TsList(p, new TsList(q, nil));
}

Ptree* TsList::List(Ptree* p1, Ptree* p2, Ptree* p3)
{
    return new TsList(p1, new TsList(p2, new TsList(p3, nil)));
}

Ptree* TsList::List(Ptree* p1, Ptree* p2, Ptree* p3, Ptree* p4)
{
    return new TsList(p1, List(p2, p3, p4));
}


Ptree *VarWalker::TranslateUserBlock(Ptree *s)
{
  Ptree *body = s->Second();
  Ptree *body2 = Translate(body);
  if (body == body2) {
    return s;
  } else {
    Ptree *rest = Ptree::ShallowSubst(body2,body,s->Cdr());
    return new PtreeUserPlainStatement(s->Car(),rest);
  }
}

Ptree *VarWalker::TranslateUserFor(Ptree *s)
{
    NewScope();
    Ptree* exp1 = s->Third();
    Ptree* exp1t = Translate(exp1);
    Ptree* exp2 = s->Nth(3);
    Ptree* exp2t = Translate(exp2);
    Ptree* exp3 = s->Nth(5);
    Ptree* exp3t = Translate(exp3);
    Ptree* body = s->Nth(7);
    Ptree* body2 = Translate(body);
    ExitScope();

    if(exp1 == exp1t && exp2 == exp2t && exp3 == exp3t && body == body2)
	return s;
    else{
	Ptree* rest = Ptree::ShallowSubst(exp1t, exp1, exp2t, exp2,
					  exp3t, exp3, body2, body, s->Cdr());
	return new PtreeUserPlainStatement(s->Car(), rest);
    }
}

// We need to make sure that nested constructs are translated.  All we do
// is translate the constituent items, making sure that the construct is
// preserved so that it is handled elsewhere.
// This is more fragile than I'd like, since there doesn't seem to be any
// way to automatically translate a block- it has to be done differently for
// each case.
Ptree *VarWalker::TranslateUserPlain(Ptree *exp)
{
  Ptree *keyword = exp->Car();

  if (keyword->Eq("par")) {
    return TranslateUserBlock(exp);
  }
  else if (keyword->Eq("pfor")) {
    return TranslateUserFor(exp);
  }
  else if (keyword->Eq("alt")) {
    return TranslateUserBlock(exp);
  }
  else if (keyword->Eq("afor")) {
    return TranslateUserFor(exp);
  } else {
    return exp;
  }
}

// Look for the variable in all environments from the current up to,
// but not including, the parent environment.  If not found, we consider
// this to be a needed argument and add it to the _args list.  Note:  We
// ignore things which are declared in the outermost scope.  This eliminates
// globals and functions.
Ptree *VarWalker::TranslateVariable(Ptree *exp)
{
  if (!_inguard) {
    //    cerr << "Examining variable:  " << exp << endl;

    Environment *e = env;
    Bind *b;
    bool found = false;
    bool found_inguard = false;

    TypeInfo t;
    exp->Typeof(this,t);
    //    cerr << "Type:  " << t.MakePtree() << endl;

    // For some reason, functions in namespaces sometimes get processed here.
    // This is probably a bug, but we guard against it here.
    if (t.IsFunction()) {
      return exp;
    }

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
  // we won't clobber it and any thread structures are treated as-is, since we know
  // that they're constant.  However, since they're void values, we keep them void
  // and simply make appropriate casts.
  bool is_tsvar = _tsvars.count(name);
  if (t.IsReferenceType() || t.IsArray() || found_inguard || is_tsvar) {
    if (!_vhash.count(name)) {
      _vhash.insert(name);
      _argnames.push_back(ArgPair(exp,true,t));
      if (is_tsvar) {
        // Thread variable- treat as a void* w/appropriate casts when used.
        _args = Ptree::Cons(Ptree::qMake("void *`exp`"),_args);
      } else {
        if (t.IsArray()) {
          t.Dereference();
          _args = Ptree::Cons(t.MakePtree(Ptree::qMake("*`exp`")),_args);
        } else {
          _args = Ptree::Cons(t.MakePtree(Ptree::qMake("`exp`")),_args);
        }
      }
    }
    // We only do the translation if the user has set a thread-struct name.
    // This allows for just scanning variables and not changing anything if
    // _tsname is left blank,
    if (_tsname) {
      Ptree *tmp1 = Ptree::qMake("(((`_tstype`*)");
      Ptree *tmp2 = Ptree::qMake(")->`exp`)");
      return TsList::List(tmp1,new TsLeaf(_tsname),tmp2);
    } else {
      return exp;
    }
  } else {
    if (!_vhash.count(name)) {
      _vhash.insert(name);
      _argnames.push_back(ArgPair(exp,false,t));
      _args = Ptree::Cons(t.MakePtree(Ptree::qMake("*`exp`")),_args);
    }
    if (_tsname) {
      Ptree *tmp1 = Ptree::qMake("(*(((`_tstype`*)");
      Ptree *tmp2 = Ptree::qMake(")->`exp`))");
      return TsList::List(tmp1,new TsLeaf(_tsname),tmp2);
    } else{
      return exp;
    }
  }
}
