#include <stdio.h>

#include "node.h"
#include "asmutil.h"
#include "asmgen.h"


// create new node in ast
struct ast* newNode (struct ast* l, struct ast* r, int nodetype) {
  struct ast* tmp = (struct ast*) malloc(sizeof(struct ast));

  if (!tmp) {
    yyerror("out of space");
    exit(0);
  }

  tmp->nodetype = nodetype;
  tmp->l = l;
  tmp->r = r;

  return tmp;
}

// create new constant number
struct ast* newNum (int64_t num) {
    struct numval* tmp = (struct numval*) malloc(sizeof(struct numval));

    if (!tmp) {
        yyerror("out of space");
        exit(0);
    }

    tmp->nodetype = 'N';
    tmp->number = num;

    return (struct ast*) tmp;
}

// create new print node
struct ast* newPrintStmt (struct ast *exp, char *str, int nodetype) {
  struct print *tmp = (struct print*) malloc(sizeof(struct print));
  struct lString *ls = (struct lString*) malloc(sizeof(struct lString));

  if (!tmp) {
    yyerror("out of space");
    exit(0);
  }

  tmp->nodetype = nodetype;

  // if we desire to create print decimal or hex
  if (nodetype == 'D' || nodetype == 'H') {
    tmp->arg.exp = exp;
  } else {
    // else we wants to create print string
    char buf[21];

    // create new string and put into data section
    sprintf(buf, "LC%u", litstrCount);
    data = genData(data, newString(buf, str));

    ls->str = str;
    ls->label = litstrCount++;
    tmp->arg.ls = ls;
  }

  return (struct ast*) tmp;
}

// create new variable node
struct ast* newVar (struct symbol *s) {

  // allocate in hash table
  struct symref *tmp = (struct symref*) malloc(sizeof(struct symref));

  if (!tmp) {
    yyerror("out of space");
    exit(0);
  }

  tmp->nodetype = 'V';
  tmp->s = s;

  return (struct ast*) tmp;
}

// create new variable node
struct ast* newVarArray (struct symbol *s, struct ast* v) {

  // allocate in hash table
  struct symarrayref *tmp = (struct symarrayref*) malloc(sizeof(struct symarrayref));

  if (!tmp) {
    yyerror("out of space");
    exit(0);
  }

  tmp->nodetype = 'W';
  tmp->s = s;
  tmp->v = v;

  return (struct ast*) tmp;
}


// create new variable assignment node
struct ast* newArray (struct symbol *s, uint8_t size) {
  struct symarray *tmp = (struct symarray*) malloc(sizeof(struct symarray));

  if (!tmp) {
    yyerror("out of space");
    exit(0);
  }

  tmp->nodetype = 'A';
  tmp->s = s;
  tmp->size = size;
  return (struct ast*) tmp;
}


// create new variable assignment node
struct ast* newDeclar (struct symbol *s, struct ast* v, uint8_t is_const) {
  struct symasgn *tmp = (struct symasgn*) malloc(sizeof(struct symasgn));

  if (!tmp) {
    yyerror("out of space");
    exit(0);
  }

  tmp->nodetype = 'J';
  tmp->s = s;
  if (v) {
    tmp->v = v;
  } else {
    tmp->v = newNum(0);
  }
  tmp->is_const = is_const;

  return (struct ast*) tmp;
}

// create new variable assignment node
struct ast* newAssign (struct symbol *s, struct ast* v) {
  struct symasgn *tmp = (struct symasgn*) malloc(sizeof(struct symasgn));

  if (!tmp) {
    yyerror("out of space");
    exit(0);
  }

  tmp->nodetype = '=';
  tmp->s = s;
  tmp->v = v;

  return (struct ast*) tmp;
}

// create new variable assignment node
struct ast* newArrayAssign (struct symbol *s, struct ast* v, struct ast* offset) {
  struct symarray *tmp = (struct symarray*) malloc(sizeof(struct symarray));

  if (!tmp) {
    yyerror("out of space");
    exit(0);
  }

  tmp->nodetype = 'G';
  tmp->s = s;
  tmp->v = v;
  tmp->offset = offset;

  return (struct ast*) tmp;
}

// create new condition node
struct ast* newIfe (struct ast* firstStmt, struct ast* secStmt, struct ast* tl, struct ast* tr, char op) {
  struct cond *tmp = (struct cond*) malloc(sizeof(struct cond));

  if (!tmp) {
    yyerror("out of space");
    exit(0);
  }

  tmp->nodetype = 'C';
  tmp->fStmt = firstStmt;
  tmp->sStmt = secStmt;
  tmp->tl = tl;
  tmp->tr = tr;
  tmp->op = op;

  return (struct ast*) tmp;
}

// create new loop node
struct ast* newVon (struct ast* from, struct ast* to, struct ast* tl) {
  struct loop* tmp = (struct loop*) malloc(sizeof(struct loop));

  if (!tmp) {
    yyerror("out of space");
    exit(0);
  }

  tmp->nodetype = 'L';
  tmp->from = from;
  tmp->to = to;
  tmp->tl = tl;

  return (struct ast*) tmp;
}

// free a tree of ASTs 
void freeNode (struct ast* node) {
  switch (node->nodetype) {
    // node with two subrtrees
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case 'B':
      freeNode(node->r);

    // node with one subtree
    case '^':
      freeNode(node->l);

    // node with no subtree
    case 'N':
    case 'V':
    case 'W':
      break;

    // for print node, we need to free expression in
    // command too
    case 'D':
    case 'H':
      freeNode(((struct print*)node)->arg.exp);
      break;

    // for print string node, we need to free string
    case 'S':
      free(((struct print*)node)->arg.ls->str);
      free(((struct print*)node)->arg.ls);
      break;

    // assignment node
    case '=':
      freeNode(((struct symasgn*)node)->v);
      break;

    case 'G':
      freeNode(((struct symarray*)node)->v);
      freeNode(((struct symarray*)node)->offset);
      break;

    // assignment node
    case 'J':
      freeNode(((struct symasgn*)node)->v);
      break;

    // assignment node
    case 'A':
      break;

    // condition node
    case 'C':
      freeNode(((struct cond*)node)->fStmt);
      freeNode(((struct cond*)node)->sStmt);

      if (((struct cond*)node)->tl)
        freeNode(((struct cond*)node)->tl);
      if (((struct cond*)node)->tr)
        freeNode(((struct cond*)node)->tr);

      break;

    // loop node
    case 'L':
      freeNode(((struct loop*)node)->from);
      freeNode(((struct loop*)node)->to);

      if (((struct loop*)node)->tl)
        freeNode(((struct loop*)node)->tl);

      break;

    default:
      printf("internal error: free bad node %c\n", node->nodetype);
  }

  // finally, free node itself 
  free(node);
}
