//
// AST nodes used by the compiler.
// These are garbage collected when allocated
// from the heap.  However, they do not register
// a finalizer, so thir destructors will not be called.
//

#include <vector>

#include "gc_cpp.h"
#include "gc_allocator.h"

#include "String.h"

class String;

#define VisitDecl(n) virtual void visit##n(n *) {}

struct Visitor;
struct Type;

struct Node : public gc {
  Node(Type *t = 0) : _has_addr(false), _output_addr(0), _type(t), _filename(0), _linenumber(0) {};
  virtual ~Node();
  // Return true if we're a null class (leaf node).
  virtual bool is_null() const { return false; };
  // Returns true if we're a constant e.g. 5, "hello", etc.
  virtual bool is_const() const { return false; };
  // Returns true if node has an address, i.e. is a valid lvalue.
  virtual bool has_address() const { return _has_addr; };
  // Specifies that we have an address.
  virtual void set_has_address() { _has_addr = true; };
  // Returns the node's type.
  Type *type() const { return _type; };
  // Calculates a constant integer if the node represents
  // a constant expression, e.g. returns a node storing
  // 30 if the node represents an expression of 3*10.
  // Returns false in second if not constant, otherwise 
  // the result in first and true in second.
  typedef std::pair<int,bool> Calc;
  virtual Calc calculate() const;
  // Print to the specified stream.  Argument is number of 
  // characters to indent.
  virtual void print(std::ostream &,int) const = 0;
  // Visitor double-dispatch main entry point.
  virtual void walk(Visitor &) = 0;

  void setFileData(const char *fn,int ln);
  int linenumber() const { return _linenumber; };
  const char *filename() const { return _filename; };
protected:
  bool        _has_addr;
  int         _output_addr;
  Type       *_type;
  const char *_filename;
  int         _linenumber;
};

inline std::ostream &operator<<(std::ostream &o,const Node *n)
{
  n->print(o,0);
  return o;
}

// Null-terminator node in an AST.
struct NullNode : public Node {
  NullNode();
  virtual bool is_null() const { return true; };
  virtual void walk(Visitor &);
  virtual void print(std::ostream &,int) const;
};

// Expression w/array notation, e.g. a[5+4].
struct ArrayExpression : public Node {
  ArrayExpression(Node *expr, Node *index) :
    _expr(expr), _index(index) {}
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
protected:
  Node *_expr;
  Node *_index;
};

// String literal node.
struct StringLiteral : public Node {
  StringLiteral(String s);
  void append(String s);
  String get() const { return _s; };
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
private:
  String _s;
};

// Identifier node.
struct Id : public Node {
  Id(String id) : _id(id) {};

  String id() const { return _id; };
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
private:
  String _id;
};

// Numeric constants.
struct Const : public Node {
  Const(int value,Type *type) : Node(type), _value(value) {};
  int value() const { return _value; };
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
private:
  int _value;
};

// Tries to calculate the value of a numeric expression.
// If it can, it returns a constant node, otherwise it
// returns the original tree.
Node *get_calculated(Node *);

// Any generic unary operator.
struct Unaryop : public Node {
  Unaryop(Node *expr) : _expr(expr) {};
  Node *expr() const { return _expr; };
  virtual void walk(Visitor &);
protected:
  Node *_expr;
};

// A negative unary operator, e.g. -5.
struct Negative : public Unaryop {
  Negative(Node *expr) : Unaryop(expr) {};
  Calc calculate() const;
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
};

// A pointer dereference, e.g. *a.
struct Pointer : public Unaryop {
  Pointer(Node *expr) : Unaryop(expr) {};
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
};

// An address-of operator, e.g. &a.
struct AddrOf : public Unaryop {
  AddrOf(Node *expr) : Unaryop(expr) {};
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
};

// Any binary operator, such as that for arithmetic
// operations (+, -, *), assignment operations, etc.
// The operator type is the token id used by the parser.
struct Binop : public Node {
  Binop(Node *l,int op,Node *r) : _left(l), _right(r), _op(op) {};
  Calc calculate() const;
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
private:
  Node *_left;
  Node *_right;
  int   _op;
};

// If-then-else statement.
struct IfStatement : public Node {
  IfStatement(Node *e,Node *t,Node *el = 0) :
    _expr(e), _then(t), _else(el) {};
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
private:
  Node *_expr;
  Node *_then;
  Node *_else;
};

// A break statement.
struct BreakStatement : public Node {
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
};

// A continue statement.
struct ContinueStatement : public Node {
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
};

// A return statement.
struct ReturnStatement : public Node {
  ReturnStatement(Node *expr = 0) : _expr(expr) {};
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
private:
  Node *_expr;
};

// A for-loop.
struct ForLoop : public Node {
  ForLoop(Node *b,Node *expr,Node *e,Node *s) :
    _begin(b), _expr(expr), _end(e), _stmt(s) {};
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
private:
  Node *_begin;
  Node *_expr;
  Node *_end;
  Node *_stmt;
};

// A while loop.
struct WhileLoop : public Node {
  WhileLoop(Node *expr,Node *stmt) :
    _expr(expr), _stmt(stmt) {};
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
private:
  Node *_expr;
  Node *_stmt;
};

// A list of nodes (base class).
struct NodeList : public Node, public std::vector<Node *,traceable_allocator<Node *> > {
  NodeList() {};
  NodeList (Node *n) { push_back(n); };
  void add(Node *n) { push_back(n); };

protected:
  void print_list(std::ostream &,const char *sep,int indent = 0) const;
};

