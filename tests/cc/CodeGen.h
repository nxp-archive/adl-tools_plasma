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

#include "AsmStore.h"
#include "StackMachine.h"

class Node;
class CData;
class Type;
class CodeGenArg;
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
  CodeGen(bool print_comments = true);

  void handleGlobals(Node *unit);

  void write(const std::string &,Node *) const;

  // Used by Node for code generation.
  void genList(NodeList *,CodeGenArg *);
  void genStatementList(StatementList *,CodeGenArg *);
  void genFunctionDefn(FunctionDefn *,CodeGenArg *);
  void genConditional(IfStatement *,CodeGenArg *);
  void genWhileLoop(WhileLoop *,CodeGenArg *);
  void genForLoop(ForLoop *,CodeGenArg *);
  void genBreak(BreakStatement *,CodeGenArg *);
  void genContinue(ContinueStatement *,CodeGenArg *);
  void genStringLiteral(StringLiteral *,CodeGenArg *);
  void genConst(Const *,CodeGenArg *);
  void genId(Id *,CodeGenArg *);
  void genArrayExpression(ArrayExpression *ae,CodeGenArg *cga);
  void genFunctionExpression(FunctionExpression *ae,CodeGenArg *cga);
  void genReturn(ReturnStatement *,CodeGenArg *);
  void genBinop(Binop *bo,CodeGenArg *cga);
  void genNegative(Negative *n,CodeGenArg *cga);
  void genPointer(Pointer *n,CodeGenArg *cga);
  void genAddrOf(AddrOf *n,CodeGenArg *cga);
private:
  unsigned asm_var_size(Type *t);
  unsigned calc_var_align(Type *t);
  const char *binop(int) const;
  std::string new_label();
  std::string new_str_label(String s);
  void empty_stack(Node *,CodeGenArg *);
  void accept_and_empty_stack(Node *,CodeGenArg *);
  int calc_local_var_addrs(SymTab *symtab,int last_fp_loc);
  void calc_function_arg_addrs(SymTab *symtab);
  int calc_function_var_addrs(FunctionDefn *fd,int last_fp_loc);
  Operand accept_and_pop(Node *n,CodeGenArg *cga);
  void binop_assign(Binop *bo,CodeGenArg *cga);
  void binop_arith(Binop *bo,CodeGenArg *cga);
  void binop_compare(Binop *bo,CodeGenArg *cga);

  // Global assembly data.
  AsmStore _globals;

  // Shared data for code generation.
  CData *_cdata;
};

#endif

