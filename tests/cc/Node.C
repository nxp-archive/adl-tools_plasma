//
// AST nodes used by the compiler.
// These are garbage collected when allocated
// from the heap.  However, they do not register
// a finalizer, so thir destructors will not be called.
//

#include <sstream>
#include <stdexcept>
#include <assert.h>
#include <iostream>

#include "Types.h"
#include "Node.h"
#include "SymTab.h"
#include "CodeGen.h"
#include "AsmStore.h"
#include "cparse.h"

using namespace std;

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

// Per-indent amount of whitespace.
static const int Incr = 2;

static int IndentIndex = ostream::xalloc();
static int PrintSyms = ostream::xalloc();

int curindent(ostream &o)
{
  return o.iword(IndentIndex);
}

// Indentation manipulator- supply correct indentation.
ostream &indent(ostream &o)
{
  o << '\n';
  for (int i = 0; i < o.iword(IndentIndex); ++i) {
    o << ' ';
  }
  return o;
}

// Modify indentation.
ostream &incrindent(ostream &o)
{
  o.iword(IndentIndex) += Incr;
  return o;
}

// Modify indentation.
ostream &decrindent(ostream &o)
{
  o.iword(IndentIndex) -= Incr;
  return o;
}

// Indicate that symbols should be printed.
ostream &printsyms(ostream &o)
{
  o.iword(PrintSyms) = true;
  return o;
}

// Turn off symbol printing.
ostream &noprintsyms(ostream &o)
{
  o.iword(PrintSyms) = false;
  return o;
}

bool do_printsyms(ostream &o)
{
  return o.iword(PrintSyms);
}

// If the given type is a constant, coerce it to the
// given type.
void coerce_const(Node *n,Type *t)
{
  if (n->is_const() && isIntType(t)) {
    n->set_type(t);
  }
}

// Given two typed terminals, sees if one is a constant.
// If it is, tries to coerce it to the type of the other.
void coerce_consts(Node *n1,Node *n2)
{
  if (n1->is_const()) {
    coerce_const(n1,n2->type());
  } else if (n2->is_const()) {
    coerce_const(n2,n1->type());
  }
}

// Checks the given integeral constant to make sure it is
// within the bounds of the given type.
bool check_const_range(Node *n,Type *t)
{
  if (Const *c = ncast<Const>(n)) {
    int value = c->value();
    switch (intType(t)) {
    case BaseType::Char:
      return (value >= -128 && value <= 127);
    default:
      break;
    }
  }
  return true;
}

// Compares two types to see if it's possible to perform
// a binary operation on them.  If it is not, then the 
// appopriate error/warnings are raised, unless raise_errors is
// false, in which case the return code is:
// 0:  Success
// 1:  Warning
// 2:  Error
int compare_types(Node *name,Type *from,Type *to,bool raise = true)
{
  const int Warning = 1, Error = 2;
  int conflict = 0;
  BaseType::BT from_t = intType(from);
  BaseType::BT to_t = intType(to);
  if (from_t != to_t) {
    if (from_t == BaseType::Char) {
      if (to_t == BaseType::Int) {
        conflict = 0;
      } else {
        conflict = Error;
      }
    } else if (from_t == BaseType::Int) {
      if (to_t == BaseType::Char) {
        conflict = Warning;
      } else {
        conflict = Error;
      }
    } else {
      conflict = Error;
    }
  }
  if (!raise) {
    return conflict;
  }
  if (conflict == Warning) {
    Warn(name,"Conversion from " << from << " to " << to << " may result in data loss.");
  } else if (conflict == Error) {
    Error(name,"Cannot convert from " << from << " to " << to);
  }
  return conflict;
}

Node::Node(Type *t) : 
  _has_return_stmt(false),
  _has_addr(false), 
  _output_addr(0), 
  _compile_loc(0),
  _type(t), 
  _coerce_to_type(0), 
  _filename(0), 
  _linenumber(0) 
{};

// Doesn't do anything, but is sometimes required due to C++ bullshit.
Node::~Node()
{
}

void Node::setFileData(const char *fn,int ln)
{
  _filename = fn;
  _linenumber = ln;
}

pair<int,bool> Node::calculate() const
{ 
  return make_pair(0,false); 
};

void Node::add(Node *n) 
{ 
  assert(0); 
}

void Node::print(ostream &o) const
{
  o << incrindent << indent << "+ " << typeid(*this).name() << incrindent;
  if (_type) {
    o << indent << "Type-string:  " << _type;
  }
  printdata(o);
  o << decrindent << decrindent;
}

