//
// Symbol table implementation file- refer to header for a
// description of the data structure.
//

#include <sstream>
#include <stdexcept>

#include "Node.h"
#include "SymTab.h"

using namespace std;

// Default symbol table vector init size.
const int DefaultSize = 50;

// Return an index for a given symbol.  This inserts the name
// into the hash if it doesn't exist and updates _nextsym accordingly.
inline int SymHash::get_symbol(String x)
{
  pair<SymHash::iterator,bool> ip = insert(value_type(x,_nextsym));
  if (ip.second) {
    ++_nextsym;
  }
  return ip.first->second;
}

// This returns an index, or -1 if not found.
inline int SymHash::symbol_index(String x) const
{
  const_iterator i = find(x);
  return (i == end()) ? -1 : i->second;
}

// Create a root-level symbol table.
SymTab::SymTab() :
  _symbols(DefaultSize),
  _level(0),
  _parent(0),
  _hash(new SymHash)
{
}

// Create a symbol table, inheriting symbols from an outer scope.
SymTab::SymTab(SymTab *parent) :
  _symbols(parent->_symbols),
  _level(parent->_level+1),
  _parent(parent),
  _hash(parent->_hash)
{
}

// Resize symbol table array if necessary.
inline void SymTab::adjust_size(unsigned s)
{
  if (_symbols.size() < s) {
    _symbols.resize(s);
  }
}

// Add a symbol.  The basic process is:  Intern the
// symbol, adjust the size of the array, if necessary,
// then look at the index location:  If empty, we add it.
// If not, check for a duplicate symbol (same level).  If okay,
// add to head of chain.
bool SymTab::add(String name,Node *value,bool raise)
{
  int index = _hash->get_symbol(name);
  adjust_size(index);

  if (Symbol *first = _symbols[index]) {
    // Entry exists.  Do we have a duplicate?
    if (first->_level == _level) {
      // We have a duplicate.
      if (raise) {
        Error(value,"Duplicate identifier.  Original is at " << first->_n->filename() << ":" << first->_n->linenumber() << ".");
      } else {
        return false;
      }
    }
    // Okay- add to front.
    Symbol *s = new Symbol(_level,value,first);
    _symbols[index] = s;
  } else {
    // Entry is empty- add symbol.
    _symbols[index] = new Symbol(_level,value);
  }
  return true;
}

// Try to find a symbol.  Basic process is:  If the symbol
// doesn't exist in the hash, then it's not there.  Else,
// if the symbol table array is too small, then it's not there.
// Else, if the array location is empty, then it's not there.
// Else, it's there.
Node *SymTab::find(String name) const
{
  int index = _hash->symbol_index(name);
  if (index < 0) {
    return 0;
  }
  if ((int)_symbols.size() <= index) {
    return 0;
  }
  if (!_symbols[index]) {
    return 0;
  }
  return _symbols[index]->_n;
}

std::ostream &operator<<(std::ostream &o,const SymTab *x)
{
  if (!x) {
    o << indent << "<empty>";
  } else {
    x->print(o);
  }
  return o;
}

void SymTab::print(std::ostream &o) const
{
  // We print out the symbols in a manner that makes sense to the user,
  // so we iterate over each level, printing only symbols of that level.
  o << indent << "Level " << _level << incrindent;
  for (Symbols::const_iterator i = _symbols.begin(); i != _symbols.end(); ++i) {
    if (*i) {
      Symbol &s = *(*i);
      if (s._level == _level) {
        o << indent << s._n->name() << ":  " << s._n->type();
      }
    }
  }
  o << decrindent;
  if (_parent) {
    _parent->print(o);
  }
}
