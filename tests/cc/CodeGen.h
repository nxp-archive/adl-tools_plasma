//
// Main code generation class.  This is designed to be re-entrant so
// that it can handle multiple threads.  Non-reentrant stuff is encapsulated
// within a single mutex class that's not visible in the header.  This
// allows the class to be used by pure C, even though the implementation
// is in Plasma.
//

#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include <string>

struct Operand;
class StackMachine;
class AsmStore;
class SharedData;
class ThreadData;
class Node;
class CData;
class Type;
class SymTab;
class FunctionDefn;
class NodeList;
class StatementList;
class IfStatement;
class WhileLoop;
class ForLoop;
class BreakStatement;
class ContinueStatment;
class StringLiteral;
class Const;
class Id;
class ArrayExpression;
class FunctionExpression;
class ReturnStatement;
class Binop;
class Negative;
class Pointer;
class AddrOf;

class CodeGen {
public:
  // This allocates a new common-data object.
  CodeGen(bool print_comments = true);
  // This inherits a common-data object.
  CodeGen(const CodeGen &);

  void handleGlobals(Node *unit);

  void write(const std::string &,Node *) const;

  // Used by Node for code generation.
  void genList(NodeList *);
  void genStatementList(StatementList *);
  void genFunctionDefn(FunctionDefn *);
  void genConditional(IfStatement *);
  void genWhileLoop(WhileLoop *);
  void genForLoop(ForLoop *);
  void genBreak(BreakStatement *);
  void genContinue(ContinueStatement *);
  void genStringLiteral(StringLiteral *);
  void genCompoundStatement(CompoundStatement *);
  void genConst(Const *);
  void genId(Id *);
  void genArrayExpression(ArrayExpression *);
  void genFunctionExpression(FunctionExpression *);
  void genReturn(ReturnStatement *);
  void genBinop(Binop *bo);
  void genNegative(Negative *n);
  void genPointer(Pointer *n);
  void genAddrOf(AddrOf *n);
private:
  AsmStore &code();
  StackMachine &stack();

  void push_loop_labels(const std::string &break_label,const std::string &continue_label);
  void pop_loop_labels();
  const std::string &break_label() const;
  const std::string &continue_label() const;

  unsigned asm_var_size(Type *t);
  unsigned calc_var_align(Type *t);
  const char *binop(int) const;
  std::string new_label();
  std::string new_str_label(String s);
  void empty_stack(Node *);
  void accept_and_empty_stack(Node *);
  int calc_local_var_addrs(SymTab *symtab,int last_fp_loc);
  void calc_function_arg_addrs(SymTab *symtab);
  int calc_function_var_addrs(FunctionDefn *fd,int last_fp_loc);
  Operand accept_and_pop(Node *na);
  void binop_assign(Binop *bo);
  void binop_arith(Binop *bo);
  void binop_compare(Binop *bo);

  // Shared data for code generation.
  SharedData *_sdata;

  // Per-thread code generation data.
  ThreadData *_tdata;
};

#endif