// A list of arguments for a function expression., e.g.
// "5,2,3" in "a = my_func(5,2,3)"
struct ArgumentList : public NodeList {
  ArgumentList() {};
  ArgumentList(Node *n) : NodeList(n) {};
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
};

// A list of parameters for a function prototype, e.g. 
// "int a, char b, char c" in int my_func(int a,char b,char c).
struct ParamList : public NodeList {
  ParamList() {};
  ParamList(Node *n) : NodeList(n) {};
  bool hasellipsis() const { return _hasellipsis; };
  void setHasellipsis() { _hasellipsis = true; };
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
private:
  bool _hasellipsis;
};

// A list of statements, such as for a function body or
// loop body.
struct StatementList : public NodeList {
  StatementList() {};
  StatementList(Node *n) : NodeList(n) {};
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
};

// This represents a single C file.
struct TranslationUnit : public NodeList {
  TranslationUnit() {};
  TranslationUnit(Node *n) : NodeList(n) {};
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
};

// A list of variable declarations, such as the ones
// put at the beginning of a block.
struct DeclarationList : public NodeList {
  DeclarationList() {};
  DeclarationList(Node *n) : NodeList(n) {};
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
};

// A function call.
struct FunctionExpression : public Node {
  FunctionExpression(Node *f,Node *a) :
    _function(f), _arglist(a) {};
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
private:
  Node *_function;
  Node *_arglist;
};

// A compound statement, e.g. "{ int i; i += 1; }".
struct CompoundStatement : public Node {
  CompoundStatement(Node *d,Node *s) :
    _declaration_list(d), _statement_list(s) {};
  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
private:
  Node *_declaration_list;
  Node *_statement_list;
};

// A function definition (declaration and body).
struct FunctionDefn : public Node {
  FunctionDefn(Node *decl,Node *body);

  bool is_extern() const { return _extern; };
  bool is_static() const { return _static; };

  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
private:
  Node *_name;
  Node *_body;
  bool  _extern;
  bool  _static;
};

// A node representing a declaration of a function or
// variable.
struct Declaration : public Node {
  Declaration (Node *n,Type *t = 0);

  Node *name() const { return _name; };
  Node *body() const { return _body; };

  void set_base_type(Type *t);
  void add_type(Type *t);

  bool is_extern() const { return _extern; };
  bool is_static() const { return _static; };

  virtual void print(std::ostream &,int) const;
  virtual void walk(Visitor &);
private:
  Node *_name;
  Node *_body;
  bool  _extern;
  bool  _static;
  bool  _is_used;
};

//
// Type nodes. 
//

// This represents the type of another node, e.g. the
// type of 5+a, where a is an int, will be a Type node
// storing int.
// Types may be nested (chained together).
struct Type {
  Type();
  Type(Type *t) : _child(t) {};
  // Set the base (innermost) type ofa type.  For instance,
  // calling this with a pointer(int) type on a pointer() type
  // will give you a pointer(pointer(int)).
  void set_base_type(Type *t);
  // Returns true if this represents a function.
  virtual bool is_function() const { return false; };
  // Prints a string representation.
  virtual std::ostream &print(std::ostream &o) const = 0;
  // Prints only the outer most type.
  virtual std::ostream &print_outer(std::ostream &o) const = 0;
protected:
  Type *_child;
};

// A void type- used to represent empty types.
struct VoidType : public Type {
  VoidType() {};
  virtual std::ostream &print(std::ostream &o) const;
  virtual std::ostream &print_outer(std::ostream &o) const;  
};

// Represents intrinsic types, e.g. int, char.
struct BaseType : public Type {
  enum Type { Int, Char };
  BaseType(Type t) : _type(t) {};
  Type type() const { return _type; };
  virtual std::ostream &print(std::ostream &o) const;
  virtual std::ostream &print_outer(std::ostream &o) const;
private:
  Type _type;
};

// A type representing a function (for prototypes and calls).
struct FunctionType : public Type {
  FunctionType(Node *p,Type *c = 0) : Type(c), _params(p) {};
  Type *get_return_type() const { return _child; };
  Node *get_params() const { return _params; };
  bool is_function() const { return true; };

  virtual std::ostream &print(std::ostream &o) const;
  virtual std::ostream &print_outer(std::ostream &o) const;

private:
  Node *_params;
};

struct PointerType : public Type {
  PointerType() {};
  PointerType(Type *t) : Type(t) {};

  virtual std::ostream &print(std::ostream &o) const;
  virtual std::ostream &print_outer(std::ostream &o) const;
};

// Base class for any visitor, implementing a double dispatch
// system.
// You need to add a virtual function for each
// type of AST node in this file.
struct Visitor {
  VisitDecl(NullNode);
  VisitDecl(ArrayExpression);
  VisitDecl(StringLiteral);
  VisitDecl(Id);
  VisitDecl(Const);
  VisitDecl(Unaryop);
  VisitDecl(Negative);
  VisitDecl(Pointer);
  VisitDecl(AddrOf);
  VisitDecl(Binop);
  VisitDecl(IfStatement);
  VisitDecl(BreakStatement);
  VisitDecl(ContinueStatement);
  VisitDecl(ReturnStatement);
  VisitDecl(ForLoop);
  VisitDecl(WhileLoop);
  VisitDecl(ArgumentList);
  VisitDecl(ParamList);
  VisitDecl(StatementList);
  VisitDecl(TranslationUnit);
  VisitDecl(DeclarationList);
  VisitDecl(FunctionExpression);
  VisitDecl(CompoundStatement);
  VisitDecl(FunctionDefn);
  VisitDecl(Declaration);
};
