%option noyywrap nodefault yylineno

%{
  #include "asmgen.h"
  #include "node.h"
  #include "parser.tab.h"
  #include <stdio.h>
  #include <stdlib.h>
  #include <math.h>
  #include <ctype.h>
  #include <string.h>

  extern int errorflag;
%}

%%
[ \t\v\f]           {}
"//"[^\n]*          {}
"/*"                {comment();}
[-]*[0-9]+           { yylval.num = (int64_t)atol(yytext); return (NUM); }
0[xX][0-9a-fA-F]+  { yylval.num = (int64_t)strtol(yytext, NULL, 0); return (NUM);}
[A-Z][A-Z]     { yylval.sym = lookup(yytext, 0, 0); return (VAR); }
["].*["]          { yylval.str = strdup(yytext); return (TEXT); }
"="              { return (TK_ASSIGN); }
"(" |
")" |
"[" |
"]" |
"{" |
"}" |
"+" |
"-" |
"*" |
"/" |
"%" |
"^" |
":" |
";" |
","               { return (yytext[0]); }
"=="          { return (TK_EQ); }
"!="          { return (TK_NE); }
">="          { return (TK_GE); }
"<="          { return (TK_LE); }
">"          { return (TK_GT); }
"<"          { return (TK_LT); }
"print"          { return (TK_PRINT); }
"hex"          { return (TK_HEX); }
"const"          { return (TK_CONST); }
"var"          { return (TK_VAR); }
"if"             { return (TK_IFE); }
"for"             { return (TK_VON); }
\n                {  }
.                 { errorflag = 1; yyerror("Mystery character %c\n", *yytext); }
%%

void comment() {
	char c, c1;

loop:
	while ((c = input()) != '*' && c != 0) {}

	if ((c1 = input()) != '/' && c != 0) {
		unput(c1);
		goto loop;
	}
}
