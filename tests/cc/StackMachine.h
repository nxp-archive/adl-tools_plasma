//
// This class abstracts x86 registers into a stack
// machine for use by the code generator.
//

#ifndef _STACK_MACHINE_H_
#define _STACK_MACHINE_H_

#include <vector>
#include <string>

class Type;
class CodeGen;

#define ASM(a,c) { \
  ostringstream s1,s2; \
  s1 << a; \
  s2 << c; \
  o(s1.str(),s2.str().c_str()); \
}

typedef std::vector<int> IntVect;

class StackMachine {
public:
  enum Regs {
    None = -1,
    FirstReg = 0,
    eax = 0,
    ebx,
    ecx,
    edx,
    esi,
    edi,
    LastReg,
  };

  enum SType { Reg, Mem };
  struct Item {
    int    _value;
    SType  _stype;
    Type  *_type;
    
    Item() : _value(None), _stype(Reg), _type(0) {};

    bool isreg() const { return _stype == Reg; };
    bool ismem() const { return _stype == Mem; };
  };

  StackMachine(CodeGen *parent,int base_fp);

  // Assembler name for a memory location, relative to stack frame.
  std::string memname(int mem) const;
  // Return a string name for a given register.
  const char *regname(int reg) const;
  // Assembler name for either a memory location or a register.
  std::string itemname(const Item &item) const;

  // Output an assembly string to the parent
  // code generator.
  void o(const std::string &str,const char *comment = 0);
  // Emits code that pushes the callee-save registers
  // used by the stack machine onto the process' stack.
  void save_callee_saves();
  // Emits code that pops the callee-save registers used by
  // the stack machine off the process' stack.  
  void load_callee_saves();
  // Finds a free eligible register (or frees one if all are
  // being used) and returns it, pushing the register onto the
  // stack machine's stack.
  //
  // This method associates the stack entry with the given Type
  // object; if none is supplied, then an 'int' type is used
  // by default.
  //
  // If preferred_reg is passed, this function will try its
  // best to return preferred_reg, if it's available.
  int push(Type *type = 0,int preferred_reg = None,const IntVect *valid_regs = 0);
  // Pops the top element off the stack machine's stack, coerces
  // it to the given type if necessary, and returns a register in
  // which the element's value now resides.
  //
  // If no type is specified, pop() returns the value of the
  // element as-is.
  int pop(Type *type = 0,const IntVect *valid_regs = 0);
  // Returns the top element of the stack, but doesn't pop
  // it.  Note that this is not guaranteed to be a register; it
  // could be a memory location!
  Item &StackMachine::peek();
  // Returns true if the stack machine is empty.
  bool is_empty() const { return _stack.empty(); };
  // Frees all registers that are marked as being in
  // intermediate use (i.e., have been pop()'d).
  void done();
  // Returns the maximum point in the process' stack, relative
  // to the current function's frame pointer, that the stack
  // machine is using for temporary variables.
  int get_max_fp() const;
  // Returns the low-order byte of the given register.  If the
  // register isn't byte-compatible (i.e., isn't %eax, %ebx, %ecx,
  // or %edx), then an exception is raised.
  //
  // Example: stack.lo('%eax') == '%al'.
  const char *lo(int reg);
  // Forces a type change of the top element of the stack.
  void force_type_change(Type *type);

  static bool staticInits();
private:
  int get_free_reg(const IntVect &valid_regs,int preferred_reg = None);
  const IntVect &get_type_valid_regs(Type *t) const;
  int copy_reg_to_temp(const IntVect &valid_regs,const char *comment_str = 0);
  int coerce_type(int curr_reg,Type *from,Type *to);
  int pop_internal(const IntVect &valid_regs);

  typedef std::vector<Item> Stack;

  // All registers in the system, in a vector so that
  // we can easily make copies.
  static IntVect _all_regs;
  // List of the caller-save registers.
  static IntVect _caller_save_regs;
  // List of the callee-save registers.
  static IntVect _callee_save_regs;
  // List of registers that allow low-order byte access.
  static IntVect _byte_compat_regs;
  // Default type for any object pushed onto the stack.
  static Type *_default_type;
  // Available registers.
  IntVect _regs_free;
  // Registers that aren't in use, except that they
  // have a result value.
  IntVect _regs_almost_free;
  // Temporary variable memory locations that are unused.
  IntVect _mem_free;
  // The stack machine's stack.  Element at the back is
  // the top of the stack.
  Stack _stack;
  // Location of next temporary memory location to be
  // used for temporary variables, relative to the current
  // function's frame pointer.
  int _next_temp;
  // Parent code generator.
  CodeGen *_parent;
  // A list of the callee-save registers that have been used
  // so far by this function.  Once processing is finished,
  // these registers will be pushed onto the process' stack
  // at the beginning of the function and popped off just
  // before the function terminates.
  IntVect _callee_save_regs_used;
};

#endif
