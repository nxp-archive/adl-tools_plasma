//
// AST nodes used by the compiler.
// These are garbage collected when allocated
// from the heap.  However, they do not register
// a finalizer, so thir destructors will not be called.
//

#ifndef _NODE_H_
#define _NODE_H_

#include <vector>

#include "gc_cpp.h"
#include "gc_allocator.h"

#include "String.h"

class String;
class SymTab;
class CodeGen;
class AsmStore;
class CompileLoc;

#define Error(n,x) { \
  ostringstream ss; \
  ss << n->filename() << ":" << n->linenumber() << ":  Error:  " << x; \
  throw runtime_error(ss.str()); \
}

#define Error1(x) { \
  ostringstream ss; \
  ss << "Error:  " << x; \
  throw runtime_error(ss.str()); \
}

#define Warn(n,x) { \
  cerr << n->filename() << ":" << n->linenumber() << ":  Warning:  " << x << "\n"; \
}

struct Type;

// Various manipulators for setting stream attributes.

// Use this manipulator to indicate that you want symbols to be printed.
std::ostream &printsyms(std::ostream &o);
// Turn of symbol printing.
std::ostream &noprintsyms(std::ostream &o);
// Return current indentation amount.
int curindent(std::ostream &o);
// Insert indentation whitespace.
std::ostream &indent(std::ostream &o);
// Increment indentation amount.
std::ostream &incrindent(std::ostream &o);
// Decrement indentation amount.
std::ostream &decrindent(std::ostream &o);

struct Node : public gc {
  Node(Type *t = 0);
  virtual ~Node();
  // Return true if we're a null class (leaf node).
  virtual bool is_null() const { return false; };
  // Returns true if we're a constant e.g. 5, "hello", etc.
  virtual bool is_const() const { return false; };
  // Returns true if node has an address, i.e. is a valid lvalue.
  bool has_address() const { return _has_addr; };
  // Specifies that we have an address.
  void set_has_address() { _has_addr = true; };
  // Getter/setter for has-return-stmt.
  bool has_return_stmt() const { return _has_return_stmt; };
  void set_has_return_stmt(bool h) { _has_return_stmt = h; };
  // Get the symbol table for this node.
  virtual SymTab *symtab() const { return 0; };
  // Get a symbol associated with this node, if applicable.
  virtual Node *symbol() const { return 0; };
  // Getter/setter for output_addr.
  int output_addr() const { return _output_addr; };
  void set_output_addr(int oa) { _output_addr = oa; };
  // Getter/setter for compile location.
  CompileLoc *compile_loc() const { return _compile_loc; };
  void set_compile_loc(CompileLoc *cl) { _compile_loc = cl; };
  // True if object is externally defined.
  virtual bool is_extern() const { return false; };
  virtual bool is_static() const { return false; };
  // Getter/setter for whether a variable is used.  Overload
  // to provide functionality.
  virtual bool is_used() const { return false; };
  virtual void set_used() { };
  // Getter/setter for coerce-to-type.
  Type *coerce_to_type() const { return _coerce_to_type; };
  void set_coerce_to_type(Type *t) { _coerce_to_type = t; };
  // Item's name, if applicable.
  virtual String name() const { return String(); };
  // Its constant value, if appropriate.
  virtual int value() const { return 0; };
  // Returns the node's type.
  Type *type() const { return _type; };
  // Change the node's type.
  void set_type(Type *t) { _type = t; };
  // This will just assert- a collection class should overload
  // this to store multiple items.
  virtual void add(Node *n);
  // Calculates a constant integer if the node represents
  // a constant expression, e.g. returns a node storing
  // 30 if the node represents an expression of 3*10.
  // Returns false in second if not constant, otherwise 
  // the result in first and true in second.
  typedef std::pair<int,bool> Calc;
  virtual Calc calculate() const;

  void setFileData(const char *fn,int ln);
  int linenumber() const { return _linenumber; };
  const char *filename() const { return _filename; };

  // These are the main "services" which traverse the ATS,
  // performing various compiler passes.

  // Print to the specified stream.  Argument is number of 
  // characters to indent.
  void print(std::ostream &) const;
  // Generate symbol tables.  Traverse the AST and generate
  // symbol tables as we go.  Throws a runtime_error if errors
  // are encountered.
  void gensymtab();
  // Perform type checking.  Throws a runtime_error if errors
  // are encountered.
  void typecheck();  
  // Perform flow-control checking:  Makes sure that all
  // branches of a function return a value, check break/continue
  // statements, etc.
  void flowcontrol();
  // Generate code.  In general, these should just call the
  // appropriate routine in CodeGen, which will do the actual work.
  virtual void codegen(CodeGen *);
  // Write code to specified stream.
  virtual void writecode(std::ostream &) const;

