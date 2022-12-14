//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//
//
// Don't include this directly- include CLexer.h to wrap this.
//
// The tokenizer class.  This is a re-entrant C class
// that does our tokenizing.  It reads from a file by
// mmaping it.
//
// Note:  To use this header, you must include it like this:
//         #undef yyFlexLexer
//         #define yyFlexLexer cFlexLexer
//         #include "FlexLexer.h"
//         #include "CLexer.h"

#ifndef _CFLEXLEXER_H_
#define _CFLEXLEXER_H_

#include <string>

struct TokChan;
struct Tokens;

class CLexer : public cFlexLexer {

public:

  CLexer(TokChan &);
  CLexer(TokChan &,const char *filename);
  ~CLexer();

  void reset(const char *filename);

  const char *filename() const { return _filename; };
  unsigned linenumber() const { return _linenumber; };

  // Main parsing routine.
  int yylex();

  // Sends an all-done signal on the channel.
  void send_alldone();

protected:
  // Send a token and linenumber.
  void send(int tk);
  // Sends an identifier.
  void send_ident();
  // Sends an integer constant.
  void send_int();
  // Sends a floating-point constant.
  void send_float();
  // Send a character constant.
  void send_char();
  // Sends a string literal.  Currently this does not do 
  // any character translation- this needs to be fixed in the future.
  void send_string();

  const char *yystr();

  virtual int LexerInput( char* buf, int max_size );

private:
  void tksetup(Tokens &tk,int t);

  int convert_char(const char *str,int len);
  void closefile();

  const char *_filename;
  unsigned    _linenumber;

  TokChan    &_chan;

  int         _fd;
  const char *_base;
  const char *_prevpos;
  const char *_srcbase;
  const char *_bufbase;
  unsigned    _size;

};
 

#endif
