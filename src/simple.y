%{
int yylex();
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

long symbols[255];
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
%token T_EXIT T_ASSIGN
%token <num> T_INT T_STR
%token <id> identifier

%left T_MUL T_DIV T_MOD T_ADD T_SUB

%type <num> stmt exp term 
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
    | T_PRINT T_STR T_NEWLINE     {printf("%s", (char*)($2));}
    ;

assig: identifier T_ASSIGN exp  { updateSymbolVal($1,$3);}
    ;

const_assig: identifier T_ASSIGN exp  { updateSymbolVal($1,$3);}
    ;

exp: term                  {$$ = $1;}
	| T_SUB exp 				{$$ = - $2; }
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


int computeSymbolIndex(char token) {
	int idx = -1;
	if(islower(token)) {
		idx = token - 'a' + 26;
	} else if(isupper(token)) {
		idx = token - 'A';
	}
	return idx;
} 


long symbolVal(char symbol) {
	int bucket = computeSymbolIndex(symbol);
	return symbols[bucket];
}


void updateSymbolVal(char symbol, long val) {
	int bucket = computeSymbolIndex(symbol);
	symbols[bucket] = val;
}
