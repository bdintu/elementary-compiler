#include <stdio.h>

#include "node.h"
#include "asmgen.h"
#include "asmutil.h"

extern FILE *fp;
extern char *text;
extern char *data;

extern uint8_t offsetCount;
extern uint8_t branchCount;
extern uint8_t litstrCount;
extern uint8_t symCount;


/* symbol table */
/* hash a symbol */
static unsigned symhash (char* sym) {
  unsigned int hash = 0;
  unsigned c;
  
  while ((c = (*(sym++)))) 
    hash = hash * 7 ^ c;

  return hash;
}

struct symbol* lookup (char* sym, uint8_t size, uint8_t is_const) {
  struct symbol* sp = &symtab[symhash(sym) % SEED];
  int scount = SEED;

  while (--scount >= 0) {
    if (sp->name && !strcmp(sp->name, sym)) {
      if (sp->is_const == 0 && is_const == 1) {
        sp->is_const = 1;
      }
      return sp;
    }

    // new entry
    if (!sp->name) {    
      sp->name = strdup(sym);
      sp->offset = 0;
      sp->size = size;
      sp->is_const = is_const;

      ++symCount;
      return sp;
    }

    // if hash collition happens
    if (++sp >= symtab + SEED){
      sp = symtab;   
    }
  }

  // if hash table is full
  yyerror("symbol table overflow\n");
  abort();
}



// evaluate number or statement
int64_t eval (struct ast* node) {
  int64_t v;

  if (!node) {
      yyerror("internal error, null eval");

      return 0;
  }

  switch (node->nodetype) {
    // constant
    case 'N':
      v = ((struct numval*)node)->number;
      break;

    // expressions
    case '+':
      v = eval(node->l) + eval(node->r);
      break;
    case '-':
      v = eval(node->l) - eval(node->r);
      break;
    case '*':
      v = eval(node->l) * eval(node->r);
      break;
    case '/':
      v = eval(node->l) / eval(node->r);
      break;
    case '%':
      v = eval(node->l) % eval(node->r);
      break;
    case '^':
      v = -eval(node->l);
      break;

    default:
      printf("internal error: bad node %c\n", node->nodetype);
  }

  return v;
}

