%{
int yylex();
#include <stdio.h>     /* c declarations used in actions */
#include <stdlib.h>
#include <ctype.h>
int symbols[255];
int symbolVal(char symbol);
void updateSymbolVal(char symbol, int val);
%}


%union {int num; char id;}         /* Yacc definitions */
%start startline
%token T_PRINT T_NEWLINE T_CONST
%token T_IF T_ELSE T_WHILE
%token T_LEFTPAREN T_RIGHTPAREN T_OPENBRACE T_CLOSEBRACE T_LEFTSQUARE T_RIGHTSQUARE
%token T_ADD T_SUB T_MUL T_DIV T_MOD
%token T_EXIT T_ASSIGN
%token <num> number
%token <id> identifier

%left T_ADD T_SUB T_MUL T_DIV T_MOD

%type <num> line exp term 
%type <id> assignment


%%
startline:
    | startline line
;

line: T_NEWLINE                 {}
    | T_CONST assignment T_NEWLINE      {}
    | assignment T_NEWLINE      {}
    | T_EXIT T_NEWLINE          {exit(0); }
    | T_PRINT term T_NEWLINE    {printf("%i\n", $2);}
    | T_PRINT exp T_NEWLINE     {printf("%i\n", $2);}
    ;

assignment: identifier T_ASSIGN exp  { updateSymbolVal($1,$3);}
    ;

exp: term                  {$$ = $1;}
    | exp T_ADD exp          {$$ = $1 + $3;}
    | exp T_SUB exp          {$$ = $1 - $3;}
    | exp T_MUL exp          {$$ = $1 * $3;}
	| exp T_DIV exp			{$$ = $1 / $3;}
	| exp T_MOD exp          {$$ = $1 % $3;}
	| T_SUB exp 				{$$ = - $2; }
    | T_LEFTPAREN exp T_RIGHTPAREN		{$$ = $2;}
    ;

term: number                {$$ = $1;}
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


int symbolVal(char symbol) {
	int bucket = computeSymbolIndex(symbol);
	return symbols[bucket];
}


void updateSymbolVal(char symbol, int val) {
	int bucket = computeSymbolIndex(symbol);
	symbols[bucket] = val;
}
