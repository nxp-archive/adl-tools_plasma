//     This class attempts to abstract the x86 registers into a stack
//     machine.  Calling push() gives you a register that isn't currently
//     in use by the stack machine, pop() gives you a register with the
//     value of the most recently pushed element.

//     Through this method the stack machine can be used to compute
//     values the same way a reverse polish notation (RPN) calculator
//     does.

//     When push() and pop() are called, it may be the case that no
//     registers are currently available; if this happens, the least
//     recently used register is 'spilled' into a temporary local
//     variable on the process' stack and freed for use.  Note that the
//     process' stack is not to be confused with this stack machine
//     abstraction--the two are completely different entities.

//     Currently, push() and pop() also implement a little bit of
//     implicit type conversion, so they take as parameters a cparse.Type
//     object; currently conversion is done between char and int types,
//     so depending on the pushed and popped types, some type conversion
//     assembly code may be generated.

//     Finally, an additional method, done(), should be called whenever
//     the stack machine is done popping values for the current
//     operation.  This is because when pop is called, the returned
//     register is not immediately made 'free' for another call to pop or
//     push.  If this were the case, then the following situation could
//     occur:

//              rightOp.calc()      # calc val of right op, put on stack
//              leftOp.calc()       # calc val of left op, put on stack
//              l = leftOp.pop()    # pop left val from stack
//              r = rightOp.pop()   # pop right val from stack
//              output('addl %s, %s' % (r, l))

//     The problem with this approach is that we don't know how many
//     registers will be used by leftOp's calc() method--it may use all
//     the remaining registers, in which case the value that rightOp's
//     calc() method put on the stack is no longer stored in a register.
//     If leftOp.pop() returned register %eax and immediately marked the
//     %eax register as being 'free for use', then the call to
//     rightOp.pop() could very well generate code that moves rightOp's
//     value from a temporary variable into %eax, thereby overwriting
//     leftOp's value!

//     So, instead, the pop() method places the %eax register (in this
//     example) into an internal list of 'almost free' registers;
//     registers that have just been returned by pop() but shouldn't be
//     used by the stack machine until a call to done() is made.  The
//     done() method simply moves the registers in the 'almost free' list
//     over to the 'free' list.

#include <sstream>
#include <stdexcept>

#include "StackMachine.h"
#include "Node.h"

using namespace std;

// Size of the 'int' type.
const int IntSize = 4;

// Size of the 'char' type.
const int CharSize = 1;

// The machine's word size.  Note that making this different
// from INT_SIZE may cause serious problems.
const int WordSize = 4;

// This is a strange multiplier that needs to be used in the allocation
// of global variables for the GNU Assembler.  Not sure exactly what it
// represents.
const int WirdMultiplier = 4;

static bool stackMachineInit = StackMachine::staticInits();

IntVect StackMachine::_all_regs;
IntVect StackMachine::_caller_save_regs;
IntVect StackMachine::_callee_save_regs;
IntVect StackMachine::_byte_compat_regs;
Type *StackMachine::_default_type = 0;

bool StackMachine::staticInits()
{
  for (int i = FirstReg; i != LastReg; ++i) {
    _all_regs.push_back((Regs)i);
  }

  _caller_save_regs.push_back(eax);
  _caller_save_regs.push_back(ecx);
  _caller_save_regs.push_back(edx);

  _callee_save_regs.push_back(ebx);
  _callee_save_regs.push_back(esi);
  _callee_save_regs.push_back(edi);

  _byte_compat_regs.push_back(eax);
  _byte_compat_regs.push_back(ebx);
  _byte_compat_regs.push_back(ecx);
  _byte_compat_regs.push_back(edx);

  _default_type = new BaseType(BaseType::Int);

  return true;
}

StackMachine::StackMachine(CodeGen *parent,int base_fp) :
  _regs_free(_all_regs),
  _next_temp(base_fp - WordSize),
  _parent(parent)
{
}

const char *StackMachine::regname(int r) const
{
  switch (r) {
  case eax:
    return "%eax";
  case ebx:
    return "%ebx";
  case ecx:
    return "%ecx";
  case edx:
    return "%edx";
  case esi:
    return "%esi";
  case edi:
    return "%edi";
  default:
    Error1("Unknown register " << r);
  }
}

