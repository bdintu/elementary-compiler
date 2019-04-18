#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern void symbolsInit();
extern void printInit();
extern int yylex();
extern int yyparse();
extern FILE* yyin;
int yyerror(const char *s);


int main() {
    symbolsInit();
    printInit();

    yyin = stdin;

    do{
        yyparse();         
    }while(!feof(yyin));

	return 0;
}


int yyerror(const char* s) {
	fprintf(stderr, "Parse error: %s\n", s);
	exit(1);
}
