//
// The symbol table.  This works as follows:  All symbols are interned
// within a single hash and are given unique integer keys (a symbol 
// counter value).  The symbol table itself is a vector of linked lists,
// where the first element in the list is the current item in scope.
// To look up an item, the string is hashed, and an index is returned (if
// it does not exist, then we're done- it's not in the symbol table).  We
// then use this index to see if there is an entry in the vector.  If so,
// we return the front item.
//
// Each symbol table has a level count which is always one greater than its
// parent (0 for the root).  Each symbol stores a level value- if a symbol is
// added and it already exists with the same level value, an error is reported.
// This allows us to detect duplicate symbols.
//
// Each new scope allocates a new vector and copies the pointers from the
// parent scope (shallow copy).  When new symbols are added, the current
// vector is updated and the added linked-list node is inserted at the
// front.  This means that the symbol table is persistent:  Adding a new
// scope does not affect the symbol tables of any other scopes.
//

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include <vector>
# include <ext/hash_map>
using namespace __gnu_cxx;

#include "gc_cpp.h"
#include "String.h"

struct SymHash : public gc, public hash_map<String,int,StringHash,equal_to<String> > {
  SymHash() : _nextsym(0) {};

  int get_symbol(String x);
  int symbol_index(String x) const;
private:
  int _nextsym;
};

// This represents a single scope of a symbol table.  A parent (constructor
// argument) represents a parent scope.  Note that we store a pointer to the
// parent scope, but it's not used for finding a symbol (as described above).
// Also, each table has a list of its children so that it's possible to traverse
// the complete tree from a root pointer.
class SymTab : public gc {
public:
  struct Symbol {
    int     _level; // Level at which it was added.
    Node   *_n;     // The item that the symbol represents.
    Symbol *_next;  // Next element in chain.

    Symbol(int l,Node *n) : _level(l), _n(n), _next(0) {};
    Symbol(int l,Node *n,Symbol *next) : _level(l), _n(n), _next(next) {};
  };
  typedef vector<Symbol *,traceable_allocator<Symbol *> > Symbols;
  typedef vector<SymTab *,traceable_allocator<SymTab *> > Children;

  typedef Symbols::const_iterator const_iterator;
  typedef Children::const_iterator const_children_iterator;

  // Create a root-level symbol table.  This creates a new symbol hash.
  SymTab();
  // Create a symbol table, inheriting symbols and hash from an outer scope.
  SymTab(SymTab *parent);

  // Add a symbol.  If raise is true, throws runtime_error if a duplicate is
  // found.  If false, returns false.
  // This throws an exception if the name is not an identifier.
  bool add(String name,Node *value,bool raise = true);

  // Returns true if the symbol occurs at the same scope
  // as this symbol table.
  bool is_local(Symbol *s) const { assert(s); return _level == s->_level; };

  // Returns a pointer to the node, or 0 if not found.
  Node *find(String s) const;

  // Get parent scope.
  SymTab *parent() const { return _parent; };

  const_iterator begin() const { return _symbols.begin(); };
  const_iterator end() const { return _symbols.end(); };

  const_children_iterator childbegin() const { return _children.begin(); };
  const_children_iterator childend() const { return _children.end(); };

  void print(std::ostream &o) const;
private:
  void add_child(SymTab *);
  void adjust_size(unsigned);
  friend std::ostream &operator<<(std::ostream &,const SymTab *);

  Symbols  _symbols;  // Symbols, indexed by values stored in symhash.
  int      _level;    // Current level of this table (0 is root).
  SymTab  *_parent;   // Parent scope.
  Children _children; // Pointers to child scopes.
  SymHash *_hash;     // Symbol hash.
};

#endif