string StackMachine::memname(int mem) const
{
  ostringstream ss;
  ss << mem << "(%ebp)";
  return ss.str();
}

string StackMachine::itemname(const Item &item) const
{
  if (item.isreg()) {
    return regname(item._value);
  } else {
    return memname(item._value);
  }
}

void StackMachine::o(const string &a,const char *c)
{
  //  _parent->o(a,c);
}

// Emits code that pushes the callee-save registers
// used by the stack machine onto the process' stack.
void StackMachine::save_callee_saves()
{
  for (IntVect::iterator i = _callee_save_regs_used.begin();
       i != _callee_save_regs_used.end(); ++i) {
    ASM("  pushl " << regname(*i),"Save callee-save registers.");
  }
}

// Emits code that pops the callee-save registers used by
// the stack machine off the process' stack.  
void StackMachine::load_callee_saves()
{
  for (IntVect::iterator i = _callee_save_regs_used.begin();
       i != _callee_save_regs_used.end(); ++i) {
    ASM("  popl " << regname(*i),"Restore callee-save registers.");
  }  
}

template<class T,typename C>
bool contains(const T &t,C c)
{
  return (find(t.begin(),t.end(),c) != t.end());
}

template<class T,typename C>
void remove_item(T &t,C c)
{
  typename T::iterator e = remove(t.begin(),t.end(),c);
  t.erase(e,t.end());
}

// Copy the least recently used register on the stack into a
// temporary variable.  The register must be in the valid_regs
// list.
int StackMachine::copy_reg_to_temp(const IntVect &valid_regs,const char *comment_str)
{
  // If no free temp variables exist, create a new one.
  if (_mem_free.empty()) {
    _mem_free.push_back(_next_temp);
    _next_temp -= WordSize;
  }

  // Get an unused temp var.
  int mem = _mem_free.back();
  _mem_free.pop_back();

  // Find the least recently used register on the stack.
  int reg = None;
  int index = 0;
  for (Stack::iterator i = _stack.begin(); i != _stack.end(); ++i) {
    if (i->isreg() && contains(valid_regs,i->_value)) {
      reg = i->_value;
      break;
    }
    ++index;
  }
  if (reg == None) {
    throw runtime_error("No free registers inside OR outside of stack.");
  }

  // Emit code to copy the register to the memory location.
  if (!comment_str) {
    comment_str = "Stack machine:  copy register to temp.";
  }
  ASM("  movl " << regname(reg) << ", " << memname(mem),comment_str);
  // Modify element's stack machine position to reference its new location.
  _stack[index]._value = mem;
  _stack[index]._stype = Mem;
  return reg;
}

// Returns a free register that is in the valid_regs list.  If
// no registers are available, the most least-recently used
// eligible one is freed (by moving its contents to a temporary
// variable) and returned.
int StackMachine::get_free_reg(const IntVect &valid_regs,int preferred_reg)
{
  if (!_regs_free.empty()) {
    int reg = None;
    if (preferred_reg != None && contains(_regs_free,preferred_reg)) {
      reg = preferred_reg;
    } else {
      for (IntVect::iterator i = _regs_free.begin(); i != _regs_free.end(); ++i) {
        if (contains(valid_regs,*i)) {
          reg = *i;
          break;
        }
      }
    }
    if (reg != None) {
      remove_item(_regs_free,reg);
      // If this register is a callee-save register that
      // we haven't used before, add it to our list of
      // used callee-save registers.
      if (!contains(_callee_save_regs_used,reg) && contains(_callee_save_regs,reg)) {
        _callee_save_regs_used.push_back(reg);
      }
      return reg;
    }
  }
  // No register was free, so move least recently used to temp location.
  return copy_reg_to_temp(valid_regs);
}

