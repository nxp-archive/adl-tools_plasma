//
// The main Plasma parser- this converts the par, alt, etc.
// statements to C++.
//

#ifndef _PLASMA_H_
#define _PLASMA_H_

struct Port {
  Ptree *chan;
  Ptree *val;
  Ptree *body;
  Port(Ptree *c,Ptree *v,Ptree *b) : chan(c), val(v), body(b) {};
};

typedef std::vector<Port> PortVect;

// Main parser metaclass.
class Plasma : public Class {
public:
  // Main entry point for translating new constructs.
  Ptree* TranslateUserPlain(Environment*,Ptree*, Ptree*);
  // Translates spawn function call.
  Ptree *TranslateFunctionCall(Environment *,Ptree *,Ptree *);
  // Setup code.
  static bool Initialize();

private:
  // Translation functions for the new types introduced by Plasma.
  Ptree* TranslatePar(Environment* env,Ptree* keyword, Ptree* rest);
  Ptree* TranslatePfor(Environment* env,Ptree* keyword, Ptree* rest);
  Ptree* TranslateAlt(Environment* env,Ptree* keyword, Ptree* rest);
  Ptree* TranslateAfor(Environment* env,Ptree* keyword, Ptree* rest);
  Ptree* TranslateSpawn(Environment* env,Ptree* keyword, Ptree* rest);

  // Various helper functions.
  Ptree *generateAltBody(Environment *env,Ptree *cur,Ptree *label,Ptree *iname,
                         const PortVect &pv,Ptree *defaultblock);
  void makeThreadStruct(Environment *env,Ptree *type,Ptree *args);
  void convertToThread(Ptree* &elist,Ptree* &thnames,Ptree *expr,VarWalker *vw,
                       Environment *env,bool heapalloc);
  bool parseAforCondition(VarWalker *vs,Environment *env,Ptree *s1,Ptree *s3);
  bool parseAltBlock(Environment *env,Ptree *body,PortVect &pv,Ptree* &defaultblock);

  bool checkForMemberCall(Environment *,Class* &,Environment* &,Ptree* &,Ptree* &);
  bool makeSpawnStruct(Environment *env,Class *,TypeInfo,Ptree *,Ptree *);
  bool makeSpawnFunc(Environment *env,Class *,TypeInfo,Ptree *,Ptree *,Ptree *,Ptree *);
};


#endif


