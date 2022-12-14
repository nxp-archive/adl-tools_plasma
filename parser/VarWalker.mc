//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//

#include <assert.h>
#include <iostream>
#include <vector>

#include "VarWalker.h"

#include "opencxx/Bind.h"
#include "opencxx/PtreeIter.h"

using namespace std;

StrHash VarWalker::_tsvars;

Class *VarWalker::_plasma = 0;

// This derived class exists so that the double-dispatch mechanism will
// end up calling TranslateVariable.  By default, you generally just get
// a character string, which won't work when there are nested par blocks.
class TsLeaf : public Leaf {
public:
  TsLeaf(Ptree *p) : Leaf(p->GetPosition(),p->GetLength()) {};
  Ptree* Translate(AbstractTranslatingWalker*);
  void Typeof(Walker*, TypeInfo&);
};

// This is just a list which calls translate on its constituent items.
class TsList : public NonLeaf {
public:
  TsList(Ptree *p,Ptree *q) : NonLeaf(p,q) {};
  Ptree *Translate(AbstractTranslatingWalker *);

  static Ptree* List();
  static Ptree* List(Ptree*);
  static Ptree* List(Ptree*, Ptree*);
  static Ptree* List(Ptree*, Ptree*, Ptree*);
  static Ptree* List(Ptree*, Ptree*, Ptree*, Ptree*);
};

Ptree* TsLeaf::Translate(AbstractTranslatingWalker* w)
{
    return w->TranslateVariable(this);
}

void TsLeaf::Typeof(Walker* w, TypeInfo& t)
{
    w->TypeofVariable(this, t);
}

Ptree *TsList::Translate(AbstractTranslatingWalker *w)
{
  Ptree *p1 = Car();
  Ptree *p2 = w->TranslateGeneric(p1);
  Ptree *q1 = Cdr();
  Ptree *q2 = w->TranslateGeneric(q1);
  if (p1 == p2 && q1 == q2) {
    return this;
  } else {
    return new TsList(p2,q2);
  }
}

Ptree* TsList::List()
{
    return 0;
}

Ptree* TsList::List(Ptree* p)
{
    return new TsList(p, 0);
}

Ptree* TsList::List(Ptree* p, Ptree* q)
{
    return new TsList(p, new TsList(q, 0));
}

Ptree* TsList::List(Ptree* p1, Ptree* p2, Ptree* p3)
{
    return new TsList(p1, new TsList(p2, new TsList(p3, 0)));
}

Ptree* TsList::List(Ptree* p1, Ptree* p2, Ptree* p3, Ptree* p4)
{
    return new TsList(p1, TsList::List(p2, p3, p4));
}

// Unfortunately, we have to handle let statements explicitly b/c
// we must register the variables with the environment.
Ptree *VarWalker::TranslateUserClosure(Ptree *s)
{
  using namespace PtreeUtil;

  Ptree *kw = s->Car();

  NewScope();
  Ptree* exp1 = Third(s);
  Ptree* exp1t = Translate(exp1);
  Ptree* body = Nth(s,4);
  Ptree* bodyt = Translate(body);
  ExitScope();

  if (Eq(kw,"let")) {

    PtreeIter iter(exp1);
    Ptree *p;

    while( (p = iter()) != 0) {
      Ptree *var,*type,*rhs;
      if (!Match(p,"[%? [%?] = %?]",&type,&var,&rhs)) {
        ErrorMessage("Malformed decl clause in let block:  ",p,kw);
        return 0;
      }
      // If no type was specified (the usual case), then the variable name
      // is actually parsed as the type name.
      if (!var) {
        var = type;
        type = Ptree::qMake("typeof(`rhs`)");
      }
      // Add it to the environment so that we can translate the body and it
      // will recognize it.
      Class *tclass = new Class(env,type->ToString());
      env->RecordVariable(var->ToString(),tclass);
      // Absorb the comma.
      if ( !iter() ) {
        break;
      }
    }    
  }

  if (exp1 == exp1t && body == bodyt) {
    return s;
  } else {
    Ptree* rest = PtreeUtil::ShallowSubst(exp1t, exp1, bodyt, body, s->Cdr());
    return new PtreeUserPlainStatement(kw, rest);
  }
}

Ptree *VarWalker::TranslateUserBlock(Ptree *s)
{
  Ptree *body = PtreeUtil::Second(s);
  Ptree *body2 = Translate(body);
  if (body == body2) {
    return s;
  } else {
    Ptree *rest = PtreeUtil::ShallowSubst(body2,body,s->Cdr());
    return new PtreeUserPlainStatement(s->Car(),rest);
  }
}

Ptree *VarWalker::TranslateUserFor(Ptree *s)
{
    NewScope();
    Ptree* exp1 = PtreeUtil::Third(s);
    Ptree* exp1t = Translate(exp1);
    Ptree* exp2 = PtreeUtil::Nth(s,3);
    Ptree* exp2t = Translate(exp2);
    Ptree* exp3 = PtreeUtil::Nth(s,5);
    Ptree* exp3t = Translate(exp3);
    Ptree* body = PtreeUtil::Nth(s,7);
    Ptree* body2 = Translate(body);
    ExitScope();

    if(exp1 == exp1t && exp2 == exp2t && exp3 == exp3t && body == body2)
        return s;
    else{
        Ptree* rest = PtreeUtil::ShallowSubst(exp1t, exp1, exp2t, exp2,
                                          exp3t, exp3, body2, body, s->Cdr());
        return new PtreeUserPlainStatement(s->Car(), rest);
    }
}

