#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

extern int yylex();
extern int yyparse();
extern FILE* yyin;
extern long symbols[255];
int yyerror(const char *s);


int main() {
	int i;
	for(i = 0 ; i < 255 ; i++) {
		symbols[i] = 0;
	}
		
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
