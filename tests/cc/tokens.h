//
//
//

#ifndef _TOKENS_H_
#define _TOKENS_H_

struct Tokens {
  int _tok;    // The token itself.
  int _ln;     // Line number.
  union {
    // For strings or identifiers:  Pointer to the
    // data and its length.
    struct {
      const char *p;
      int l;
    } _str;
    // For integer data.
    int _int;
    // For floating point data.
    double _fp;
  };
};

#define ALL_DONE -1
#define AUTO 1
#define BREAK 2
#define CASE 3
#define CHAR 4
#define CONST 5
#define CONTINUE 6
#define DEFAULT 7
#define DO 8
#define DOUBLE 9
#define ELSE 10
#define ENUM 11
#define EXTERN 12
#define FLOAT 13
#define FOR 14
#define GOTO 15
#define IF 16
#define INT 17
#define LONG 18
#define REGISTER 19
#define RETURN 20
#define SHORT 21
#define SIGNED 22
#define SIZEOF 23
#define STATIC 24
#define STRUCT 25
#define SWITCH 26
#define TYPEDEF 27
#define UNION 28
#define UNSIGNED 29
#define VOID 30
#define VOLATILE 31
#define WHILE 32
#define IDENTIFIER 33
#define INTCONST 34
#define FPCONST 35 
#define STRING_LITERAL 36
#define ELLIPSIS 37
#define RIGHT_ASSIGN 38
#define LEFT_ASSIGN 39
#define ADD_ASSIGN 40
#define SUB_ASSIGN 41
#define MUL_ASSIGN 42
#define DIV_ASSIGN 43
#define MOD_ASSIGN 44
#define AND_ASSIGN 45
#define XOR_ASSIGN 46
#define OR_ASSIGN 47
#define RIGHT_OP 48
#define LEFT_OP 49
#define INC_OP 50
#define DEC_OP 51
#define PTR_OP 52
#define AND_OP 53
#define OR_OP 54
#define LE_OP 55
#define GE_OP 56
#define EQ_OP 57
#define NE_OP 58

#endif
