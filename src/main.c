#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>


static FILE *fp;
static char *text;
static char *data;


static uint8_t offsetCount = 0;
static uint8_t branchCount = 0;
static uint8_t litstrCount = 3;
static uint8_t symCount = 0;


#include "node.c"
#include "asmgen.c"

int errorflag;

// print error
void yyerror (char *s, ...) {
  va_list ap;
  va_start(ap, s);

  fprintf(stderr, "line: %d, error: ", yylineno);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}

int main(int argc, char **argv) {
  extern FILE *yyin;
  char *asmFile;
  char *ptr;
  char buf[21];

  if (argc > 1) {
      if (!(yyin = fopen(argv[1], "r"))) {
          perror(argv[1]);
          return 1;
      }
  }
  else {
      printf("\nPlease specify a source file.\n\n");
      return 1;
  }

  text = (char *) malloc(sizeof(char));
  data = (char *) malloc(sizeof(char));
  *(text) = '\0';
  *(data) = '\0';

  // create new file
  asmFile = strdup(argv[1]);
  ptr = strchr(asmFile, '.');

  *(ptr+1) = 's';
  *(ptr+2) = '\0';

  yyparse();
  if (errorflag) {
    printf("\nParsing Error\n");
    return -1;
  }

  fp = newFile(asmFile);

  // print symbol count to file
  sprintf(buf, "$%u", (symCount + 1) * 8);
  text = genText(sub(buf, "%rsp"), text);
  // put Bss, Data, Text section to ASM code
  putBssSec(fp, "");
  putDataSec(fp, data);
  putTextSec(fp, text);

  // close file
  closeFile(fp);

  return 0;
}
