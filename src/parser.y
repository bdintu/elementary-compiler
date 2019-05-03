%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "asmgen.h"
#include "node.h"

extern int errorflag;

%}

%union {
    struct ast* node;
    struct symbol* sym;
    int64_t num;
    char* str;
}

%token<str> TEXT
%token<num> NUM
%token<sym> VAR
%token TK_ASSIGN TK_CONST TK_VAR
%token TK_NEWLINE
%token TK_IF TK_FOR
%token TK_PRINT TK_HEX

%right TK_ASSIGN
%left '+' '-'
%left '*' '/' '%'
%left TK_EQ TK_NE TK_GE TK_LE TK_GT TK_LT
%nonassoc NEG

%type <node> stmt exp term block

%start program

%%

program:
| program stmt      {   
                      asmGen($2);
                      freeNode($2);
                    }
| program error ';' { errorflag = 1; yyerrok; }
;

stmt:
  exp 
|  exp ';'
| TK_IF exp TK_EQ exp '{' block '}'      { $$ = newIfe($2, $4, $6, 'e'); }
| TK_IF exp TK_NE exp '{' block '}'      { $$ = newIfe($2, $4, $6, 'n'); }
| TK_IF exp TK_GE exp '{' block '}'      { $$ = newIfe($2, $4, $6, 'h'); }
| TK_IF exp TK_LE exp '{' block '}'      { $$ = newIfe($2, $4, $6, 'm'); }
| TK_IF exp TK_GT exp '{' block '}'      { $$ = newIfe($2, $4, $6, 'g'); }
| TK_IF exp TK_LT exp '{' block '}'      { $$ = newIfe($2, $4, $6, 'l'); }
| TK_FOR exp ':' exp '{' block '}'      { $$ = newVon($2, $4, $6); }
;

exp:
  term
| TK_CONST VAR TK_ASSIGN exp         { $$ = newDeclar($2, $4, 1); }
| TK_VAR VAR TK_ASSIGN exp         { $$ = newDeclar($2, $4, 0); }
| TK_VAR VAR                { $$ = newDeclar($2, NULL, 0); }
| TK_VAR VAR '[' NUM ']'    { $$ = newArray($2, $4); }
| VAR TK_ASSIGN exp         { $$ = newAssign($1, $3); }
| VAR '[' exp ']' TK_ASSIGN exp     { $$ = newArrayAssign($1, $6, $3); }
| exp '+' exp               { $$ = newNode($1, $3, '+'); }
| exp '-' exp               { $$ = newNode($1, $3, '-'); }
| exp '*' exp               { $$ = newNode($1, $3, '*'); }
| exp '/' exp               { $$ = newNode($1, $3, '/'); }
| exp '%' exp               { $$ = newNode($1, $3, '%'); }
| '^' exp %prec NEG         { $$ = newNode($2, NULL, '^'); }
| '(' exp ')'               { $$ = $2; }
| TK_PRINT TEXT               { $$ = newPrintStmt(NULL, $2, 'S'); }
| TK_PRINT exp                { $$ = newPrintStmt($2, NULL, 'D'); }
| TK_PRINT TK_HEX '(' exp ')'                { $$ = newPrintStmt($4, NULL, 'H'); }
;

term:
  NUM                       { $$ = newNum($1); }
| VAR                       { $$ = newVar($1); }
| VAR '[' exp ']'           { $$ = newVarArray($1, $3); }
;

block:
                            { $$ = NULL; }
| stmt block                { 
                              if ($2 == NULL) {
                                $$ = $1;
                              } else {
                                $$ = newNode($1, $2, 'B');
                              }
                            }
;
