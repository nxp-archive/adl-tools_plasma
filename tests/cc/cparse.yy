//
// C subset grammar.  This uss the lemon parser generator
// http://www.hwaci.com/sw/lemon/lemon.html to create the parser.
//

%include {   

#include <iostream>  

#include "Tokens.h"
#include "cparse.h"
#include "CParser.h"

  using namespace std;

}

%nonassoc LOWER_THAN_ELSE.
%nonassoc ELSE.

%start_symbol translation_unit

%extra_argument { CParser *cp }

%token_type {Tokens* }
%type expr {int}

%parse_accept {
  cerr << "Parsing complete!" << endl;
}

%syntax_error {  
  cerr << cp->filename() << ":" << cp->linenumber() << ":  syntax error." << endl;  
  cp->seterror();
}   

%parse_failure {
  cerr << "Parse failed!" << endl;
}

translation_unit ::= extern_decl_list.

extern_decl_list ::= extern_decl.
extern_decl_list ::= extern_decl_list extern_decl.

extern_decl ::= function_definition.
extern_decl ::= declaration.

function_definition ::= type_specifier declarator compound_statement.
function_definition ::= STATIC type_specifier declarator compound_statement.

declaration ::= type_specifier declarator SEMICOLON.
declaration ::= EXTERN type_specifier declarator SEMICOLON.

declaration_list_opt ::= .
declaration_list_opt ::= declaration_list.

declaration_list ::= declaration.
declaration_list ::= declaration_list declaration.

type_specifier ::= INT.
type_specifier ::= CHAR.

declarator ::= direct_declarator.
declarator ::= ASTERISK declarator.

direct_declarator ::= IDENTIFIER.
direct_declarator ::= direct_declarator LPAREN parameter_type_list RPAREN.
direct_declarator ::= direct_declarator LPAREN RPAREN.

parameter_type_list ::= parameter_list.
parameter_type_list ::= parameter_list COMMA ELLIPSIS.

parameter_list ::= parameter_declaration.
parameter_list ::= parameter_list COMMA parameter_declaration.

parameter_declaration ::= type_specifier declarator.

compound_statement ::= LBRACE declaration_list_opt statement_list RBRACE.
compound_statement ::= LBRACE declaration_list_opt RBRACE.

expression_statement ::= expression SEMICOLON.
expression_statement ::= error SEMICOLON. { cerr << "Reached the error token." << endl; }

expression ::= equality_expression.
expression ::= equality_expression ASSIGN expression.
expression ::= equality_expression ADD_ASSIGN expression.
expression ::= equality_expression SUB_ASSIGN expression.

equality_expression ::= relational_expression.
equality_expression ::= equality_expression EQ relational_expression.
equality_expression ::= equality_expression NOT_EQ relational_expression.

relational_expression ::= additive_expression.
relational_expression ::= relational_expression LESS additive_expression.
relational_expression ::= relational_expression GREATER additive_expression.
relational_expression ::= relational_expression LESS_EQ additive_expression.
relational_expression ::= relational_expression GREATER_EQ additive_expression.

postfix_expression ::= primary_expression.
postfix_expression ::= postfix_expression LPAREN argument_expression_list RPAREN.
postfix_expression ::= postfix_expression LPAREN RPAREN.
postfix_expression ::= postfix_expression LBRACKET expression RBRACKET.

argument_expression_list ::= expression.
argument_expression_list ::= argument_expression_list COMMA expression.

unary_expression ::= postfix_expression.
unary_expression ::= MINUS unary_expression.
unary_expression ::= PLUS unary_expression.
unary_expression ::= EXCLAMATION unary_expression.
unary_expression ::= ASTERISK unary_expression.
unary_expression ::= AMPERSAND unary_expression.

mult_expression ::= unary_expression.
mult_expression ::= mult_expression ASTERISK unary_expression.
mult_expression ::= mult_expression DIV unary_expression.
mult_expression ::= mult_expression MODULO unary_expression.

additive_expression ::= mult_expression.
additive_expression ::= additive_expression PLUS mult_expression.
additive_expression ::=  additive_expression MINUS mult_expression.

primary_expression ::= IDENTIFIER.
primary_expression ::= INTCONST.
primary_expression ::= FPCONST.
primary_expression ::= CHARCONST.
primary_expression ::= string_literal.
primary_expression ::= LPAREN expression RPAREN.

string_literal ::= STRING_LITERAL.
string_literal ::= string_literal STRING_LITERAL.

statement ::= compound_statement.
statement ::= expression_statement.
statement ::= selection_statement.
statement ::= iteration_statement.
statement ::= jump_statement.

jump_statement ::= RETURN SEMICOLON.
jump_statement ::= RETURN expression SEMICOLON.
jump_statement ::= BREAK SEMICOLON.
jump_statement ::= CONTINUE SEMICOLON.

iteration_statement ::= WHILE LPAREN expression RPAREN statement.
iteration_statement ::= FOR LPAREN expression_statement expression_statement expression RPAREN statement.

selection_statement ::= IF LPAREN expression RPAREN statement. [LOWER_THAN_ELSE]
selection_statement ::= IF LPAREN expression RPAREN statement ELSE statement.

statement_list ::= statement.
statement_list ::= statement_list statement.
