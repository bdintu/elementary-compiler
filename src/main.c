#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern void symbolsInit();
extern char* stringBuffer;
extern int yylex();
extern int yyparse();
extern FILE* yyin;
int yyerror(const char *s);


int main() {
    symbolsInit();
    stringBuffer = (char*)malloc(1024*sizeof(char));

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
