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
#include "cparse.h"

using namespace std;

// Per-indent amount of whitespace.
static const int Incr = 2;

static int IndentIndex = ostream::xalloc();

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

NullNode::NullNode() : Node(0) 
{}

void ArrayExpression::printdata(ostream &o) const
{
  o << indent << "Expr : " << _expr
    << indent << "Index: " << _index;
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

void IfStatement::printdata(ostream &o) const
{
  o << indent << "Expr:  " << _expr
    << indent << "Then:  " << _then
    << indent << "Else:  " << _else;
}

void ForLoop::printdata(ostream &o) const
{
  o << indent << "Begin:  " << _begin
    << indent << "Expr :  " << _expr
    << indent << "End  :  " << _end
    << indent << "Stmt :  " << _stmt;
}

void WhileLoop::printdata(ostream &o) const
{
  o << indent << "Expr:" << _expr
    << indent << "Stmt:" << _stmt;
}

void NodeList::printdata(ostream &o) const
{
  for (const_iterator i = begin(); i != end(); ++i) {
    o << *i;
  }
}

void FunctionExpression::printdata(ostream &o) const
{
  o << indent << "Name:" << _function
    << indent << "Args:" << _arglist;
}

void CompoundStatement::printdata(ostream &o) const
{
  o << indent << "Decls:" << _declaration_list
    << indent << "Stmts:" << _statement_list;
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
}

void ReturnStatement::printdata(ostream &o) const
{
  o << indent << "Expr:  " << _expr;
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

//
// List double-dispatch walk functions here.
//

#define WalkDefn(n)\
  void n::walk(Visitor &x) { \
    x.visit##n(this); \
  }

WalkDefn(NullNode);
WalkDefn(ArrayExpression);
WalkDefn(StringLiteral);
WalkDefn(Id);
WalkDefn(Const);
WalkDefn(Unaryop);
WalkDefn(Negative);
WalkDefn(Pointer);
WalkDefn(AddrOf);
WalkDefn(Binop);
WalkDefn(IfStatement);
WalkDefn(BreakStatement);
WalkDefn(ContinueStatement);
WalkDefn(ReturnStatement);
WalkDefn(ForLoop);
WalkDefn(WhileLoop);
WalkDefn(ArgumentList);
WalkDefn(ParamList);
WalkDefn(StatementList);
WalkDefn(TranslationUnit);
WalkDefn(DeclarationList);
WalkDefn(FunctionExpression);
WalkDefn(CompoundStatement);
WalkDefn(FunctionDefn);
WalkDefn(Declaration);