  // Used internally- don't call directly.
  virtual void gensymtab(SymTab *);
  virtual void typecheck(Node *);
  virtual void flowcontrol(Node *,bool);
protected:
  // This prints class-specific information.
  virtual void printdata(std::ostream &) const;

  bool        _has_return_stmt;
  bool        _has_addr;
  int         _output_addr;
  CompileLoc *_compile_loc;
  Type       *_type;
  Type       *_coerce_to_type;
  const char *_filename;
  int         _linenumber;
};

std::ostream &operator<<(std::ostream &o,const Node *n);

// Null-terminator node in an AST.
struct NullNode : public Node {
  NullNode();
  virtual bool is_null() const { return true; };
};

// Expression w/array notation, e.g. a[5+4].
struct ArrayExpression : public Node {
  ArrayExpression(Node *expr, Node *index) :
    _expr(expr), _index(index) {}
  virtual void gensymtab(SymTab *);
  virtual void typecheck(Node *);
  virtual void codegen(CodeGen *);

  Node *expr() const { return _expr; };
  Node *index() const { return _index; };
protected:
  virtual void printdata(std::ostream &) const;

  Node *_expr;
  Node *_index;
};

// String literal node.
struct StringLiteral : public Node {
  StringLiteral(String s);
  void append(String s);
  String get() const { return _s; };

  virtual void codegen(CodeGen *);
protected:
  virtual void printdata(std::ostream &) const;

private:
  String _s;
};

// Identifier node.
struct Id : public Node {
  Id(String id) : _id(id), _symbol(0) {};

  virtual String name() const { return _id; };
  virtual Node *symbol() const { return _symbol; };

  virtual void gensymtab(SymTab *);
  virtual void typecheck(Node *);
  virtual void codegen(CodeGen *);
protected:
  virtual void printdata(std::ostream &) const;

private:
  String  _id;
  Node   *_symbol;
};

// Numeric constants.
struct Const : public Node {
  Const(int value,Type *type) : Node(type), _value(value) {};
  Calc calculate() const;

  virtual bool is_const() const { return true; };
  virtual int value() const { return _value; };

  virtual void codegen(CodeGen *);
protected:
  virtual void printdata(std::ostream &) const;

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

  virtual void gensymtab(SymTab *);
  virtual void typecheck(Node *);
protected:
  virtual void printdata(std::ostream &) const;  

  Node *_expr;
};

// A negative unary operator, e.g. -5.
struct Negative : public Unaryop {
  Negative(Node *expr) : Unaryop(expr) {};
  Calc calculate() const;
  virtual void typecheck(Node *curr_func);
  virtual void codegen(CodeGen *);
};

// A pointer dereference, e.g. *a.
struct Pointer : public Unaryop {
  Pointer(Node *expr) : Unaryop(expr) {};
  virtual void typecheck(Node *curr_func);
  virtual void codegen(CodeGen *);
};

// An address-of operator, e.g. &a.
struct AddrOf : public Unaryop {
  AddrOf(Node *expr) : Unaryop(expr) {};
  virtual void typecheck(Node *curr_func);
  virtual void codegen(CodeGen *);
};

// Any binary operator, such as that for arithmetic
// operations (+, -, *), assignment operations, etc.
// The operator type is the token id used by the parser.
struct Binop : public Node {
  Binop(Node *l,int op,Node *r) : _left(l), _right(r), _op(op) {};
  Calc calculate() const;

  Node *left() const { return _left; };
  Node *right() const { return _right; };
  int op() const { return _op; };

  const char *op_str() const;

  bool is_assign_op() const;
  bool is_compare_op() const;

  virtual void gensymtab(SymTab *);
  virtual void typecheck(Node *);
  virtual void codegen(CodeGen *);
protected:
  virtual void printdata(std::ostream &) const;
private:
  Node *_left;
  Node *_right;
  int   _op;
};

// If-then-else statement.
struct IfStatement : public Node {
  IfStatement(Node *e,Node *t,Node *el = 0) :
    _expr(e), _then(t), _else(el) {};

  virtual void gensymtab(SymTab *);
  virtual void typecheck(Node *);
  virtual void flowcontrol(Node *,bool);
  virtual void codegen(CodeGen *);

  Node *expr() const { return _expr; };
  Node *then_blk() const { return _then; };
  Node *else_blk() const { return _else; };
protected:
  virtual void printdata(std::ostream &) const;
private:
  Node *_expr;
  Node *_then;
  Node *_else;
};

// A break statement.
struct BreakStatement : public Node {
  virtual void flowcontrol(Node *,bool);
  virtual void codegen(CodeGen *);
};