void Node::printdata(std::ostream &) const
{
}

std::ostream &operator<<(std::ostream &o,const Node *n)
{
  if (n) {
    n->print(o);
  } else {
    o << incrindent << indent << "null" << decrindent;
  }
  return o;
}

void Node::gensymtab()
{
  gensymtab(0);
}

void Node::typecheck()
{
  typecheck(0);
}

void Node::flowcontrol()
{
  flowcontrol(0,false);
}

bool Node::dochecks(CodeGen &)
{
  return true;
}

void Node::codegen(CodeGen &)
{
}

void Node::writecode(ostream &) const
{
}

void Node::gensymtab(SymTab *)
{
}

void Node::typecheck(Node *)
{
}

void Node::flowcontrol(Node *,bool)
{
}

NullNode::NullNode() : Node(0) 
{}

void ArrayExpression::printdata(ostream &o) const
{
  o << indent << "Expr : " << _expr
    << indent << "Index: " << _index;
}

void ArrayExpression::gensymtab(SymTab *p)
{
  _expr->gensymtab(p);
  _index->gensymtab(p);
}

void ArrayExpression::typecheck(Node *curr_func)
{
  _expr->typecheck(curr_func);
  _index->typecheck(curr_func);
  if (!isIntType(_index)) {
    Error(_index,"Array index is not an int or char.");
  } else if (!isPtrType(_expr)) {
    Error(_expr,"Array expression is not a pointer.");
  } else {
    _type = _expr->type()->child();
    set_has_address();
  }
}

void ArrayExpression::codegen(CodeGen &cg)
{
  cg.genArrayExpression(this);
}

StringLiteral::StringLiteral(String s) : 
  Node(new PointerType(new BaseType(BaseType::Char))),
  _s(s)
{
}

void StringLiteral::append(String s)
{
  _s = _s.append(s);
}

void StringLiteral::printdata(ostream &o) const
{
  o << indent << "Data:  \"" << _s << "\"";
}

void StringLiteral::codegen(CodeGen &cg)
{
  cg.genStringLiteral(this);
}

void Id::printdata(ostream &o) const
{
  o << indent << "Name:  " << _id;
}

void Id::gensymtab(SymTab *p)
{
  if (Node *s = p->find(_id)) {
    _has_addr = true;
    _symbol = s;
    _symbol->set_used();
  } else {
    Error(this,"Unknown identifier " << _id);
  }
}

void Id::typecheck(Node *curr_func)
{
  assert(_symbol);
  _type = _symbol->type();
}

void Id::codegen(CodeGen &cg)
{
  cg.genId(this);
}

void Const::printdata(ostream &o) const
{
  o << indent << "Value:  " << _value;
}

void Const::codegen(CodeGen &cg)
{
  cg.genConst(this);
}

Node::Calc Const::calculate() const
{
  return make_pair(_value,true);
}

Node *get_calculated(Node *node)
{
  pair<int,bool> res = node->calculate();
  if (res.second) {
    return new Const(res.first,new BaseType(BaseType::Int));
  } else {
    return node;
  }
}

void Unaryop::printdata(ostream &o) const
{
  o << indent << "Expr:  " << _expr;
}

void Unaryop::gensymtab(SymTab *p)
{
  _expr->gensymtab(p);
}

void Unaryop::typecheck(Node *curr_func)
{
  _expr->typecheck(curr_func);
}

Node::Calc Negative::calculate() const
{
  Calc cr = _expr->calculate();
  if (cr.second) {
    return make_pair(-cr.first,true);
  } else {
    return cr;
  }
}

void Negative::typecheck(Node *curr_func)
{
  Unaryop::typecheck(curr_func);
  _type = _expr->type();
}

void Negative::codegen(CodeGen &cg)
{
  cg.genNegative(this);
}

void AddrOf::typecheck(Node *curr_func)
{
  Unaryop::typecheck(curr_func);
  if (!_expr->has_address()) {
    Error(this,"Address-of (&) target has no address.");
  } else {
    _expr->set_output_addr(1);
    _type = new PointerType(_expr->type());
  }
}

void AddrOf::codegen(CodeGen &cg)
{
  cg.genAddrOf(this);
}

void Pointer::typecheck(Node *curr_func)
{
  Unaryop::typecheck(curr_func);
  if (PointerType *pt = tcast<PointerType>(_expr->type())) {
    _type = pt->child();
    set_has_address();
  } else {
    Error(this,"Pointer dereference (*) target is not a pointer.");
  }
}

