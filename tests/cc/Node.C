//
// AST nodes used by the compiler.
// These are garbage collected when allocated
// from the heap.  However, they do not register
// a finalizer, so thir destructors will not be called.
//

#include <sstream>
#include <stdexcept>
#include <assert.h>

#include "Node.h"
#include "SymTab.h"
#include "cparse.h"

using namespace std;

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

void Node::gensymtab(SymTab *)
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

void Const::printdata(ostream &o) const
{
  o << indent << "Value:  " << _value;
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

Node::Calc Negative::calculate() const
{
  Calc cr = _expr->calculate();
  if (cr.second) {
    return make_pair(-cr.first,true);
  } else {
    return cr;
  }
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

void Binop::printdata(ostream &o) const
{
  o << indent << "Left:  " << _left
    << indent << "Op  :  ";
  switch (_op) {
  case PLUS:
    o << "+";
    break;
  case MINUS:
    o << "-";
    break;
  case ASTERISK:
    o << "*";
    break;
  case DIV:
    o << "/";
    break;
  case MODULO:
    o << "%";
    break;
  case ADD_ASSIGN:
    o << "+=";
    break;
  case SUB_ASSIGN:
    o << "-=";
    break;
  case EQ:
    o << "==";
    break;
  case NOT_EQ:
    o << "!=";
    break;
  case LESS:
    o << "<";
    break;
  case GREATER:
    o << ">";
    break;
  case LESS_EQ:
    o << "<=";
    break;
  case GREATER_EQ:
    o << ">=";
    break;
  case ASSIGN:
    o << "=";
    break;
  default: {
    ostringstream ss;
    ss << "Unknown binary operator " << _op;
    throw runtime_error(ss.str());
  }
  }
  o << indent << "Right:  " << _right;
}

void Binop::gensymtab(SymTab *p)
{
  _left->gensymtab(p);
  _right->gensymtab(p);
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

void CompoundStatement::printdata(ostream &o) const
{
  o << indent << "Decls:" << _declaration_list
    << indent << "Stmts:" << _statement_list;
  printsyms(o);
}

void SymNode::printsyms(ostream &o) const
{
  if (do_printsyms(o)) {
    o << indent << incrindent << "Symbols:" << _symtab << decrindent;
  }
}

void CompoundStatement::gensymtab(SymTab *p)
{
  _symtab = new SymTab(p);
  _declaration_list->gensymtab(_symtab);
  _statement_list->gensymtab(_symtab);
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

void ReturnStatement::printdata(ostream &o) const
{
  o << indent << "Expr:  " << _expr;
}

void ReturnStatement::gensymtab(SymTab *p)
{
  if (_expr) {
    _expr->gensymtab(p);
  }
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

