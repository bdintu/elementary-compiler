%{

int yylex();
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.c"
map_long_t m;

void symbolsInit();
void createSymbol(char* symbol, long val, int is_const);
void updateSymbol(char* symbol, long val);
long readSymbol(char* symbol);


char* arrayname[255];
void createArray(char* symbol, int size);
void updateArray(char* symbol, int index, long val);
long readArray(char* symbol, int index);

char* sout[255], sbuffer[255];
void printInit();
void printStr();
void addStr(char* str);
void addInt(long ival);
void addHex(long ival);
%}


%union {
    long num;
    char* id;
}

%start start
%token T_PRINT T_NEWLINE T_CONST T_VAR T_DEL
%token T_IF T_ELSE T_WHILE T_COMMA T_SEMI T_HEX
%token T_LEFTPAREN T_RIGHTPAREN T_OPENBRACE T_CLOSEBRACE T_LEFTSQUARE T_RIGHTSQUARE
%token T_ADD T_SUB T_MUL T_DIV T_MOD
%token T_EXIT T_ASSIGN T_EQ T_NE T_GT T_LT
%token <num> T_INT T_STR
%token <id> identifier

%left T_MUL T_DIV T_MOD T_ADD T_SUB
%left T_IF T_ELSE T_WHILE
%left T_EQ T_NE T_GT T_LT

%type <num> stmt exp term condition stringformat
%type <id> assign update


%%
start:
    | start stmt
    ;

stmt: T_NEWLINE                 {}
    | assign                     {}
    | update                     {}
    | T_EXIT                    {exit(0);}
    | T_PRINT term              {printf("%ld", $2);}
    | T_PRINT exp               {printf("%ld", $2);}
    | T_PRINT stringformat      {printStr();}
    | ifelse                    {}
    | whileloop                 {}
    | stmt T_SEMI stmt          {}
    | stmt T_SEMI stmt T_SEMI   {}
    ;

assign: T_CONST identifier T_ASSIGN exp  { createSymbol($2,$4,1);}
    | T_VAR identifier  { createSymbol($2,0,0);}
    | T_VAR identifier T_ASSIGN exp  { createSymbol($2,$4,0);}
    | T_VAR identifier T_LEFTSQUARE exp T_RIGHTSQUARE { createArray($2,$4);}
    ;

update: identifier T_ASSIGN exp {updateSymbol($1,$3);}
    | identifier T_LEFTSQUARE exp T_RIGHTSQUARE T_ASSIGN exp { updateArray($1,$3,$6);}
    ;

ifelse: T_IF condition T_OPENBRACE stmt T_CLOSEBRACE {}
    | T_IF condition T_OPENBRACE stmt T_CLOSEBRACE T_ELSE ifelse {}
    | T_IF condition T_OPENBRACE stmt T_CLOSEBRACE T_ELSE T_OPENBRACE stmt T_CLOSEBRACE {}
    ;

whileloop: T_WHILE condition T_OPENBRACE stmt T_CLOSEBRACE {}
    ;

condition: term
    | exp T_GT exp          {$$ =  $1 > $3? 1: 0;}
	| exp T_LT exp          {$$ =  $1 < $3? 1: 0;}
	| exp T_EQ exp          {$$ = $1 == $3? 1: 0;}
	| exp T_NE exp          {$$ = $1 != $3? 1: 0;}
    ;

stringformat: exp                   {addInt($1);}
    | T_STR                         {addStr($1);}
    | T_HEX exp T_RIGHTPAREN        {addHex($2);}
    | exp T_COMMA stringformat      {addInt($1);}
    | T_STR T_COMMA stringformat    {addStr($1);}
    | T_HEX exp T_RIGHTPAREN T_COMMA stringformat   {addHex($2);}

exp: term                   {$$ = $1;}
	| T_SUB exp 			{$$ = -$2; }
    | T_LEFTPAREN exp T_RIGHTPAREN		{$$ = $2;}
    | exp T_MUL exp         {$$ = $1 * $3;}
	| exp T_DIV exp         {$$ = $1 / $3;}
	| exp T_MOD exp         {$$ = $1 % $3;}
    | exp T_ADD exp         {$$ = $1 + $3;}
    | exp T_SUB exp         {$$ = $1 - $3;}
    ;

term: T_INT                 {$$ = $1;}
    | identifier			{$$ = readSymbol($1);} 
    | identifier T_LEFTSQUARE exp T_RIGHTSQUARE   {$$ = readArray($1,$3);} 
    ;
%%


void symbolsInit() {
    map_init(&m);
}

void createSymbol(char* symbol, long val, int is_const) {
    map_set(&m, symbol, val, is_const);
}
    
void updateSymbol(char* symbol, long val) {

    long test = readSymbol(symbol);
    
    int is_const = map_get_isconst(&m, symbol);
    if (is_const) {
        yyerror("var is const");
    } else {
        map_set(&m, symbol, val, 0);
    }
}

long readSymbol(char* symbol) {
    long *val = map_get(&m, symbol);
    if (val) {
        return *val;
    } else {
        yyerror("not found!");
    }
}

void genArrayName(char* symbol, int index) {
    sprintf(arrayname, "%d", index);
    strcat(arrayname, symbol);
}

void createArray(char* symbol, int size) {
    for (int i=0; i<size; ++i) {
        genArrayName(symbol, i);
        createSymbol(arrayname, 0, 0);
    }
}
    
void updateArray(char* symbol, int index, long val) {
    genArrayName(symbol, index);
    updateSymbol(arrayname, val);
}

long readArray(char* symbol, int index) {
    genArrayName(symbol, index);
    return readSymbol(arrayname);
}

void printInit() {
    strcpy(sout, "");
    strcpy(sbuffer, "");
}
 
void printStr() {
    printf("%s ", sout);
    strcpy(sout, "");
}

void addStr(char* str) {
    char *tmp = strdup(sout);
    strcpy(sout, str);
    strcat(sout, tmp);
    free(tmp);
}

void addInt(long ival) {
    sprintf(sbuffer, "%ld", ival);
    addStr(sbuffer);
}

void addHex(long ival) {
    sprintf(sbuffer, "0x%lx", ival);
    addStr(sbuffer);
}