void Pointer::codegen(CodeGen &cg)
{
  cg.genPointer(this);
}

Node::Calc Binop::calculate() const
{
  Calc cl = _left->calculate();
  if (!cl.second) {
    return cl;
  }
  Calc cr = _right->calculate();
  if (!cr.second) {
    return cr;
  }
  int l = cl.first, r = cl.second;
  int res;
  switch (_op) {
  case PLUS:
    res = l + r;
    break;
  case MINUS:
    res = l - r;
    break;
  case ASTERISK:
    res = l * r;
    break;
  case DIV:
    res = l / r;
    break;
  case MODULO:
    res = l % r;
    break;
  case ADD_ASSIGN:
    res = l + r;
    break;
  case SUB_ASSIGN:
    res = l - r;
    break;
  case EQ:
    res = (l == r);
    break;
  case NOT_EQ:
    res = (l != r);
    break;
  case LESS:
    res = (l < r);
    break;
  case GREATER:
    res = (l > r);
    break;
  case LESS_EQ:
    res = (l <= r);
    break;
  case GREATER_EQ:
    res = (l >= r);
    break;
  default: {
    ostringstream ss;
    ss << "Unknown binary operator " << _op;
    throw runtime_error(ss.str());
  }
  }
  return Calc(res,true);
}

const char *Binop::op_str() const
{
  switch (_op) {
  case PLUS:
    return "+";
  case MINUS:
    return "-";
  case ASTERISK:
    return "*";
  case DIV:
    return "/";
  case MODULO:
    return "%";
  case ASSIGN:
    return "=";
  case ADD_ASSIGN:
    return "+=";
  case SUB_ASSIGN:
    return "-=";
  case EQ:
    return "==";
  case NOT_EQ:
    return "!=";
  case LESS:
    return "<";
  case GREATER:
    return ">";
  case LESS_EQ:
    return "<=";
  case GREATER_EQ:
    return ">=";
  default: {
    ostringstream ss;
    ss << "Unknown binary operator " << _op;
    throw runtime_error(ss.str());
  }
  }
}

void Binop::printdata(ostream &o) const
{
  o << indent << "Left:  " << _left
    << indent << "Op  :  " << op_str()
    << indent << "Right:  " << _right;
}

void Binop::gensymtab(SymTab *p)
{
  _left->gensymtab(p);
  _right->gensymtab(p);
}

bool Binop::is_assign_op() const
{
  switch (_op) {
  case ASSIGN:
  case ADD_ASSIGN:
  case SUB_ASSIGN:
    return true;
  default:
    return false;
  }
}

bool Binop::is_compare_op() const
{
  switch (_op) {
  case EQ:
  case NOT_EQ:
  case LESS:
  case GREATER:
  case LESS_EQ:
  case GREATER_EQ:
    return true;
  default:
    return false;
  }
}

void Binop::typecheck(Node *curr_func)
{
  _left->typecheck(curr_func);
  _right->typecheck(curr_func);
  if (is_assign_op()) {
    if (!_left->has_address()) {
      Error(this,"Invalid lvalue:  Not an address.");
    }
    _left->set_output_addr(1);
    coerce_const(_right,_left->type());
    compare_types(_left,_right->type(),_left->type());
    _right->set_coerce_to_type(_left->type());
    _type = _left->type();
  } else {
    coerce_consts(_left,_right);
    bool left_conflicts = compare_types(_left,_left->type(),_right->type(),false);
    bool right_conflicts = compare_types(_right,_right->type(),_left->type(),false);
    Node *from, *to;
    if (left_conflicts < right_conflicts) {
      from = _right;
      to = _left;
    } else {
      from = _left;
      to = _right;
    }
    compare_types(from,from->type(),to->type());
    from->set_coerce_to_type(to->type());
    to->set_coerce_to_type(to->type());
    _type = to->type();
  }
}

void Binop::codegen(CodeGen &cg)
{
  cg.genBinop(this);
}

void IfStatement::printdata(ostream &o) const
{
  o << indent << "Expr:  " << _expr
    << indent << "Then:  " << _then
    << indent << "Else:  " << _else;
}

void IfStatement::gensymtab(SymTab *p)
{
  _expr->gensymtab(p);
  _then->gensymtab(p);
  _else->gensymtab(p);
}

// Simple type checking on an expression used by a statement
// that expects a boolean value, e.g. if, while, etc.  Throws
// an exception if there's a typing problem.
void process_conditional(Node *expr)
{
  if (!isIntType(expr)) {
    Error(expr,"Expression must be coercable to a boolean (must be int or char).");
  }
}

