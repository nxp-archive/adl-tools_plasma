//
// The main Plasma parser- this converts the par, alt, etc.
// statements to C++.
//

#ifndef _PLASMA_H_
#define _PLASMA_H_

#include <vector>

// This stuff is used for alt/afor processing.
struct Port {
  Ptree   *chan;           // Channel expression of a port statement.
  Ptree   *op;             // Operator used to access channel.
  Ptree   *val;            // Value expression of a port statement.
  Ptree   *body;           // Body of a port statement.
  Ptree   *s1, *s2, *s3;   // Afor loop expressions.
  Ptree   *stack;          // Stack/stack index var for afor.
  TypeInfo indextype;      // Index type for afor blocks.
  Ptree   *loopvar;        // Loop-variable for afor.
  Port(bool l,Ptree *c,Ptree *o,Ptree *v,Ptree *b) : chan(c), op(o), val(v), body(b), 
                                                     s1(nil), s2(nil), s3(nil),
                                                     stack(nil), loopvar(nil)
  {};
  bool isloop() const { return s1; };
  bool needstack() const { return stack; };
};

typedef std::vector<Port> PortVect;

// Main parser metaclass.
class Plasma : public Class {
public:
  // Main entry point for translating new constructs.
  Ptree* TranslateUserPlain(Environment*,Ptree*, Ptree*);
  // Translates spawn function call.
  Ptree *TranslateFunctionCall(Environment *,Ptree *,Ptree *);
  Ptree *TranslateMemberCall(Environment* env, Ptree* object,
                             Ptree* op, Ptree* member, Ptree* arglist);
  // Setup code.
  static bool Initialize();
  void InitializeInstance(Ptree* def, Ptree* margs);

private:
  // Translation functions for the new types introduced by Plasma.
  Ptree* TranslatePar(Environment* env,Ptree* keyword, Ptree* rest);
  Ptree* TranslatePfor(Environment* env,Ptree* keyword, Ptree* rest);
  Ptree* TranslateAlt(Environment* env,Ptree* keyword, Ptree* rest);
  Ptree* TranslateAfor(Environment* env,Ptree* keyword, Ptree* rest);
  Ptree* TranslateSpawn(Environment* env,Ptree* keyword, Ptree* rest);

  // Various helper functions.
  void makeThreadStruct(Environment *env,Ptree *type,Ptree *args,const ArgVect &av);
  void convertToThread(Ptree* &elist,Ptree* &thnames,Ptree *expr,VarWalker *vw,
                       Environment *env,bool heapalloc);
  Ptree *generateAltBlock(Environment *env,const PortVect &pv,Ptree *defaultblock);
  Ptree *generateAltBody(Environment *env,Ptree *cur,Ptree *label,Ptree *handle,Ptree *uflag,
                         const PortVect &pv,Ptree *defaultblock);
  bool parseAforCondition(VarWalker *vs,Environment *env,Ptree *s1,Ptree *s3);
  bool parseAltBody(Environment *env,Ptree *rest,PortVect &pv,Ptree* &defaultblock);
  bool parseAforBody(Environment *env,Ptree *rest,PortVect &pv,Ptree* &defaultblock);
  bool parseAltBlock(Environment *env,Ptree *body,bool isloop,PortVect &pv,Ptree* &defaultblock);
  bool checkForMemberCall(Environment *,Class* &,Environment* &,Ptree* &,Ptree* &);
  bool makeSpawnStruct(Environment *env,Class *,TypeInfo,Ptree *,Ptree *);
  bool makeSpawnFunc(Environment *env,Class *,TypeInfo,Ptree *,Ptree *,Ptree *,Ptree *,bool);
};


#endif


