//
// AST nodes used by the compiler.
// These are garbage collected when allocated
// from the heap.  However, they do not register
// a finalizer, so thir destructors will not be called.
//

#include <sstream>
#include <stdexcept>

#include "Node.h"
#include "cparse.h"

using namespace std;

// Per-indent amount of whitespace.
static int Incr = 2;

// Helper class for indenting by a given amount of
// white space.
struct Indent {
  Indent(int n) : _n(n) {};
  int _n;
};

ostream &operator<<(ostream &o,Indent indent)
{
  for (int i = 0; i != indent._n; ++i) {
    o << " ";
  }
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

NullNode::NullNode() : Node(new VoidType) 
{}

void NullNode::print(ostream &o,int) const
{
  o << "null";
}

void ArrayExpression::print(ostream &o,int) const
{
  o << _expr << "[" << _index << "]";
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

void StringLiteral::print(ostream &o,int) const
{
  o << _s;
}

void Id::print(ostream &o,int) const
{
  o << _id;
}

void Const::print(ostream &o,int) const
{
  o << _value;
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

Node::Calc Negative::calculate() const
{
  Calc cr = _expr->calculate();
  if (cr.second) {
    return make_pair(-cr.first,true);
  } else {
    return cr;
  }
}

void Negative::print(ostream &o,int) const
{
  o << "-" << _expr;
}

void Pointer::print(ostream &o,int) const
{
  o << "*(" << _expr << ")";
}

void AddrOf::print(ostream &o,int) const
{
  o << "&(" << _expr << ")";
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

void Binop::print(ostream &o,int) const
{
  o << "(" << _left << " ";
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
    o << "+";
    break;
  case SUB_ASSIGN:
    o << "-";
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
  default: {
    ostringstream ss;
    ss << "Unknown binary operator " << _op;
    throw runtime_error(ss.str());
  }
  } 
  o << " " << _right << ")";
}

void IfStatement::print(ostream &o,int indent) const
{
  o << Indent(indent) << "if (" << _expr << ") {\n";
  _then->print(o,indent+Incr);
  o << Indent(indent) << "} else {\n";
  _else->print(o,indent+Incr);
  o << Indent(indent) << "}\n";
}

void BreakStatement::print(ostream &o,int indent) const
{
  o << Indent(indent) << "break;\n";
}

void ContinueStatement::print(ostream &o,int indent) const
{
  o << Indent(indent) << "continue;\n";
}

void ReturnStatement::print(ostream &o,int indent) const
{
  o << Indent(indent) << "return;\n";
}

void ForLoop::print(ostream &o,int indent) const
{
  o << Indent(indent) << "for (" << _begin << "; " << _expr << "; " << _end << ")\n";
  _stmt->print(o,indent+Incr);
}

void WhileLoop::print(ostream &o,int indent) const
{
  o << Indent(indent) << "while (" << _expr << ")\n";
  _stmt->print(o,indent+Incr);
}

void NodeList::print_list(ostream &o,const char *sep,int indent) const
{
  bool first = true;
  for (const_iterator i = begin(); i != end(); ++i) {
    if (indent) {
      o << Indent(indent);
    }
    if (!first) o << sep;
    first = false;
    o << *i;
    if (indent) {
      o << "\n";
    }
  }
}

void ArgumentList::print(ostream &o,int indent) const
{
  print_list(o,", ");
}

void ParamList::print(ostream &o,int indent) const
{
  print_list(o,", ");
  if (_hasellipsis) {
    o << ", ...";
  }
}

void StatementList::print(ostream &o,int indent) const
{
  print_list(o,";",indent);
}

void TranslationUnit::print(ostream &o,int indent) const
{
  print_list(o,"\n",indent);
}

void DeclarationList::print(ostream &o,int indent) const
{
  print_list(o,"\n",indent);
}

void FunctionExpression::print(ostream &o,int indent) const
{
  o << _function << "(" << _arglist << ")";
}

void CompoundStatement::print(ostream &o,int indent) const
{
  o << Indent(indent) << "{\n";
  _declaration_list->print(o,indent+Incr);
  _statement_list->print(o,indent+Incr);
  o << "}\n";
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

void FunctionDefn::print(ostream &o,int indent) const
{
  o << Indent(indent);
  if (_extern) {
    o << "extern ";
  }
  if (_static) {
    o << "static ";
  }
  o << _name << endl;
  _body->print(o,indent+Incr);
}

Declaration::Declaration (Node *n,Type *t) :
  Node(t), _name(n),
  _extern(false), _static(false), _is_used(false)
{
}

void Declaration::set_base_type(Type *t)
{
  assert(t);
  if (!t) {
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

void Declaration::print(ostream &o,int indent) const
{
  o << Indent(indent);
  if (_extern) {
    o << "extern ";
  }
  if (_static) {
    o << "static ";
  }
  o << _name << endl;
  _body->print(o,indent+Incr);
}

Type::Type() : _child(new VoidType)
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

ostream &VoidType::print(std::ostream &o) const
{
  o << "void";
  return o;
}
    
ostream &VoidType::print_outer(std::ostream &o) const
{
  o << "void";
  return o;
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
  o << ")->";
  _child->print(o);
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