void IfStatement::typecheck(Node *curr_func)
{
  _expr->typecheck(curr_func);
  process_conditional(_expr);
  _then->typecheck(curr_func);
  _else->typecheck(curr_func);
}

void IfStatement::flowcontrol(Node *curr_func,bool in_loop)
{
  _then->flowcontrol(curr_func,in_loop);
  _else->flowcontrol(curr_func,in_loop);
  if (_then->has_return_stmt() && _else->has_return_stmt()) {
    set_has_return_stmt(true);
  }
}

void IfStatement::codegen(CodeGen &cg)
{
  cg.genConditional(this);
}

void BreakStatement::flowcontrol(Node *,bool in_loop)
{
  if (!in_loop) {
    Error(this,"Break statement outside of any loop.");
  }
}

void BreakStatement::codegen(CodeGen &cg)
{
  cg.genBreak(this);
}

void ContinueStatement::flowcontrol(Node *,bool in_loop)
{
  if (!in_loop) {
    Error(this,"Continue statement outside of any loop.");
  }
}

void ContinueStatement::codegen(CodeGen &cg)
{
  cg.genContinue(this);
}

void ForLoop::printdata(ostream &o) const
{
  o << indent << "Begin:  " << _begin
    << indent << "Expr :  " << _expr
    << indent << "End  :  " << _end
    << indent << "Stmt :  " << _stmt;
}

void ForLoop::gensymtab(SymTab *p)
{
  _begin->gensymtab(p);
  _expr->gensymtab(p);
  _end->gensymtab(p);
  _stmt->gensymtab(p);
}

void ForLoop::typecheck(Node *curr_func)
{
  _begin->typecheck(curr_func);
  _expr->typecheck(curr_func);
  process_conditional(_expr);
  _end->typecheck(curr_func);
  _stmt->typecheck(curr_func);
}

void ForLoop::flowcontrol(Node *curr_func,bool in_loop)
{
  _stmt->flowcontrol(curr_func,true);
  set_has_return_stmt(_stmt->has_return_stmt());
}

void ForLoop::codegen(CodeGen &cg)
{
  cg.genForLoop(this);
}

void WhileLoop::printdata(ostream &o) const
{
  o << indent << "Expr:" << _expr
    << indent << "Stmt:" << _stmt;
}

void WhileLoop::gensymtab(SymTab *p)
{
  _expr->gensymtab(p);
  _stmt->gensymtab(p);
}

void WhileLoop::typecheck(Node *curr_func)
{
  _expr->typecheck(curr_func);
  process_conditional(_expr);
  _stmt->typecheck(curr_func);
}

void WhileLoop::flowcontrol(Node *curr_func,bool in_loop)
{
  _stmt->flowcontrol(curr_func,true);
  set_has_return_stmt(_stmt->has_return_stmt());
}

void WhileLoop::codegen(CodeGen &cg)
{
  cg.genWhileLoop(this);
}

void NodeList::printdata(ostream &o) const
{
  for (const_iterator i = begin(); i != end(); ++i) {
    o << *i;
  }
}

void NodeList::gensymtab(SymTab *p)
{
  for (iterator i = begin(); i != end(); ++i) {
    (*i)->gensymtab(p);
  }
}

void NodeList::typecheck(Node *curr_func)
{
  for (iterator i = begin(); i != end(); ++i) {
    (*i)->typecheck(curr_func);
  }  
}

void NodeList::codegen(CodeGen &cg)
{
  cg.genList(this);
}

void NodeList::writecode(std::ostream &out) const
{
  for (const_iterator i = begin(); i != end(); ++i) {
    (*i)->writecode(out);
  }
}

NodeList &FunctionExpression::get_arglist() const
{
  return ncastr<NodeList>(arglist());
}

void FunctionExpression::printdata(ostream &o) const
{
  o << indent << "Name:" << _function
    << indent << "Args:" << _arglist;
}

void FunctionExpression::gensymtab(SymTab *p)
{
  _function->gensymtab(p);
  _arglist->gensymtab(p);
}

ParamList *get_params(Node *n)
{
  FunctionType &ft = tcastr<FunctionType>(n->type());
  return &(ncastr<ParamList>(ft.get_params()));
}

int nlsize(Node *n)
{
  NodeList &nl = ncastr<NodeList>(n);
  return nl.size();
}

