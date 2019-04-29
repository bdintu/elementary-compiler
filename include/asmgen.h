#ifndef ASMGEN_INCLUDED
#define ASMGEN_INCLUDED


#include <stdlib.h>
#include <stdint.h>
#include "node.h"

#define SEED 8699

/* simple symtab of fixed size */
struct symbol symtab[SEED];

struct symbol* lookup(char*, uint8_t, uint8_t);


/* evaluate an ast */
int64_t eval (struct ast* );
void asmGen (struct ast* );

/* delete and free an ast */
void freeNode (struct ast* );

/* interface to the lexer */
extern int yylineno;    /* from lexer */
void yyerror (char *s, ...);


#endif // ASMGEN_INCLUDED