// A continue statement.
struct ContinueStatement : public Node {
  virtual void flowcontrol(Node *,bool);
  virtual void codegen(CodeGen *);
};

// A return statement.
struct ReturnStatement : public Node {
  ReturnStatement(Node *expr = 0) : _expr(expr) {};

  Node *expr() const { return _expr; };

  virtual void gensymtab(SymTab *);
  virtual void typecheck(Node *);
  virtual void flowcontrol(Node *,bool);
  virtual void codegen(CodeGen *);
protected:
  virtual void printdata(std::ostream &) const;
private:
  Node *_expr;
};

// A for-loop.
struct ForLoop : public Node {
  ForLoop(Node *b,Node *expr,Node *e,Node *s) :
    _begin(b), _expr(expr), _end(e), _stmt(s) {};

  Node *begin() const { return _begin; };
  Node *expr() const { return _expr; };
  Node *end() const { return _end; };
  Node *stmt() const { return _stmt; };

  virtual void gensymtab(SymTab *);
  void typecheck(Node *);
  virtual void flowcontrol(Node *,bool);
  virtual void codegen(CodeGen *);
protected:
  virtual void printdata(std::ostream &) const;
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

  Node *expr() const { return _expr; };
  Node *stmt() const { return _stmt; };

  virtual void gensymtab(SymTab *);
  virtual void typecheck(Node *);
  virtual void flowcontrol(Node *,bool);
  virtual void codegen(CodeGen *);
protected:
  virtual void printdata(std::ostream &) const;
private:
  Node *_expr;
  Node *_stmt;
};

// A list of nodes (base class).
struct NodeList : public Node, public std::vector<Node *,traceable_allocator<Node *> > {
  NodeList() {};
  NodeList (Node *n) { push_back(n); };
  virtual void add(Node *n) { push_back(n); };

  virtual void gensymtab(SymTab *);
  virtual void typecheck(Node *);
  virtual void codegen(CodeGen *);
  virtual void writecode(std::ostream &) const;
protected:
  virtual void printdata(std::ostream &) const;
};

// A list of arguments for a function expression., e.g.
// "5,2,3" in "a = my_func(5,2,3)"
struct ArgumentList : public NodeList {
  ArgumentList() {};
  ArgumentList(Node *n) : NodeList(n) {};
};

// A list of parameters for a function prototype, e.g. 
// "int a, char b, char c" in int my_func(int a,char b,char c).
struct ParamList : public NodeList {
  ParamList() {};
  ParamList(Node *n) : NodeList(n) {};
  bool has_ellipsis() const { return _hasellipsis; };
  void set_has_ellipsis() { _hasellipsis = true; };
private:
  bool _hasellipsis;
};

// A list of statements, such as for a function body or
// loop body.
struct StatementList : public NodeList {
  StatementList() {};
  StatementList(Node *n) : NodeList(n) {};

  virtual void flowcontrol(Node *,bool);
  virtual void codegen(CodeGen *);
};

// Inherit from this if you have a symbol table.
struct SymNode {
  SymNode() : _symtab(0) {};
  SymTab *getsymtab() const { return _symtab; };
  void printsyms(std::ostream &o) const;
protected:
  SymTab *_symtab;
};

// This represents a single C file.
struct TranslationUnit : public NodeList, public SymNode {
  TranslationUnit() {};
  TranslationUnit(Node *n) : NodeList(n) {};

  virtual SymTab *symtab() const { return getsymtab(); };

  virtual void gensymtab(SymTab *);
protected:
  virtual void printdata(std::ostream &) const;
};

// A list of variable declarations, such as the ones
// put at the beginning of a block.
struct DeclarationList : public NodeList {
  DeclarationList() {};
  DeclarationList(Node *n) : NodeList(n) {};
};

// A function call.
struct FunctionExpression : public Node {
  FunctionExpression(Node *f,Node *a) :
    _function(f), _arglist(a) {};

  Node *function() const { return _function; };
  Node *arglist() const { return _arglist; };

  virtual void gensymtab(SymTab *);
  virtual void typecheck(Node *);
  virtual void codegen(CodeGen *);
protected:
  virtual void printdata(std::ostream &) const;
private:
  Node *_function;
  Node *_arglist;
};

// A compound statement, e.g. "{ int i; i += 1; }".
struct CompoundStatement : public Node, public SymNode {
  CompoundStatement(Node *d,Node *s) :
    _declaration_list(d), _statement_list(s) {};

  virtual SymTab *symtab() const { return getsymtab(); };

  Node *declaration_list() const { return _declaration_list; };
  Node *statement_list() const { return _statement_list; };