Node *getsymbol(Node *n)
{
  Id &id = ncastr<Id>(n);
  return id.symbol();
}

void FunctionExpression::typecheck(Node *curr_func)
{
  _function->typecheck(curr_func);
  if (!_function->type()->is_function()) {
    Error(_function,"Target of function expression is not a function!");
  }
  Node *symbol = getsymbol(_function);
  _type = symbol->type()->get_return_type();
  _arglist->typecheck(curr_func);
  ParamList *params = get_params(symbol);
  int num_args = nlsize(_arglist);
  int num_params = nlsize(params);
  if (!params->has_ellipsis() && num_args > num_params) {
    Error(this,"Too many arguments passed to function.");
  } else if (num_args < num_params) {
    Error(this,"Too few arguments passed to function.");
  }
  NodeList &args = ncastr<NodeList>(_arglist);
  NodeList::iterator param = params->begin();
  NodeList::iterator arg = args.begin();
  for (; param != params->end(); ++arg, ++param) {
    coerce_const(*arg,(*param)->type());
    compare_types(this,(*arg)->type(),(*param)->type());
    (*arg)->set_coerce_to_type((*param)->type());
  }
  // Deal with variable number of arguments here.
  if (params->has_ellipsis() && (num_args > num_params)) {
    for ( ; arg != args.end(); ++arg) {
      (*arg)->set_coerce_to_type((*arg)->type());
    }
  }
}

void FunctionExpression::codegen(CodeGen &cg)
{
  cg.genFunctionExpression(this);
}

void SymNode::printsyms(ostream &o) const
{
  if (do_printsyms(o)) {
    o << indent << incrindent << "Symbols:" << _symtab << decrindent;
  }
}

void CompoundStatement::printdata(ostream &o) const
{
  o << indent << "Decls:" << _declaration_list
    << indent << "Stmts:" << _statement_list;
  printsyms(o);
}

void CompoundStatement::gensymtab(SymTab *p)
{
  _symtab = new SymTab(p);
  _declaration_list->gensymtab(_symtab);
  _statement_list->gensymtab(_symtab);
}

void CompoundStatement::typecheck(Node *curr_func)
{
  _statement_list->typecheck(curr_func);
}

void CompoundStatement::flowcontrol(Node *curr_func,bool in_loop)
{
  _statement_list->flowcontrol(curr_func,in_loop);
  set_has_return_stmt(_statement_list->has_return_stmt());
}

void CompoundStatement::codegen(CodeGen &cg)
{
  cg.genCompoundStatement(this);
}

FunctionDefn::FunctionDefn(Node *decl,Node *body) :
  _body(body)
{
  Declaration &d = dynamic_cast<Declaration&>(*decl);
  _type = d.type();
  _name = d.name();
  _extern = d.is_extern();
  _static = d.is_static();
}

void FunctionDefn::printdata(ostream &o) const
{
  o << indent << "Extern:  " << _extern
    << indent << "Static:  " << _static
    << indent << "Name  :  " << _name
    << indent << "Body  :" << _body;
  printsyms(o);
}

void FunctionDefn::gensymtab(SymTab *p)
{
  p->add(_name,this);
  _symtab = new SymTab(p);
  _type->gensymtab(_symtab);
  _body->gensymtab(_symtab);
}

void FunctionDefn::typecheck(Node *curr_func)
{
  _body->typecheck(this);
}

void FunctionDefn::flowcontrol(Node *curr_func,bool in_loop)
{
  _body->flowcontrol(this,false);
  if (!_body->has_return_stmt()) {
    Warn(this,"Function " << name() << " does not return through all branches.");
  }
}

void FunctionDefn::codegen(CodeGen &cg)
{
  cg.genFunctionDefn(this);
}

void FunctionDefn::writecode(ostream &out) const
{
  if (_code) {
    _code->write(out);
  }
}

void ReturnStatement::printdata(ostream &o) const
{
  o << indent << "Expr:  " << _expr;
}

void ReturnStatement::gensymtab(SymTab *p)
{
  _expr->gensymtab(p);
}

void ReturnStatement::typecheck(Node *curr_func)
{
  _expr->typecheck(curr_func);
  Type *return_type = curr_func->type()->get_return_type();
  coerce_const(_expr,return_type);
  compare_types(this,_expr->type(),return_type);
  _expr->set_coerce_to_type(return_type);
}

void ReturnStatement::flowcontrol(Node *curr_func,bool in_loop)
{
  set_has_return_stmt(true);
}

void ReturnStatement::codegen(CodeGen &cg)
{
  cg.genReturn(this);
}

