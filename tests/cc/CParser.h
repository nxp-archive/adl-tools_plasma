//
// The main parser class.  This wraps the parser internals,
// which are generated by Lemon.
//

#ifndef _CPARSER_H_
#define _CPARSER_H_

#include <iosfwd>
#include <vector>

struct Tokens;
struct Node;

typedef std::vector<Node *> Nodes;

// Wrapper class.  The copy-constructor is shallow
// and there is no destructor b/c allocation is garbage collected.
class CParser {
public:
  CParser();
  CParser(const CParser &cp) : _parser(cp._parser) {}

  void parse(int t,Tokens tk);

  int linenumber() const { return _linenumber; };
  const char *filename() const { return _filename; };
  void setinfo(Node *) const;

  // Resets error status only.
  void reset();
  // Resets error status and clears all data structures.
  void reset_all();
  void seterror() { _error = true; };
  bool error() const { return _error; };

  // Stores a translation unit AST.
  void add_translation_unit(Node *);

  // Print all ASTs.
  void print_ast_list(std::ostream &);

private:
  void *_parser;  // The parser (opaque type generated by Lemon).

  const char *_filename;
  int         _linenumber;
  bool        _error;

  // All of the translation units that have been parsed.
  Nodes       _asts;
};

#endif
