//
// C subset grammar.  This uss the lemon parser generator
// http://www.hwaci.com/sw/lemon/lemon.html to create the parser.
//

%include {   

#include <iostream>  

#include "Tokens.h"
#include "cparse.h"
#include "CParser.h"
#include "Node.h"

  using namespace std;

  Node *declaration(Type *typespec,Node *declarator) {
    Declaration &d = dynamic_cast<Declaration&>(*declarator);
    if (dynamic_cast<FunctionType*>(declarator->type())) {
      d.set_extern();
    }
    d.set_base_type(typespec);
    return declarator;
  }

}

%nonassoc LOWER_THAN_ELSE.
%nonassoc ELSE.

%start_symbol translation_unit

%extra_argument { CParser *cp }

%token_type {Tokens }
%type expr {int}

%syntax_error {  
  cerr << cp->filename() << ":" << cp->linenumber() << ":  syntax error." << endl;  
  cp->seterror();
}   

%parse_failure {
  cerr << "Parse failed!" << endl;
}

%default_type { Node * }

translation_unit ::= extern_decl_list(B). {
  cp->add_translation_unit(B);
}

extern_decl_list(A) ::= extern_decl(B). {
  A = new TranslationUnit(B);
}
extern_decl_list(A) ::= extern_decl_list(B) extern_decl(C). {
  A = B;
  B->add(C);
}

extern_decl(A) ::= function_definition(B). {
  A = B;
}
extern_decl(A) ::= declaration(B). {
  A = B;
}

function_definition(A) ::= type_specifier(B) declarator(C) compound_statement(D). {
  Declaration &d = dynamic_cast<Declaration&>(*C);  
  d.set_base_type(B);
  A = new FunctionDefn(C,D);
}
function_definition(A) ::= STATIC type_specifier(B) declarator(C) compound_statement(D). {
  Declaration &d = dynamic_cast<Declaration&>(*C);  
  d.set_static();
  d.set_base_type(B);
  A = new FunctionDefn(C,D);
}

declaration(A) ::= type_specifier(B) declarator(C) SEMICOLON. {
  A = declaration(B,C);  
}
declaration(A) ::= EXTERN type_specifier(B) declarator(C) SEMICOLON. {
  A = C;
  Declaration &d = dynamic_cast<Declaration&>(*A);
  d.set_extern();
  d.set_base_type(B);
}

declaration_list_opt(A) ::= . {
  A = new NullNode();
}
declaration_list_opt(A) ::= declaration_list(B). {
  A = B;
}

declaration_list(A) ::= declaration(B). {
  A = new DeclarationList(B);
}
declaration_list(A) ::= declaration_list(B) declaration(C). {
  A = B;
  B->add(C);
}

%type type_specifier { Type * }
type_specifier(A) ::= INT. {
  A = new BaseType(BaseType::Int);
}
type_specifier(A) ::= CHAR. {
  A = new BaseType(BaseType::Char);
}

declarator(A) ::= direct_declarator(B). {
  A = B;
}
declarator(A) ::= ASTERISK declarator(B). {
  A = B;
  dynamic_cast<Declaration&>(*A).set_base_type(new PointerType);
}

direct_declarator(A) ::= IDENTIFIER(B). {
  A = new Declaration(String(B._str.p,B._str.l));
}
direct_declarator(A) ::= direct_declarator(B) LPAREN parameter_type_list(C) RPAREN. {
  A = B;
  dynamic_cast<Declaration&>(*A).add_type(new FunctionType(C));
}
direct_declarator(A) ::= direct_declarator(B) LPAREN RPAREN. {
  A = B;
  dynamic_cast<Declaration&>(*A).add_type(new FunctionType(new ParamList));
}

parameter_type_list(A) ::= parameter_list(B). {
  A = B;
}
parameter_type_list(A) ::= parameter_list(B) COMMA ELLIPSIS. {
  A = B;
  dynamic_cast<ParamList&>(*A).set_has_ellipsis();
}

