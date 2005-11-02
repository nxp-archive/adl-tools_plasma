//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
//
// Simple class for storing assembly code.
// Rather than having an AST for assembly, we just
// have a list of strings that we print out at the
// end.
//

#ifndef _ASM_STORE_H_
#define _ASM_STORE_H_

#include <list>
#include <string>
#include <iosfwd>

class AsmStore {
public:
  AsmStore(bool print_comments = true);

  // Output a line of assembly.
  void o(const std::string &str,const char *comment = 0);
  // Output a comment, if comments are enabled.
  void c(const std::string &str,int indent_amt = 2);

  // Add code to front.
  void splice_front(AsmStore &);
  // Add code to back.
  void splice_back(AsmStore &);

  // Write data to the specified output stream.
  void write(std::ostream &) const;

private:
  typedef std::list<std::string> AList;

  bool  _print_comments;
  AList _asm;

};

#endif