  virtual void gensymtab(SymTab *);
  virtual void typecheck(Node *);
  virtual void flowcontrol(Node *,bool);
  virtual void codegen(CodeGen *);
protected:
  virtual void printdata(std::ostream &) const;
private:
  Node *_declaration_list;
  Node *_statement_list;
};

// A function definition (declaration and body).
struct FunctionDefn : public Node, public SymNode {
  FunctionDefn(Node *decl,Node *body);

  virtual String name() const { return _name; };
  Node *body() const { return _body; };
  void setcode(AsmStore *c) { _code = c; };

  virtual SymTab *symtab() const { return getsymtab(); };

  virtual bool is_extern() const { return _extern; };
  virtual bool is_static() const { return _static; };

  virtual void gensymtab(SymTab *);
  virtual void typecheck(Node *);
  virtual void flowcontrol(Node *,bool);
  virtual void codegen(CodeGen *);
  virtual void writecode(std::ostream &) const;
protected:
  virtual void printdata(std::ostream &) const;
private:
  String    _name;
  Node     *_body;
  AsmStore *_code;
  bool      _extern;
  bool      _static;
};

// A node representing a declaration of a function or
// variable.
struct Declaration : public Node {
  Declaration (String n,Type *t = 0);

  virtual String name() const { return _name; };

  virtual bool is_used() const { return _is_used; };
  virtual void set_used() { _is_used = true; };

  void set_base_type(Type *t);
  void add_type(Type *t);

  virtual bool is_extern() const { return _extern; };
  virtual bool is_static() const { return _static; };

  void set_extern() { _extern = true; };
  void set_static() { _static = true; };

  virtual void gensymtab(SymTab *);
protected:
  virtual void printdata(std::ostream &) const;
private:
  String  _name;
  bool    _extern;
  bool    _static;
  bool    _is_used;
};

//
// Type nodes. 
//

// This represents the type of another node, e.g. the
// type of 5+a, where a is an int, will be a Type node
// storing int.
// Types may be nested (chained together).
struct Type : public gc {
  Type();
  Type(Type *t) : _child(t) {};
  Type *child() const { return _child; };

  // Set the base (innermost) type ofa type.  For instance,
  // calling this with a pointer(int) type on a pointer() type
  // will give you a pointer(pointer(int)).
  void set_base_type(Type *t);
  // Returns the return type, if applicable (i.e. it's a function).
  virtual Type *get_return_type() const { return 0; };
  // Returns true if this represents a function.
  virtual bool is_function() const { return false; };
  // Returns size of type in bytes.
  virtual unsigned size() const;
  // Prints a string representation.
  virtual std::ostream &print(std::ostream &o) const = 0;
  // Prints only the outer most type.
  virtual std::ostream &print_outer(std::ostream &o) const = 0;

  // Traversal function for generating symbol tables.
  virtual void gensymtab(SymTab *);

protected:

  Type *_child;
};

// Use this to print the Type pointer, as it supports nulls.
std::ostream &operator<<(std::ostream &o,const Type *);

// Represents intrinsic types, e.g. int, char.
// Note:  The None type doesn't represent a type but is used to
// note that something is not a base type.
struct BaseType : public Type {
  enum BT { None, Int, Char, Double };
  BaseType(BT t) : _type(t) {};
  BT type() const { return _type; };
  virtual unsigned size() const;
  virtual std::ostream &print(std::ostream &o) const;
  virtual std::ostream &print_outer(std::ostream &o) const;
private:
  BT _type;
};

// A type representing a function (for prototypes and calls).
struct FunctionType : public Type {
  FunctionType(Node *p,Type *c = 0) : Type(c), _params(p) {};
  Node *get_params() const { return _params; };
  bool is_function() const { return true; };
  Type *get_return_type() const { return _child; };

  virtual std::ostream &print(std::ostream &o) const;
  virtual std::ostream &print_outer(std::ostream &o) const;

  virtual void gensymtab(SymTab *);
private:
  Node *_params;
};

struct PointerType : public Type {
  PointerType() {};
  PointerType(Type *t) : Type(t) {};
  virtual unsigned size() const;

  virtual std::ostream &print(std::ostream &o) const;
  virtual std::ostream &print_outer(std::ostream &o) const;
};

template <class T>
T *ncast(Node *n)
{
  return dynamic_cast<T *>(n);
}

template <class T>
T &ncastr(Node *n)
{
  return dynamic_cast<T &>(*n);
}

template <class T>
T *tcast(Type *t)
{
  return dynamic_cast<T *>(t);
}

template <class T>
T &tcastr(Type *t)
{
  return dynamic_cast<T &>(*t);
}

// Functions for querying types.

BaseType::BT intType(Type *t);
bool isIntType(Type *t);
bool isIntType(Node *n);
bool isPtrType(Type *t);
bool isPtrType(Node *n);

#endif
