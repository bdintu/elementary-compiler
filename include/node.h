#ifndef NODE_INCLUDED
#define NODE_INCLUDED


#include <stdint.h>

/* symbol table */
struct symbol {             /* a variable name */
    char *name;
    int offset;
    uint8_t size;
    uint8_t is_const;
};

/* node types
 * + - * / %
 * M unary minus
 * E expression list
 * C IF statement
 * L LOOP statement่
 * ์V symbol ref
 * = assignment
 * K constant
 * S literal string
 * A Expression
 */

/* node in the abstraction syntax tree */
/* all have common initial nodetype */
struct ast {
    int nodetype;
    struct ast* l;
    struct ast* r;
};

struct cond {
    int nodetype;       /* type C */
    struct ast* fStmt;   /* condition */
    struct ast* sStmt;
    struct ast* tl;     /* then branch or do list */
    struct ast* tr;     /* then branch or do list */
    char op;
};

struct loop {
    int nodetype;       /* type L */
    struct ast* from;   /* start number */
    struct ast* to;     /* end number */
    struct ast* tl;     /* then branch or do list */
};

struct lString {
    char* str;
    uint8_t label;
};

struct print {
    int nodetype;           /* type D H S */
    union arg {             /* argument of the print function */
        struct lString* ls;  /* string argument */
        struct ast* exp;    /* expression argument */
    } arg;
};

struct numval {
    int nodetype;       /* type N */
    int64_t number;
};

struct symref {
    int nodetype;       /* type V */
    struct symbol* s;
};

struct symarrayref {
    int nodetype;       /* type R */
    struct symbol* s;
    struct ast* v;      /* value */
};

struct symasgn {
    int nodetype;       /* type = */
    struct symbol* s;
    struct ast* v;      /* value */
    int is_const;      /* is_const */
};

struct symarray {
    int nodetype;       /* type A */
    struct symbol* s;
    struct ast* v;      /* value */
    struct ast* offset;      /* value */
    uint8_t size;
};


/* build an ast */
struct ast* newNode (struct ast* l, struct ast* r, int nodetype);
struct ast* newPrintStmt (struct ast *exp, char *str, int nodetype);
struct ast* newVar (struct symbol *s);
struct ast* newVarArray (struct symbol *s, struct ast* v);
struct ast* newDeclar (struct symbol *s, struct ast* v, uint8_t is_const);
struct ast* newArray (struct symbol *s, uint8_t size);
struct ast* newAssign (struct symbol *s, struct ast* v);
struct ast* newArrayAssign (struct symbol *s, struct ast* v, struct ast* offset);
struct ast* newNum (int64_t num);
struct ast* newIfe (struct ast* firstStmt, struct ast* secStmt, struct ast* tl, struct ast* tr, char op);
struct ast* newVon (struct ast* from, struct ast* to, struct ast* tl);


#endif // NODE_INCLUDED
