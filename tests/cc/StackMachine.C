//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
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

#include <iterator>
#include <sstream>
#include <stdexcept>
#include <iostream>

#include "StackMachine.h"
#include "Node.h"
#include "Types.h"
#include "AsmStore.h"

#define ASM(a,c) { \
  ostringstream s1,s2; \
  s1 << a; \
  s2 << c; \
  o(s1.str(),s2.str().c_str()); \
}

using namespace std;

IntVect StackMachine::_all_regs;
IntVect StackMachine::_caller_save_regs;
IntVect StackMachine::_callee_save_regs;
IntVect StackMachine::_byte_compat_regs;
Type *StackMachine::_default_type = 0;

// Converts the register to the equivalent byte-access version.  If the
// register isn't byte-compatible (i.e., isn't %eax, %ebx, %ecx,
// or %edx), then an exception is raised.
void Operand::setlo()
{
  // Ignore if not a register.
  if (is_reg()) {
    if (_value >= eax && _value <= edx) {
      _flags |= Byte;
    } else {
      Error1("Register " << _value << " is not byte-compatible.");
    }
  }
}

ostream &operator<<(ostream &o,Operand op)
{
  if (op.is_reg()) {
    if (op.is_bytereg()) {
      switch (op.value()) {
      case Operand::al:
        o << "%al";
        break;
      case Operand::bl:
        o << "%bl";
        break;
      case Operand::cl:
        o << "%cl";
        break;
      case Operand::dl:
        o << "%dl";
        break;
      default:
        assert(0);
      }
    } else {
      switch (op.value()) {
      case Operand::eax:
        o << "%eax";
        break;
      case Operand::ebx:
        o << "%ebx";
        break;
      case Operand::ecx:
        o << "%ecx";
        break;
      case Operand::edx:
        o << "%edx";
        break;
      case Operand::esi:
        o << "%esi";
        break;
      case Operand::edi:
        o << "%edi";
        break;
      default:
        assert(0);
      }
    }
  } else if (op.is_mem()) {
    o << op.value() << "(%ebp)";
  } else {
    o << "$" << op.value();
  }
  return o;
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

void printRegs(const IntVect &iv)
{
  ostream_iterator<int> i(cerr," ");
  copy(iv.begin(),iv.end(),i);
}

bool StackMachine::staticInits()
{
  _all_regs.push_back(Operand::ebx);
  _all_regs.push_back(Operand::esi);
  _all_regs.push_back(Operand::edi);
  _all_regs.push_back(Operand::eax);
  _all_regs.push_back(Operand::ecx);
  _all_regs.push_back(Operand::edx);

  _caller_save_regs.push_back(Operand::eax);
  _caller_save_regs.push_back(Operand::ecx);
  _caller_save_regs.push_back(Operand::edx);

  _callee_save_regs.push_back(Operand::ebx);
  _callee_save_regs.push_back(Operand::esi);
  _callee_save_regs.push_back(Operand::edi);

  _byte_compat_regs.push_back(Operand::eax);
  _byte_compat_regs.push_back(Operand::ebx);
  _byte_compat_regs.push_back(Operand::ecx);
  _byte_compat_regs.push_back(Operand::edx);

  _default_type = new BaseType(BaseType::Int);

  return true;
}

StackMachine::StackMachine(AsmStore &code,int base_fp) :
  _next_temp(base_fp - WordSize),
  _code(code)
{
  static bool dummy = staticInits();
  _regs_free = _all_regs;
}

void StackMachine::o(const string &a,const char *c)
{
  _code.o(a,c);
}

// Saves the caller-save registers, which should be done
// before the current function makes a function call, so that
// the registers don't get corrupted by the called function.
//
// Normally, this is done by pushing the caller-save registers
// onto the stack just before the function call is made and
// popping them off afterwards; however, due to the workings of
// this particular stack machine it's much easier to just move
// the contents of the caller-save registers, if they are
// currently being used, into temporary variables.
void StackMachine::save_caller_saves()
{
  IntVect tmp(1);
  for (IntVect::const_iterator i = _caller_save_regs.begin();
       i != _caller_save_regs.end(); ++i) {
    if (!contains(_regs_free,*i)) {
      tmp[0] = *i;
      copy_reg_to_temp(tmp,"Save caller-save register to temp.");
      _regs_free.push_back(*i);
    }
  }
}

// Emits code that pushes the callee-save registers
// used by the stack machine onto the process' stack.
void StackMachine::save_callee_saves(AsmStore &code)
{
  for (IntVect::iterator i = _callee_save_regs_used.begin();
       i != _callee_save_regs_used.end(); ++i) {
    ostringstream ss;
    ss << "  pushl " << Operand(*i);
    code.o(ss.str(),"Save callee-save registers.");
  }
}

// Emits code that pops the callee-save registers used by
// the stack machine off the process' stack.  
void StackMachine::load_callee_saves()
{
  for (IntVect::iterator i = _callee_save_regs_used.begin();
       i != _callee_save_regs_used.end(); ++i) {
    ASM("  popl " << Operand(*i),"Restore callee-save registers.");
  }  
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
  int reg = Operand::None;
  int index = 0;
  for (Stack::iterator i = _stack.begin(); i != _stack.end(); ++i) {
    if (i->is_reg() && contains(valid_regs,i->value())) {
      reg = i->value();
      break;
    }
    ++index;
  }
  if (reg == Operand::None) {
    throw runtime_error("No free registers inside OR outside of stack.");
  }

  // Emit code to copy the register to the memory location.
  if (!comment_str) {
    comment_str = "Stack machine:  copy register to temp.";
  }
  ASM("  movl " << Operand(reg) << ", " << Operand(mem,Operand::Mem),comment_str);
  // Modify element's stack machine position to reference its new location.
  _stack[index]._op = Operand(mem,Operand::Mem);
  return reg;
}

// Returns a free register that is in the valid_regs list.  If
// no registers are available, the most least-recently used
// eligible one is freed (by moving its contents to a temporary
// variable) and returned.
int StackMachine::get_free_reg(const IntVect &valid_regs,int preferred_reg)
{
  if (!_regs_free.empty()) {
    int reg = Operand::None;
    if (preferred_reg != Operand::None && contains(_regs_free,preferred_reg)) {
      reg = preferred_reg;
    } else {
      // We iterate from the back, since free registers are added to the back.  This lets
      // us re-use the preferred set (caller-save).
      for (IntVect::reverse_iterator i = _regs_free.rbegin(); i != _regs_free.rend(); ++i) {
        if (contains(valid_regs,*i)) {
          reg = *i;
          break;
        }
      }
    }
    if (reg != Operand::None) {
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
  BaseType::BT t = intType(type);
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
Operand StackMachine::push(Type *type,int preferred_reg,const IntVect *valid_regs)
{
  if (!type) {
    type = _default_type;
  }
  _stack.push_back(Item());
  _stack.back()._type = type;
  if (!valid_regs) {
    valid_regs = &get_type_valid_regs(type);
  }
  Operand reg = get_free_reg(*valid_regs,preferred_reg);
  _stack.back()._op = reg;
  return reg;
}

// Attempts to coerce the element in the current register
// from the given type to the given type.
Operand StackMachine::coerce_type(Operand curr_reg,Type *from,Type *to)
{
  if (typeid(from) == typeid(to)) {
    return curr_reg;
  }
  BaseType::BT from_t = intType(from);
  BaseType::BT to_t = intType(to);
  if (from_t == BaseType::Char) {
    if (to_t == BaseType::Int) {
      return curr_reg;
    }
  } else if (from_t == BaseType::Int) {
    if (to_t == BaseType::Char) {
      Operand cr_lo(curr_reg);
      cr_lo.setlo();
      ASM("  movzbl " << cr_lo << ", " << curr_reg,
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
Operand StackMachine::pop(Type *type,const IntVect *valid_regs)
{
  Type *prev_type = _stack.back()._type;
  if (type) {
    if (!valid_regs) {
      valid_regs = &get_type_valid_regs(type);
    }
    return coerce_type(pop_internal(*valid_regs),prev_type,type);
  } else {
    return pop_internal(_all_regs);
  }
}

// Pops the top element of the stack into a free register
// that is also in valid_regs and returns the register name.  If
// no registers are free, the least recently used one is first
// copied into a temporary variable and then used.
Operand StackMachine::pop_internal(const IntVect &valid_regs)
{
  Operand last = _stack.back()._op;
  _stack.pop_back();

  // If the top of the stack is a register, just return
  // the name of the register and add the register to our
  // free register list.
  if (last.is_reg() && contains(valid_regs,last.value())) {
    _regs_almost_free.push_back(last.value());
    return last;
  }

  // Otherwise, copy the temp variable at the top of stack
  // into a free register, possibly requiring us to spill the
  // current contents of the memory register into another temp
  // register.
  Operand reg(get_free_reg(valid_regs));
  ASM("  movl " << last << ", " << reg,
      "Stack machine:  copy temp to register.");

  // If our location was a register but not in valid_regs,
  // make the register free for use.
  if (last.is_reg()) {
    _regs_free.push_back(last.value());
  }

  _regs_almost_free.push_back(reg.value());

  return reg;
}

// Returns the top element of the stack, but doesn't pop
// it.  Note that this is not guaranteed to be a register; it
// could be a memory location!
Operand StackMachine::peek()
{
  assert(!empty());
  return _stack.back()._op;
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

// Forces a type change of the top element of the stack.
void StackMachine::force_type_change(Type *type)
{
  assert(!empty());
  _stack.back()._type = type;
}