// Returns the valid registers that an element of the given
// type can occupy.  For instance, 8-bit chars should only be
// placed in %eax/%ebx/%ecx/%edx because these are the only
// registers with low-order byte sub-registers
// (%al/%bl/%cl/%dl).
const IntVect &StackMachine::get_type_valid_regs(Type *type) const
{
  BaseType::Type t = intType(type);
  if (t == BaseType::Char) {
    return _byte_compat_regs;
  } else if (t == BaseType::Int || isPtrType(type)) {
    return _all_regs;
  } else {
    Error1("No compatible register available for type " << type);
  }
}

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
int StackMachine::push(Type *type,int preferred_reg,const IntVect *valid_regs)
{
  if (!type) {
    type = _default_type;
  }
  _stack.push_back(Item());
  _stack.back()._type = type;
  if (!valid_regs) {
    valid_regs = &get_type_valid_regs(type);
  }
  int reg = get_free_reg(*valid_regs,preferred_reg);
  _stack.back()._value = reg;
  return reg;
}

// Attempts to coerce the element in the current register
// from the given type to the given type.
int StackMachine::coerce_type(int curr_reg,Type *from,Type *to)
{
  if (typeid(from) == typeid(to)) {
    return curr_reg;
  }
  BaseType::Type from_t = intType(from);
  BaseType::Type to_t = intType(to);
  if (from_t == BaseType::Char) {
    if (to_t == BaseType::Int) {
      return curr_reg;
    }
  } else if (from_t == BaseType::Int) {
    if (to_t == BaseType::Char) {
      ASM("  movzbl " << lo(curr_reg) << ", " << curr_reg,
          "Implicit cast: " << from << " -> " << to);
      return curr_reg;
    }
  }
  assert(0);
}

// Pops the top element off the stack machine's stack, coerces
// it to the given type if necessary, and returns a register in
// which the element's value now resides.
//
// If no type is specified, pop() returns the value of the
// element as-is.
int StackMachine::pop(Type *type,const IntVect *valid_regs)
{
  Type *prev_type = _stack.back()._type;
  if (!type) {
    if (!valid_regs) {
      valid_regs = &get_type_valid_regs(type);
    }
    int reg = pop_internal(*valid_regs);
    return coerce_type(reg,prev_type,type);
  } else {
    return pop_internal(_all_regs);
  }
}

// Pops the top element of the stack into a free register
// that is also in valid_regs and returns the register name.  If
// no registers are free, the least recently used one is first
// copied into a temporary variable and then used.
int StackMachine::pop_internal(const IntVect &valid_regs)
{
  Item last = _stack.back();
  _stack.pop_back();

  // If the top of the stack is a register, just return
  // the name of the register and add the register to our
  // free register list.
  if (last.isreg() && contains(valid_regs,last._value)) {
    _regs_almost_free.push_back(last._value);
    return last._value;
  }

  // Otherwise, copy the temp variable at the top of stack
  // into a free register, possibly requiring us to spill the
  // current contents of the memory register into another temp
  // register.
  int reg = get_free_reg(valid_regs);
  ASM("  movl " << itemname(last) << ", " << regname(reg),
      "Stack machine:  copy temp to register.");

  // If our location was a register but not in valid_regs,
  // make the register free for use.
  if (last.isreg()) {
    _regs_free.push_back(last._value);
  }
  return reg;
}

// Returns the top element of the stack, but doesn't pop
// it.  Note that this is not guaranteed to be a register; it
// could be a memory location!
StackMachine::Item &StackMachine::peek()
{
  return _stack.back();
}

// Frees all registers that are marked as being in
// intermediate use (i.e., have been pop()'d).
void StackMachine::done()
{
  _regs_free.insert(_regs_free.end(),_regs_almost_free.begin(),_regs_almost_free.end());
  _regs_almost_free.clear();
}

// Returns the maximum point in the process' stack, relative
// to the current function's frame pointer, that the stack
// machine is using for temporary variables.
int StackMachine::get_max_fp() const
{
  return _next_temp + WordSize;
}

// Returns the low-order byte of the given register.  If the
// register isn't byte-compatible (i.e., isn't %eax, %ebx, %ecx,
// or %edx), then an exception is raised.
//
// Example: stack.lo('%eax') == '%al'.
const char *StackMachine::lo(int reg)
{
  switch (reg) {
  case eax:
    return "%al";
  case ebx:
    return "%bl";
  case ecx:
    return "%cl";
  case edx:
    return "%dl";
  default:
    Error1("Register " << reg << " is not byte-compatible.");
  } 
}

// Forces a type change of the top element of the stack.
void StackMachine::force_type_change(Type *type)
{
  assert(!is_empty());
  _stack.back()._type = type;
}
