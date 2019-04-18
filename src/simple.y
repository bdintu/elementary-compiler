%{

int yylex();
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap.c"
hashtable_t *symbols;

void symbolsInit();
long symbolVal(char symbol);
void updateSymbolVal(char symbol, long val);
%}


%union {
    long num;
    char id;
}

%start start
%token T_PRINT T_NEWLINE T_CONST
%token T_IF T_ELSE T_WHILE
%token T_LEFTPAREN T_RIGHTPAREN T_OPENBRACE T_CLOSEBRACE T_LEFTSQUARE T_RIGHTSQUARE
%token T_ADD T_SUB T_MUL T_DIV T_MOD
%token T_EXIT T_ASSIGN T_EQ T_NE T_GT T_LT
%token <num> T_INT T_STR
%token <id> identifier

%left T_MUL T_DIV T_MOD T_ADD T_SUB
%left T_IF T_ELSE
%left T_EQ T_NE T_GT T_LT

%type <num> stmt exp term condition
%type <id> assig const_assig


%%
start:
    | start stmt
    ;

stmt: T_NEWLINE                 {}
    | T_CONST const_assig T_NEWLINE      {}
    | assig T_NEWLINE      {}
    | T_EXIT T_NEWLINE          {exit(0); }
    | T_PRINT term T_NEWLINE    {printf("%ld", $2);}
    | T_PRINT exp T_NEWLINE     {printf("%ld", $2);}
    | T_PRINT T_STR T_NEWLINE   {printf("%s", (char*)($2));}
    | ifelse T_NEWLINE          {}
    | whileloop T_NEWLINE          {}
    ;

assig: identifier T_ASSIGN exp  { updateSymbolVal($1,$3);}
    ;

const_assig: identifier T_ASSIGN exp  { updateSymbolVal($1,$3);}
    ;

ifelse: T_IF condition T_OPENBRACE exp T_CLOSEBRACE {}
    | T_IF condition T_OPENBRACE exp T_CLOSEBRACE T_ELSE ifelse {}
    | T_IF condition T_OPENBRACE exp T_CLOSEBRACE T_ELSE T_OPENBRACE exp T_CLOSEBRACE {}
    ;

whileloop: T_WHILE condition T_OPENBRACE exp T_CLOSEBRACE {}
    ;

condition: term
    | exp T_GT exp           {$$ =  $1 > $3? 1: 0;}
	| exp T_LT exp           {$$ =  $1 < $3? 1: 0;}
	| exp T_EQ exp          {$$ = $1 == $3? 1: 0;}
	| exp T_NE exp          {$$ = $1 != $3? 1: 0;}
    ;

exp: term                   {$$ = $1;}
	| T_SUB exp 			{$$ = -$2; }
    | T_LEFTPAREN exp T_RIGHTPAREN		{$$ = $2;}
    | exp T_MUL exp         {$$ = $1 * $3;}
	| exp T_DIV exp         {$$ = $1 / $3;}
	| exp T_MOD exp         {$$ = $1 % $3;}
    | exp T_ADD exp         {$$ = $1 + $3;}
    | exp T_SUB exp         {$$ = $1 - $3;}
    ;

term: T_INT                {$$ = $1;}
    | identifier			{$$ = symbolVal($1);} 
    ;
%%


void symbolsInit() {
	symbols = ht_create( 65536 );
}
    
long symbolVal(char symbol) {
    char *p = malloc(sizeof(char));
    *p = symbol;
	return ht_get(symbols, p);
}

void updateSymbolVal(char symbol, long val) {
    char *p = malloc(sizeof(char));
    *p = symbol;
	ht_set(symbols, p, val);
    free(p);
}
