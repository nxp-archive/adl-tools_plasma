//
// C subset grammar.  This uss the lemon parser generator
// http://www.hwaci.com/sw/lemon/lemon.html to create the parser.
//

%include {   

#include <iostream>  

#include "Tokens.h"
#include "cparse.h"

  using namespace std;

}

%token_type {Tokens}
%type expr {int}

%left PLUS MINUS.   
%left DIVIDE MULT.  

%parse_accept {
  cout << "Parsing complete!" << endl;
}

%syntax_error {  
  std::cout << "Syntax error!" << std::endl;  
}   

cprog ::= stmt_list .
cprog ::= .

stmt_list ::= stmt.
stmt_list ::= stmt_list stmt. 

stmt ::= expr(A) SEMICOLON. { cout << "Result:  " << A << endl; }

expr(A) ::= expr(B) MINUS  expr(C).   { A = B - C; }  
expr(A) ::= expr(B) PLUS   expr(C).   { A = B + C; }  
expr(A) ::= expr(B) MULT   expr(C).   { A = B * C; }  
expr(A) ::= expr(B) DIVIDE expr(C).  { 
         if(C != 0){
           A = B / C;
         }else{
           cout << "divide by zero" << endl;
         }
}

expr(A) ::= INTCONST(B). { A = B._int; } 

