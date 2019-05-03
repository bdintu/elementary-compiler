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
%token T_ASSIGN T_CONST T_VAR
%token T_NEWLINE
%token T_IF T_ELSE T_FOR
%token T_PRINT T_HEX

%right T_ASSIGN
%left '+' '-'
%left '*' '/' '%'
%left T_EQ T_NE T_GE T_LE T_GT T_LT
%nonassoc NEG

%type <node> stmt exp if term block

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
| if
| T_FOR exp ':' exp '{' block '}'      { $$ = newFor($2, $4, $6); }
;

if:
  T_IF exp T_EQ exp '{' block '}'      { $$ = newIf($2, $4, $6, NULL, 'e'); }
| T_IF exp T_NE exp '{' block '}'      { $$ = newIf($2, $4, $6, NULL, 'n'); }
| T_IF exp T_GE exp '{' block '}'      { $$ = newIf($2, $4, $6, NULL, 'h'); }
| T_IF exp T_LE exp '{' block '}'      { $$ = newIf($2, $4, $6, NULL, 'm'); }
| T_IF exp T_GT exp '{' block '}'      { $$ = newIf($2, $4, $6, NULL, 'g'); }
| T_IF exp T_LT exp '{' block '}'      { $$ = newIf($2, $4, $6, NULL, 'l'); }

| T_IF exp T_EQ exp '{' block '}' T_ELSE '{' block '}'  { $$ = newIf($2, $4, $6, $10, 'e'); }
| T_IF exp T_NE exp '{' block '}' T_ELSE '{' block '}'  { $$ = newIf($2, $4, $6, $10, 'n'); }
| T_IF exp T_GE exp '{' block '}' T_ELSE '{' block '}'  { $$ = newIf($2, $4, $6, $10, 'h'); }
| T_IF exp T_LE exp '{' block '}' T_ELSE '{' block '}'  { $$ = newIf($2, $4, $6, $10, 'm'); }
| T_IF exp T_GT exp '{' block '}' T_ELSE '{' block '}'  { $$ = newIf($2, $4, $6, $10, 'g'); }
| T_IF exp T_LT exp '{' block '}' T_ELSE '{' block '}'  { $$ = newIf($2, $4, $6, $10, 'e'); }

exp:
  term
| T_CONST VAR T_ASSIGN exp         { $$ = newDeclar($2, $4, 1); }
| T_VAR VAR T_ASSIGN exp         { $$ = newDeclar($2, $4, 0); }
| T_VAR VAR                { $$ = newDeclar($2, NULL, 0); }
| T_VAR VAR '[' NUM ']'    { $$ = newArray($2, $4); }
| VAR T_ASSIGN exp         { $$ = newAssign($1, $3); }
| VAR '[' exp ']' T_ASSIGN exp     { $$ = newArrayAssign($1, $6, $3); }
| exp '+' exp               { $$ = newNode($1, $3, '+'); }
| exp '-' exp               { $$ = newNode($1, $3, '-'); }
| exp '*' exp               { $$ = newNode($1, $3, '*'); }
| exp '/' exp               { $$ = newNode($1, $3, '/'); }
| exp '%' exp               { $$ = newNode($1, $3, '%'); }
| '^' exp %prec NEG         { $$ = newNode($2, NULL, '^'); }
| '(' exp ')'               { $$ = $2; }
| T_PRINT TEXT               { $$ = newPrint(NULL, $2, 'S'); }
| T_PRINT exp                { $$ = newPrint($2, NULL, 'D'); }
| T_PRINT T_HEX '(' exp ')'                { $$ = newPrint($4, NULL, 'H'); }
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
