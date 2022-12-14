//
// Simple driver for testing parsing issues.
//

#include <iostream>
#include <iomanip>

#include "opencxx/PtreeIter.h"

#include "ParseTest.h"

using namespace std;
using namespace Opencxx;
using namespace PtreeUtil;

bool ParseTest::Initialize()
{
  SetMetaclassForPlain("ParseTest");
  RegisterNewClosureStatement("define");
  RegisterNewClosureStatement("func");
  
  cout.setf(ios_base::boolalpha);

  return true;
}

Ptree* ParseTest::TranslateUserPlain(Environment* env,Ptree* keyword, Ptree* rest)
{
  if (Eq(keyword,"define")) {
    cout << "Define statement:  " << rest << endl;
    Ptree *body = Nth(rest,3);
    TranslateExpression(env,body);
    PtreeIter iter(Second(body));
    Ptree *p;
    
    

    while ( (p = iter()) ) {
      char *str;
      unsigned ui;
      bool     b;

      Ptree *key,*value;

      if (PtreeUtil::Match(p,"[ [%? = %?] ; ]",&key,&value)) {
        cout << "Key:  " << key << "\n"
             << "Value:  " << value << endl;
        if (value->Reify(b)) {
          cout << "  Boolean: " << b << endl;
        }
        if (value->Reify(str)) {
          cout << "  String: " << str << endl;
        } else if(value->Reify(ui)) {
          cout << "  Integer: " << ui << endl;
        }
      } else {
        cout << "Stmt:  " << p << endl;
      }
    }
    return rest;
  } else if (Eq(keyword,"func")) {
    Ptree *tbody = Cons(keyword,rest);
    Ptree *args;
    if (GetArgDeclList((PtreeDeclarator*)tbody,args)) {
      while (args) {
        Ptree *arg = Second(args->Car());
        cout << "Arg:  " << arg << endl;
        TypeInfo type;
        type.Set(arg->GetEncodedType(),env);
        cout << "Type:  " << type.MakePtree() << endl;
        args = args->Cdr();
        if (!args) {
          break;
        }
        args = args->Cdr();
      }
    }
    Ptree *body = Nth(rest,3);
    rest = TranslateExpression(env,body);
    return Cons(keyword,rest);
  }

  ErrorMessage(env, "unknown user statement encountered",0,keyword);
  return 0;
}

static Class* parse_test_setup(Ptree* def, Ptree* marg)
{
  Class* c = new ParseTest;
  c->InitializeInstance(def, marg);
  return c;
}

static MetaclassRegistration _parse_test_setup_var("ParseTest", parse_test_setup, 
                                                   ParseTest::Initialize, 0);