Declaration::Declaration (String n,Type *t) :
  Node(t), _name(n),
  _extern(false), _static(false), _is_used(false)
{
}

void Declaration::set_base_type(Type *t)
{
  assert(t);
  if (!_type) {
    _type = t;
  } else {
    _type->set_base_type(t);
  }
}

void Declaration::add_type(Type *t)
{
  t->set_base_type(_type);
  _type = t;
}

void Declaration::printdata(ostream &o) const
{
  o << indent << "Extern:  " << _extern
    << indent << "Static:  " << _static
    << indent << "Used  :  " << _is_used
    << indent << "Name  :  " << _name;
}

void Declaration::gensymtab(SymTab *p)
{
  p->add(_name,this);
}

void StatementList::flowcontrol(Node *curr_func,bool in_loop)
{
  for (iterator i = begin(); i != end(); ++i) {
    if ( has_return_stmt() ) {
      Warn(this,"Function " << curr_func->name() << " has at least one unreachable statement.");
    }
    (*i)->flowcontrol(curr_func,in_loop);
    if ( (*i)->has_return_stmt() ) {
      set_has_return_stmt(true);
    }
  }
}

void StatementList::codegen(CodeGen &cg)
{
  cg.genStatementList(this);
}

void TranslationUnit::printdata(std::ostream &o) const
{
  NodeList::printdata(o);
  printsyms(o);
}

void TranslationUnit::gensymtab(SymTab *p)
{
  // This should be a top-level item w/no parent.
  assert(!p);
  _symtab = new SymTab();
  NodeList::gensymtab(_symtab);
}

bool TranslationUnit::dochecks(CodeGen &cg)
{
  return cg.dochecks(this);
}

void TranslationUnit::codegen(CodeGen &cg)
{
  cg.genTranslationUnit(this);
}


ostream &operator<<(ostream &o,const Type *t)
{
  if (!t) {
    o << "void";
  } else {
    t->print(o);
  }
  return o;
}

Type::Type() : _child(0)
{
}

void Type::set_base_type(Type *t)
{
  if (!_child) {
    _child = t;
  } else {
    _child->set_base_type(t);
  }
}

void Type::gensymtab(SymTab *parent)
{
}

unsigned Type::size() const
{
  throw runtime_error("Cannot calculate size for type.");
}

unsigned BaseType::size() const
{
  switch (_type) {
  case Int:
    return IntSize;
  case Char:
    return CharSize;
  case Double:
    return DoubleSize;
  default:
    return Type::size();
  }
}

ostream &BaseType::print(ostream &o) const
{
  switch (_type) {
  case Int:
    o << "int";
    break;
  case Char:
    o << "char";
    break;
  case Double:
    o << "double";
    break;
  case None:
    o << "<none>";
    break;
  }
  return o;
}

ostream &BaseType::print_outer(ostream &o) const
{
  return print(o);
}

ostream &FunctionType::print(ostream &o) const
{
  o << "function(";
  NodeList &nl = dynamic_cast<NodeList &>(*_params);
  bool first = true;
  for (NodeList::const_iterator i = nl.begin(); i != nl.end(); ++i) {
    if (!first)
      o << ",";
    (*i)->type()->print(o);
    first = false;
  }
  o << ")->" << _child;
  return o;
}

void FunctionType::gensymtab(SymTab *p)
{
  _params->gensymtab(p);
}

ostream &FunctionType::print_outer(ostream &o) const
{
  o << "function";
  return o;
}

unsigned PointerType::size() const
{
  return 4;
}

ostream &PointerType::print(std::ostream &o) const
{
  o << "pointer(";
  _child->print(o);
  o << ")";
  return o;
}
    
ostream &PointerType::print_outer(std::ostream &o) const
{
  o << "pointer";
  return o;
}

BaseType::BT intType(Type *t)
{
  if (BaseType *bt = tcast<BaseType>(t)) {
    return bt->type();
  }
  return BaseType::None;
}

// Returns true if the object has an integer type (int or char).
bool isIntType(Type *t)
{
  switch (intType(t)) {
  case BaseType::None:
    return false;
  default:
    return true;
  }
}

// Returns true if the object has an integer type (int or char).
bool isIntType(Node *n)
{
  return isIntType(n->type());
}

// Returns true if we have a pointer type.
bool isPtrType(Type *t)
{
  return (tcast<PointerType>(t));
}

bool isPtrType(Node *n)
{
  return isPtrType(n->type());
}