// generate ASM code
void asmGen (struct ast* node) {
  struct symbol *s;
  char buf[21], buf2[21], tmpbuf[21];
  uint8_t condBranchNum, loopBranchNum;

  if (!node) {
    yyerror("internal error, null node");
    return;
  }

  switch (node->nodetype) {
    // constant number
    case 'N':
      text = genText(text, genComment("number"));
      text = genText(text, sub("$8", "%rsp"));
      sprintf(buf, "$%ld", ((struct numval*)node)->number);
      text = genText(text, movq(buf, "%rax"));
      text = genText(text, movq("%rax", "(%rsp)"));
      break;
      
    // variable reference
    case 'V':
      s = lookup(((struct symasgn*)node)->s->name, 0, ((struct symasgn*)node)->is_const);

      if (s->offset == 0) {
        yyerror("%s not define", s->name);
        exit(0);
      }

      text = genText(text, genComment("reference variable"));
      sprintf(buf, "-%u(%%rbp)", s->offset);
      text = genText(text, movq(buf, "%rax"));
      text = genText(text, sub("$8", "%rsp"));
      text = genText(text, movq("%rax", "(%rsp)"));

      break;
      
    // variable reference
    case 'W':
      s = lookup(((struct symarray*)node)->s->name, ((struct symarray*)node)->size, 0);

      if (s->offset == 0) {
        yyerror("%s not define", s->name);
        exit(0);
      }

      asmGen(((struct symarray*)node)->v);

      text = genText(text, genComment("reference array"));
      sprintf(buf, "-%u(%%rbp, %rax, 8)", s->offset);
      text = genText(text, movq(buf, "%rax"));
      text = genText(text, sub("$8", "%rsp"));
      text = genText(text, movq("%rax", "(%rsp)"));

      break;

    // print decimal
    case 'D':
      asmGen(((struct print*)node)->arg.exp);
      
      text = genText(text, genComment("print decimal"));
      text = genText(text, movq("(%rsp)", "%rax"));
      text = genText(text, add("$8", "%rsp"));
      text = genText(text, movq("%rax", "%rsi"));
      text = genText(text, movl("$.LC0", "%edi"));
      text = genText(text, movl("$0", "%eax"));
      text = genText(text, syscallPrint());

      break;

    // print hexadecimal
    case 'H':
      asmGen(((struct print*)node)->arg.exp);

      text = genText(text, genComment("print hexadecimal"));
      text = genText(text, movq("(%rsp)", "%rax"));
      text = genText(text, add("$8", "%rsp"));
      text = genText(text, movq("%rax", "%rsi"));
      text = genText(text, movl("$.LC1", "%edi"));
      text = genText(text, movl("$0", "%eax"));
      text = genText(text, syscallPrint());

      break;
        
    // print string
    case 'S':
      sprintf(buf, "$.LC%u", ((struct print*)node)->arg.ls->label);

      text = genText(text, genComment("print string"));
      text = genText(text, movl(buf, "%esi"));
      text = genText(text, movl("$.LC2", "%edi"));
      text = genText(text, movl("$0", "%eax"));
      text = genText(text, syscallPrint());

      break;

    // variable assignment
    case 'G':
      s = lookup(((struct symarray*)node)->s->name, 0, 0);

      if (s->is_const) {
        yyerror("const can not assign");
        exit(0);
      }

      if (s->offset == 0) {
        yyerror("var not declared");
        exit(0);
      }

//      text = genText(text, movq("fffff", "fffff"));
      asmGen(((struct symarray*)node)->v);
//      text = genText(text, movq("fffff", "fffff"));
      asmGen(((struct symarray*)node)->offset);
//      text = genText(text, movq("fffff", "fffff"));



      text = genText(text, genComment("assign array"));
      text = genText(text, movq("(%rsp)", "%rdi"));
      text = genText(text, add("$8", "%rsp"));
      text = genText(text, movq("(%rsp)", "%rax"));

      sprintf(buf, "-%u(%%rbp, %rdi, 8)", s->offset);
      text = genText(text, movq("%rax", buf));


      break;

    // variable assignment
    case '=':
      s = lookup(((struct symasgn*)node)->s->name, 0, ((struct symasgn*)node)->is_const);

      if (s->is_const) {
        yyerror("const can not assign");
        exit(0);
      }

      if (s->offset == 0) {
        yyerror("var not declared");
        exit(0);
      }

      asmGen(((struct symasgn*)node)->v);

      text = genText(text, genComment("assign variable"));
      text = genText(text, movq("(%rsp)", "%rax"));
      text = genText(text, add("$8", "%rsp"));
      sprintf(buf, "-%u(%%rbp)", s->offset);
      text = genText(text, movq("%rax", buf));
      text = genText(text, sub("$8", "%rsp"));
      text = genText(text, movq("%rax", "(%rsp)"));

      break;
        
    // variable assignment
    case 'J':
      s = lookup(((struct symasgn*)node)->s->name, 0, ((struct symasgn*)node)->is_const);

      if (s->is_const == 1) {
        // 2 is mean variable declared
        s->is_const = 2;
      } else if (s->is_const == 2) {
        yyerror("const not assign");
        exit(0);
      }

      // if assign to new variable, find an offset for it
      if (s->offset == 0) {
        s->offset = (++offsetCount) * 8;
      } else {
        yyerror("var is declared");
        exit(0);
      }

      asmGen(((struct symasgn*)node)->v);

      text = genText(text, genComment("declared variable"));
      sprintf(buf, "-%u(%%rbp)", s->offset);
      text = genText(text, movq("%rax", buf));

      break;
        
    // variable assignment
    case 'A':
      s = lookup(((struct symarray*)node)->s->name, 0, 0);

      if (s->is_const == 1) {
        // 2 is mean variable declared
        s->is_const = 2;
      } else if (s->is_const == 2) {
        yyerror("const not assign");
        exit(0);
      }

      // if assign to new variable, find an offset for it
      if (s->offset == 0) {
        offsetCount += ((struct symarray*)node)->size;
        s->offset = 8*offsetCount;
        symCount += ((struct symarray*)node)->size -1;
      } else {
        yyerror("var is declared");
        exit(0);
      }

      break;

    // expressions
    case '+':
      asmGen(node->r);
      asmGen(node->l);

      text = genText(text, genComment("expression add"));
      text = genText(text, movq("(%rsp)", "%rdx"));
      text = genText(text, add("$8", "%rsp"));
      text = genText(text, movq("(%rsp)", "%rax"));
      text = genText(text, add("$8", "%rsp"));
      text = genText(text, add("%rdx", "%rax"));
      text = genText(text, sub("$8", "%rsp"));
      text = genText(text, movq("%rax", "(%rsp)"));

      break;

    case '-':
      asmGen(node->r);
      asmGen(node->l);

      text = genText(text, genComment("expression sub"));
      text = genText(text, movq("(%rsp)", "%rax"));
      text = genText(text, add("$8", "%rsp"));
      text = genText(text, sub("(%rsp)", "%rax"));
      text = genText(text, movq("%rax", "(%rsp)"));

      break;

    case '*':
      asmGen(node->r);
      asmGen(node->l);

      text = genText(text, genComment("expression plus"));
      text = genText(text, movq("(%rsp)", "%rax"));
      text = genText(text, add("$8", "%rsp"));
      text = genText(text, imul("(%rsp)", "%rax"));
      text = genText(text, movq("%rax", "(%rsp)"));

      break;
      
    case '/':
      asmGen(node->r);
      asmGen(node->l);

      text = genText(text, genComment("expression div"));
      text = genText(text, movq("(%rsp)", "%rax"));
      text = genText(text, add("$8", "%rsp"));
      text = genText(text, cqto());
      text = genText(text, idiv("(%rsp)"));
      text = genText(text, movq("%rax", "(%rsp)"));

      break;

    case '%':
      asmGen(node->r);
      asmGen(node->l);

      text = genText(text, genComment("expression ,mod"));
      text = genText(text, movq("(%rsp)", "%rax"));
      text = genText(text, add("$8", "%rsp"));
      text = genText(text, cqto());
      text = genText(text, idiv("(%rsp)"));
      text = genText(text, movq("%rdx", "(%rsp)"));

      break;

    case '^':
      asmGen(node->l);

      text = genText(text, genComment("expression neg"));
      text = genText(text, movq("(%rsp)", "%rax"));
      text = genText(text, neg("%rax"));
      text = genText(text, movq("%rax", "(%rsp)"));

      break;

    // condition
    case 'C':
      // need number to assign to label for jumping
      condBranchNum = branchCount++;

      asmGen(((struct cond*)node)->sStmt);
      asmGen(((struct cond*)node)->fStmt);

      text = genText(text, genComment("condition"));
      text = genText(text, movq("(%rsp)", "%rax"));
      text = genText(text, add("$8", "%rsp"));
      text = genText(text, sub("(%rsp)", "%rax"));
      text = genText(text, movq("%rax", "(%rsp)"));
      
      
      text = genText(text, cmp("$0", "(%rsp)"));
      sprintf(buf, "L%u", condBranchNum);

      char op = ((struct cond*)node)->op;
      switch (op) {
          case 'e':
            text = genText(text, newJump("jne", buf));
            break;

          case 'n':
            text = genText(text, newJump("je", buf));
            break;

          case 'g':
            text = genText(text, newJump("jl", buf));
            break;

          case 'l':
            text = genText(text, newJump("jg", buf));
            break;

          case 'h':
            text = genText(text, newJump("jle", buf));
            break;

          case 'm':
            text = genText(text, newJump("jge", buf));
            break;
      }

      if (((struct cond*)node)->tl) {
          asmGen(((struct cond*)node)->tl);
          sprintf(buf, "L%u", condBranchNum+1);
          text = genText(text, newJump("jmp", buf));
      }

      sprintf(buf, "L%u", condBranchNum);
      text = genText(text, newLabel(buf));

      if (((struct cond*)node)->tr)
          asmGen(((struct cond*)node)->tr);

      ++condBranchNum;
      sprintf(buf, "L%u", condBranchNum);
      text = genText(text, newLabel(buf));

      text = genText(text, add("$8", "%rsp"));

      break;

    // loop
    case 'L':
      if (((struct loop*)node)->tl) {

        // need number to assign to label for jumping
        loopBranchNum = branchCount; 
        branchCount += 2;
        
        // keep iteration number in %rcx
        text = genText(text, genComment("loop"));
        text = genText(text, push("%rcx"));
        
        // if argument 'from' is variable we need to resolve it first
        if(((struct loop*)node)->from->nodetype == 'V') {
          struct symasgn* tmp = (struct stmasgn*)(((struct loop*)node)->from);
          s = lookup(tmp->s->name, 0, tmp->is_const);
          sprintf(buf, "-%u(%%rbp)", s->offset);
        } else {
          // eles just put a number
          sprintf(buf, "$%ld", eval(((struct loop*)node)->from));
        }
        
        // use jump label as variable to store iteration count
        sprintf(tmpbuf, "L%u", loopBranchNum);
        s = lookup(tmpbuf, 0, 0);
        if (s->offset == 0) {
          s->offset = (++offsetCount) * 8;
        }
        sprintf(tmpbuf, "-%u(%%rbp)", s->offset);
        
        text = genText(text, (movq(buf, "%rcx")));
        text = genText(text, (movq("%rcx", tmpbuf)));
        sprintf(buf, "L%u", loopBranchNum);
        text = genText(text, newJump("jmp", buf));
        sprintf(buf, "L%u", loopBranchNum + 1);
        text = genText(text, newLabel(buf));

        // evaluate loop body
        asmGen(((struct loop*)node)->tl);
        text = genText(text, movq(tmpbuf, "%rcx"));
        text = genText(text, add("$1", "%rcx"));
        text = genText(text, movq("%rcx", tmpbuf));
        // if argument 'from' is variable, update it as well
        if(((struct loop*)node)->from->nodetype == 'V') {
          struct symasgn* tmp = (struct stmasgn*)(((struct loop*)node)->from);
          s = lookup(tmp->s->name, 0, tmp->is_const);
          sprintf(buf, "-%u(%%rbp)", s->offset);
          text = genText(text, add("$1", buf));
        }
        
        sprintf(buf, "L%u", loopBranchNum);
        text = genText(text, newLabel(buf));

        // if argument 'to' is variable, we need to resolve it too
        if(((struct loop*)node)->to->nodetype == 'V') {
          struct symasgn* tmp = (struct stmasgn*)(((struct loop*)node)->to);
          s = lookup(tmp->s->name, 0, 0);
          sprintf(buf, "-%u(%%rbp)", s->offset);
        } else {
          sprintf(buf, "$%ld", eval(((struct loop*)node)->to));
        }
        text = genText(text, cmp(buf, "%rcx"));
        sprintf(buf, "L%u", loopBranchNum + 1);
        text = genText(text, newJump("jl", buf)); 
        text = genText(text, pop("%rcx"));
      }

      break;

    // block statements
    case 'B':
      asmGen(node->l);
      asmGen(node->r);
      break;

    default:
      printf("internal error: bad node %c\n", node->nodetype);
  }

}
