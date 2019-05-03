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

"=="          { return (T_EQ); }
"!="          { return (T_NE); }
">="          { return (T_GE); }
"<="          { return (T_LE); }
">"          { return (T_GT); }
"<"          { return (T_LT); }
"="              { return (T_ASSIGN); }
"print"          { return (T_PRINT); }
"hex"          { return (T_HEX); }
"const"          { return (T_CONST); }
"var"          { return (T_VAR); }
"if"             { return (T_IF); }
"for"             { return (T_FOR); }

[-]*[0-9]+           { yylval.num = (int64_t)atol(yytext); return (NUM); }
0[xX][0-9a-fA-F]+  { yylval.num = (int64_t)strtol(yytext, NULL, 0); return (NUM);}
[a-z]{2}        { yylval.sym = lookup(yytext, 0, 0); return (VAR); }
["].*["]          { yylval.str = strdup(yytext); return (TEXT); }

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