parameter_list(A) ::= parameter_declaration(B). {
  A = new ParamList(B);
}
parameter_list(A) ::= parameter_list(B) COMMA parameter_declaration(C). {
  A = B;
  A->add(C);
}

parameter_declaration(A) ::= type_specifier(B) declarator(C). {
  A = declaration(B,C);
}

compound_statement(A) ::= LBRACE declaration_list_opt(B) statement_list(C) RBRACE. {
  A = new CompoundStatement(B,C);
}
compound_statement(A) ::= LBRACE declaration_list_opt(B) RBRACE. {
  A = new CompoundStatement(B,new NullNode());
}

expression_statement(A) ::= expression(B) SEMICOLON. {
  A = B;
}

expression(A) ::= equality_expression(B). {
  A = B;
}
expression(A) ::= equality_expression(B) ASSIGN(Op) expression(C). {
  A = new Binop(B,Op._tok,C);
}
expression(A) ::= equality_expression(B) ADD_ASSIGN(Op) expression(C). {
  A = new Binop(B,Op._tok,C);
}
expression(A) ::= equality_expression(B) SUB_ASSIGN(Op) expression(C). {
  A = new Binop(B,Op._tok,C);
}

equality_expression(A) ::= relational_expression(B). {
  A = B;
}
equality_expression(A) ::= equality_expression(B) EQ(Op) relational_expression(C). {
  A = get_calculated(new Binop(B,Op._tok,C));
}
equality_expression(A) ::= equality_expression(B) NOT_EQ(Op) relational_expression(C). {
  A = get_calculated(new Binop(B,Op._tok,C));
}

relational_expression(A) ::= additive_expression(B). {
  A = B;
}
relational_expression(A) ::= relational_expression(B) LESS(Op) additive_expression(C). {
  A = get_calculated(new Binop(B,Op._tok,C));
}
relational_expression(A) ::= relational_expression(B) GREATER(Op) additive_expression(C). {
  A = get_calculated(new Binop(B,Op._tok,C));
}
relational_expression(A) ::= relational_expression(B) LESS_EQ(Op) additive_expression(C). {
  A = get_calculated(new Binop(B,Op._tok,C));
}
relational_expression(A) ::= relational_expression(B) GREATER_EQ(Op) additive_expression(C). {
  A = get_calculated(new Binop(B,Op._tok,C));
}

postfix_expression(A) ::= primary_expression(B). {
  A = B;
}
postfix_expression(A) ::= postfix_expression(B) LPAREN argument_expression_list(C) RPAREN. {
  A = new FunctionExpression(B,C);
}
postfix_expression(A) ::= postfix_expression(B) LPAREN RPAREN. {
  A = new FunctionExpression(B,new ArgumentList);
}
postfix_expression(A) ::= postfix_expression(B) LBRACKET expression(C) RBRACKET. {
  A = new ArrayExpression(B,C);
}  

argument_expression_list(A) ::= expression(B). {
  A = new ArgumentList(B);
  cp->setinfo(A);
}
argument_expression_list(A) ::= argument_expression_list(B) COMMA expression(C). {
  A = B;
  A->add(C);
}

unary_expression(A) ::= postfix_expression(B). {
  A = B;
}
unary_expression(A) ::= MINUS unary_expression(B). {
  A = get_calculated(new Negative(B));
  cp->setinfo(A);
}
unary_expression ::= PLUS unary_expression.
unary_expression(A) ::= EXCLAMATION unary_expression(B). {
  // Hack:  replace !expr with (expr == 0).
  A = get_calculated(new Binop(B,EQ,new Const(0,new BaseType(BaseType::Int)))); 
  cp->setinfo(A);
}
unary_expression(A) ::= ASTERISK unary_expression(B). {
  A = new Pointer(B);
  cp->setinfo(A);
}
unary_expression(A) ::= AMPERSAND unary_expression(B). {
  A = new AddrOf(B);
  cp->setinfo(A);
}

