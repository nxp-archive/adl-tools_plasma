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

"auto"			{ send(AUTO); }
"break"			{ send(BREAK); }
"case"			{ send(CASE); }
"char"			{ send(CHAR); }
"const"			{ send(CONST); }
"continue"		{ send(CONTINUE); }
"default"		{ send(DEFAULT); }
"do"			{ send(DO); }
"double"		{ send(DOUBLE); }
"else"			{ send(ELSE); }
"enum"			{ send(ENUM); }
"extern"		{ send(EXTERN); }
"float"			{ send(FLOAT); }
"for"			{ send(FOR); }
"goto"			{ send(GOTO); }
"if"			{ send(IF); }
"int"			{ send(INT); }
"long"			{ send(LONG); }
"register"		{ send(REGISTER); }
"return"	    { send(RETURN); }
"short"			{ send(SHORT); }
"signed"		{ send(SIGNED); }
"sizeof"		{ send(SIZEOF); }
"static"		{ send(STATIC); }
"struct"		{ send(STRUCT); }
"switch"		{ send(SWITCH); }
"typedef"		{ send(TYPEDEF); }
"union"			{ send(UNION); }
"unsigned"		{ send(UNSIGNED); }
"void"			{ send(VOID); }
"volatile"		{ send(VOLATILE); }
"while"			{ send(WHILE); }

{L}({L}|{D})*	{ send_ident(); }

0[xX]{H}+{IS}?	{ send_int(); }
0{D}+{IS}?		{ send_int(); }
{D}+{IS}?		{ send_int(); }
 
{D}+{E}{FS}?		    { send_float(); }
{D}*"."{D}+({E})?{FS}?	{ send_float(); }
{D}+"."{D}*({E})?{FS}?	{ send_float(); }

L?\"(\\.|[^\\"])*\"	{ send_string(); }

"..."			{ send(ELLIPSIS); }
">>="			{ send(RIGHT_ASSIGN); }
"<<="			{ send(LEFT_ASSIGN); }
"+="			{ send(ADD_ASSIGN); }
"-="			{ send(SUB_ASSIGN); }
"*="			{ send(MUL_ASSIGN); }
"/="			{ send(DIV_ASSIGN); }
"%="			{ send(MOD_ASSIGN); }
"&="			{ send(AND_ASSIGN); }
"^="			{ send(XOR_ASSIGN); }
"|="			{ send(OR_ASSIGN); }
">>"			{ send(RIGHT_OP); }
"<<"			{ send(LEFT_OP); }
"++"			{ send(INC_OP); }
"--"			{ send(DEC_OP); }
"->"			{ send(PTR_OP); }
"&&"			{ send(AND_OP); }
"||"			{ send(OR_OP); }
"<="			{ send(LE_OP); }
">="			{ send(GE_OP); }
"=="			{ send(EQ_OP); }
"!="			{ send(NE_OP); }

";"			{ send(SEMICOLON); }
("{"|"<%")	{ send(LBRACE); }
("}"|"%>")	{ send(RBRACE); }
","			{ send(COMMA); }
":"			{ send(COLON); }
"="			{ send(EQUALS); }
"("			{ send(LPAREN); }
")"			{ send(RPAREN); }
("["|"<:")	{ send(LBRACKET); }
("]"|":>")	{ send(RBRACKET); }
"."			{ send(DOT); }
"&"			{ send(AND); }
"!"			{ send(NOT); }
"~"			{ send(COMPL); }
"-"			{ send(MINUS); }
"+"			{ send(PLUS); }
"*"			{ send(MULT); }
"/"			{ send(DIVIDE); }
"%"			{ send(MOD); }
"<"			{ send(LESS); }
">"			{ send(GREATER); }
"^"			{ send(XOR); }
"|"			{ send(OR); }
"?"			{ send(QUESTION); }

\n          { _linenumber++;  }

.			{ /* ignore bad characters */ }

<<EOF>>     { send(0); return 0; };

%%

//"

