//
// The main Plasma parser- this converts the par, alt, etc.
// statements to C++.
//

#ifndef _PLASMA_H_
#define _PLASMA_H_

#include <list>

#include "opencxx/mop2.h"

using namespace Opencxx;

// This stuff is used for alt/afor processing.
struct Port : public gc {
  Ptree   *chan;           // Channel expression of a port statement.
  Ptree   *op;             // Operator used to access channel.
  Ptree   *val;            // Value expression of a port statement.
  Ptree   *body;           // Body of a port statement.
  Ptree   *s1, *s2, *s3;   // Afor loop expressions.
  Ptree   *stack;          // Stack/stack index var for afor.
  TypeInfo indextype;      // Index type for afor blocks.
  Ptree   *loopvar;        // Loop-variable for afor.
  Port(bool l,Ptree *c,Ptree *o,Ptree *v,Ptree *b) : 
    chan(c), op(o), val(v), body(b), 
    s1(0), s2(0), s3(0),
    stack(0), loopvar(0)
  {};
  bool isloop() const { return s1; };
  bool needstack() const { return stack; };
};

// Stores a port or a list- used to create 
// a tree of port/port-list objects.
class PortNode {
public:
  typedef std::list<PortNode,gc_allocator<PortNode> > PortList;

  PortNode (Port *p) : _port(p) {};
  PortNode () : _port(0) {};
  bool isport() const { return _port; };
  Port &port() const { assert(_port); return *_port; };
  const PortList &list() const { return _list; };
  PortList &list() { return _list; };
private:
  Port     *_port;
  PortList  _list;
};

typedef PortNode::PortList PortList;

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
  Ptree* TranslateAlt(Environment* env,Ptree* keyword, Ptree* rest,bool reverse);
  Ptree* TranslateAlt(Environment* env,Ptree* keyword, Ptree* rest);
  Ptree* TranslatePriAlt(Environment* env,Ptree* keyword, Ptree* rest);
  Ptree* TranslateAfor(Environment* env,Ptree* keyword, Ptree* rest);
  Ptree* TranslateSpawn(Environment* env,Ptree* keyword, Ptree* rest);

  // Various helper functions.
  void makeThreadStruct(Environment *env,Ptree *type,Ptree *args,const ArgVect &av);
  void convertToThread(Ptree* &elist,Ptree* &thnames,Ptree *expr,VarWalker *vw,
                       Environment *env,bool heapalloc);
  Ptree *generateAltBlock(Environment *env,const PortList &pv,Ptree *defaultblock);
  Ptree *generateAltBody(Environment *env,Ptree *cur,Ptree *label,
                         Ptree *handle,const PortList &pv,Ptree *defaultblock);
  bool parseAforCondition(VarWalker *vs,Environment *env,Ptree *s1,Ptree *s3);
  bool parseAltBody(Environment *env,Ptree *rest,PortList &pv,Ptree* &defaultblock,bool reverse);
  bool parseAforBody(Environment *env,Ptree *rest,PortList &pv,Ptree* &defaultblock);
  bool parseAltBlock(Environment *env,Ptree *body,bool isloop,PortList &pv,Ptree* &defaultblock);
  bool checkForMemberCall(Environment *,Class* &,Environment* &,Ptree* &,Ptree* &);
  bool makeSpawnStruct(Environment *env,Class *,TypeInfo,Ptree *,Ptree *);
  bool makeSpawnFunc(Environment *env,Class *,TypeInfo,Ptree *,Ptree *,Ptree *,Ptree *,bool);
};


#endif