mult_expression(A) ::= unary_expression(B). {
  A = B;
}
mult_expression(A) ::= mult_expression(B) ASTERISK(Op) unary_expression(C). {
  A = get_calculated(new Binop(B,Op._tok,C));
  cp->setinfo(A);
}
mult_expression(A) ::= mult_expression(B) DIV(Op) unary_expression(C). {
  A = get_calculated(new Binop(B,Op._tok,C));
  cp->setinfo(A);
}
mult_expression(A) ::= mult_expression(B) MODULO(Op) unary_expression(C). {
  A = get_calculated(new Binop(B,Op._tok,C));
  cp->setinfo(A);
}

additive_expression(A) ::= mult_expression(B). {
  A = B;
}
additive_expression(A) ::= additive_expression(B) PLUS(Op) mult_expression(C). {
  A = get_calculated(new Binop(B,Op._tok,C));
  cp->setinfo(A);
}
additive_expression(A) ::=  additive_expression(B) MINUS(Op) mult_expression(C). {
  A = get_calculated(new Binop(B,Op._tok,C));
  cp->setinfo(A);
}

primary_expression(A) ::= IDENTIFIER(B). { 
  A = new Id(String(B._str.p,B._str.l)); 
  cp->setinfo(A); 
}
primary_expression(A) ::= INTCONST(B). { 
  A = new Const(B._int,new BaseType(BaseType::Int)); 
  cp->setinfo(A); 
}
primary_expression(A) ::= FPCONST(B). { 
  A = new Const((int)B._fp,new BaseType(BaseType::Double)); 
  cp->setinfo(A); 
}
primary_expression(A) ::= CHARCONST(B). { 
  A = new Const(B._int,new BaseType(BaseType::Char)); 
  cp->setinfo(A); 
}
primary_expression(A) ::= string_literal(B). {
  A = B;
}
primary_expression(A) ::= LPAREN expression(B) RPAREN. { 
  A = B; 
}

%type string_literal { StringLiteral * }
string_literal(A) ::= STRING_LITERAL(B). { 
  A = new StringLiteral(String(B._str.p,B._str.l)); 
  cp->setinfo(A); 
}
string_literal(A) ::= string_literal(B) STRING_LITERAL(C). { 
  A = B;
  B->append(String(C._str.p,C._str.l)); 
}

statement(A) ::= compound_statement(B). {
  A = B;
}
statement(A) ::= expression_statement(B). {
  A = B;
}
statement(A) ::= selection_statement(B). {
  A = B;
}
statement(A) ::= iteration_statement(B). {
  A = B;
}
statement(A) ::= jump_statement(B). {
  A = B;
}

jump_statement(A) ::= RETURN SEMICOLON. { 
  A = new ReturnStatement; 
  cp->setinfo(A); 
}
jump_statement(A) ::= RETURN expression(B) SEMICOLON. { 
  A = new ReturnStatement(B); 
  cp->setinfo(A); 
}
jump_statement(A) ::= BREAK SEMICOLON. { 
  A = new BreakStatement; 
  cp->setinfo(A); 
}
jump_statement(A) ::= CONTINUE SEMICOLON. { 
  A = new ContinueStatement; 
  cp->setinfo(A); 
}

iteration_statement(A) ::= WHILE LPAREN expression(B) RPAREN statement(C). { 
  A = new WhileLoop(B,C); 
  cp->setinfo(A); 
}
iteration_statement(A) ::= FOR LPAREN expression_statement(B) expression_statement(C) expression(D) RPAREN statement(E). { 
  A = new ForLoop(B,C,D,E); 
  cp->setinfo(A); 
}

selection_statement(A) ::= IF LPAREN expression(B) RPAREN statement(C). [LOWER_THAN_ELSE] { 
  A = new IfStatement(B,C,new NullNode); 
  cp->setinfo(A); 
}
selection_statement(A) ::= IF LPAREN expression(B) RPAREN statement(C) ELSE statement(D). { 
  A = new IfStatement(B,C,D);  
  cp->setinfo(A); 
}

statement_list(A) ::= statement(B). { 
  A = new StatementList(B); 
  cp->setinfo(A); 
}
statement_list(A) ::= statement_list(B) statement(C). { 
  A = B; 
  A->add(C); 
}