Ptree *translateList(Walker *w,Ptree *s)
{
  Ptree *a = s->Car();
  Ptree *a2 = w->Translate(a);
  Ptree *rest = s->Cdr();
  Ptree *rest2 = (rest) ? translateList(w,rest) : rest;
  if (a == a2 && rest == rest2) {
    return s;
  } else {
    return PtreeUtil::Cons(a2,rest2);
  }
}

Ptree *VarWalker::TranslateUserWhile(Ptree *s)
{
  Ptree *exp = PtreeUtil::Third(s);
  Ptree *exp2 = translateList(this,exp);
  Ptree *body = PtreeUtil::Nth(s,4);
  Ptree *body2 = Translate(body);

  if (exp == exp2 && body == body2) {
    return s;
  } else {
    Ptree *rest = PtreeUtil::ShallowSubst(exp2,exp,body2,body,s->Cdr());
    return new PtreeUserPlainStatement(s->Car(),rest);
  }
}

// We need to make sure that nested constructs are translated.  All we do
// is translate the constituent items, making sure that the construct is
// preserved so that it is handled elsewhere.
Ptree *VarWalker::TranslateUserPlain(Ptree *exp)
{
  Ptree *keyword = exp->Car();

  if (keyword->IsA(UserKeyword2)) {
    return TranslateUserClosure(exp);
  }
  else if (keyword->IsA(UserKeyword6)) {
    return TranslateUserBlock(exp);
  }
  else if (keyword->IsA(UserKeyword3)) {
    return TranslateUserFor(exp);
  }
  else if (keyword->IsA(UserKeyword)) {
    return TranslateUserWhile(exp);
  } 
  else {
    return exp;
  }
}

Ptree *VarWalker::TranslateUserStatement(Ptree *exp)
{
  Ptree* object = PtreeUtil::First(exp);
  Ptree* op = PtreeUtil::Second(exp);
  Ptree* keyword = PtreeUtil::Third(exp);
  Ptree* p1 = PtreeUtil::Nth(exp,3);
  Ptree* args = PtreeUtil::Nth(exp,4);
  Ptree* p2 = PtreeUtil::Nth(exp,5);
  Ptree* body = PtreeUtil::Nth(exp,6);

  Ptree *object2 = Translate(object);
  Ptree *args2 = Translate(args);
  Ptree *body2 = Translate(body);

  if (object == object2 && args == args2 && body == body2) {
    return exp;
  } else {
    return new PtreeUserStatementExpr(object2,PtreeUtil::List(op,keyword,p1,args2,p2,body2)
                                      );
  }
}

// Add 'this' pointer info to argument structures.
void VarWalker::storeThis(Ptree *ctype)
{
  if (!_handledThis) {
    _argnames.push_back(ArgPair(Ptree::Make("this"),true,TypeInfo()));
    _argnames.back().setFwdDef(Ptree::qMake("class `ctype`"));
    _args = PtreeUtil::Cons(Ptree::qMake("`ctype` *_this"),_args);
    _handledThis = true;
  }
}

// Translates a this pointer.
Ptree *VarWalker::TranslateThis(Ptree *exp)
{
  if (!_inguard) {
    TypeInfo t;
    TypeofThis(exp,t);
    Ptree *ctype = t.MakePtree();
    // Have to get rid of qualified name if present (don't know why it's
    // qualified!).
    if (!ctype->IsLeaf()) {
      ctype = PtreeUtil::Ca_ar(ctype);
    }
    storeThis(ctype);
    // We have to modify "this" to something else b/c it's a reserved word in
    // C++.
    if (_tsname) {
      // Only translate is tsname is set.
      Ptree *tmp1 = Ptree::qMake("(((`_tstype`*)");
      Ptree *tmp2 = Ptree::qMake(")->_this)");
      return TsList::List(tmp1,new TsLeaf(_tsname),tmp2);
    }
  }
  return exp;
}

// This will act like LookupThis, but will ignore the Plasma metaclass.
Class *VarWalker::findThis()
{
  for (Environment *e = env; e != 0; e = e->GetOuterEnvironment()) {
    if (Class *c = e->IsClassEnvironment()) {
      if (c != _plasma) {
        return c;
      }
    }
  }
  return 0;
}

// Handles functions- if this is an implicit method, we translate it to an explicit
// method call and store the this pointer.
Ptree *VarWalker::handleFunction(Ptree *exp)
{
  TypeInfo t;
  exp->Typeof(this,t);

  if (t.IsFunction()) {
    if (Class *c = findThis()) {
      if (c->LookupMember(exp)) {
        // We have an implicit method call.
        storeThis(c->Name());
        if (_tsname) {
          // Only translate is tsname is set.
          Ptree *tmp1 = Ptree::qMake("(((`_tstype`*)");
          Ptree *tmp2 = Ptree::qMake(")->_this)->`exp`");
          return TsList::List(tmp1,new TsLeaf(_tsname),tmp2);
        }        
      }
    }
    return exp;
  }
  return 0;
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

    // Handle namespace functions and methods here.
    if (Ptree *e = handleFunction(exp)) {
      return e;
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
        _args = PtreeUtil::Cons(Ptree::qMake("void *`exp`"),_args);
      } else {
        if (t.IsArray()) {
          t.Dereference();
          _args = PtreeUtil::Cons(t.MakePtree(Ptree::qMake("*`exp`")),_args);
        } else {
          _args = PtreeUtil::Cons(t.MakePtree(Ptree::qMake("`exp`")),_args);
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
      _args = PtreeUtil::Cons(t.MakePtree(Ptree::qMake("*`exp`")),_args);
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
