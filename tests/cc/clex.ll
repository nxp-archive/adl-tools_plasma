%{

//
// Tokenizer for our C compiler.  The input is handled as
// a mmap'd file, so we don't copy identifiers or strings- we
// use the String class, which stores a pointer and length.
//
// Unfortunately, flex modifies its buffer, so we have to copy
// data from the file to a buffer.  This could be an area for
// future optimization.  Still, it should be pretty fast- only a
// single copy, vs. copying from the file via fread, plus copying
// to a string.
//

#include <string.h>

#include "String.h"
#include "CFlexLexer.h"

#include "Tokens.h"
#include "cparse.h"

  using namespace std;

%}

/* This generates an interactive c++ tokenizer which
	doesn't call yywrap- when it reaches EOF, it assumes it's done.
*/

%option batch
%option c++
%option noyywrap
%option prefix="c"
%option yyclass="CLexer"

D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

%x comment

%%

[ \t]+                  /* do nothing */

   /* C++ style comments. */
"//".*\n                 _linenumber++;

  /* C style comments. */
"/*"                     BEGIN(comment);
<comment>[^*\n]*         
<comment>[^*\n]*\n       _linenumber++;
<comment>"*"+[^*/\n]*    
<comment>"*"+[^*/\n]*\n  _linenumber++;
<comment>"*/"            BEGIN(INITIAL);

"break"			{ send(BREAK); }
"char"			{ send(CHAR); }
"continue"		{ send(CONTINUE); }
"else"			{ send(ELSE); }
"extern"		{ send(EXTERN); }
"for"			{ send(FOR); }
"if"			{ send(IF); }
"int"			{ send(INT); }
"return"	    { send(RETURN); }
"static"		{ send(STATIC); }
"while"			{ send(WHILE); }

{L}({L}|{D})*	{ send_ident(); }

0[xX]{H}+{IS}?	{ send_int(); }
0{D}+{IS}?		{ send_int(); }
{D}+{IS}?		{ send_int(); }
 
{D}+{E}{FS}?		    { send_float(); }
{D}*"."{D}+({E})?{FS}?	{ send_float(); }
{D}+"."{D}*({E})?{FS}?	{ send_float(); }

L?\"(\\.|[^\\"])*\"	{ send_string(); }

\'(\\.|[^\\'])*\' { send_char(); }

"..."			{ send(ELLIPSIS); }
"+="			{ send(ADD_ASSIGN); }
"-="			{ send(SUB_ASSIGN); }
"<="			{ send(LESS_EQ); }
">="			{ send(GREATER_EQ); }
"=="			{ send(EQ); }
"!="			{ send(NOT_EQ); }

"="         { send(ASSIGN); }
";"			{ send(SEMICOLON); }
("{"|"<%")	{ send(LBRACE); }
("}"|"%>")	{ send(RBRACE); }
","			{ send(COMMA); }
"("			{ send(LPAREN); }
")"			{ send(RPAREN); }
("["|"<:")	{ send(LBRACKET); }
("]"|":>")	{ send(RBRACKET); }
"&"			{ send(AMPERSAND); }
"!"			{ send(EXCLAMATION); }
"-"			{ send(MINUS); }
"+"			{ send(PLUS); }
"*"			{ send(ASTERISK); }
"<"			{ send(LESS); }
">"			{ send(GREATER); }
"/"         { send(DIV); }
"%"         { send(MODULO); }

\n          { _linenumber++;  }

.			{ /* ignore bad characters */ }

<<EOF>>     { send(0); return 0; };

%%

//"

