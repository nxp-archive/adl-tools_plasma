//
// Simple driver for testing parsing issues.
//

#include <iostream>

#include "opencxx/PtreeIter.h"

#include "ParseTest.h"

using namespace std;
using namespace PtreeUtil;

bool ParseTest::Initialize()
{
  SetMetaclassForPlain("ParseTest");
  RegisterNewClosureStatement("define");
  RegisterNewClosureStatement("func");
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
      if (Ca_ar(p)->Reify(str)) {
        cout << "String: " << str << endl;
      } else {
        cout << "Other :  " << p << endl;
      }
    }
    return rest;
  } else if (Eq(keyword,"func")) {
    Ptree *body = Nth(rest,3);
    rest = TranslateExpression(env,body);
    return Cons(keyword,rest);
  }

  ErrorMessage(env, "unknown user statement encountered",0,keyword);
  return 0;
}

