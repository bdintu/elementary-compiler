D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*


%{
#include "simple.tab.h"
#include "string.h"

void comment(); 
%}

%x SINGLE_STRING

%%
"const"             {return T_CONST;}
"del"               {return T_DEL;}
"else"              {return T_ELSE;}
"exit"              {return T_EXIT;}
"if"                {return T_IF;}
"print"             {return T_PRINT;}
"var"               {return T_VAR;}
"while"             {return T_WHILE;}

"hex("              {return T_HEX;}

"=="                {return T_EQ;}
"!="                {return T_NE;}
">="                {return T_GT;}
"<="                {return T_LT;}

"="                 {return T_ASSIGN;}
"+"                 {return T_ADD;}
"-"                 {return T_SUB;}
"*"                 {return T_MUL;}
"/"                 {return T_DIV;}
"%"                 {return T_MOD;}
"("                 {return T_LEFTPAREN;}
")"                 {return T_RIGHTPAREN;}
"{"                 {return T_OPENBRACE;}
"}"                 {return T_CLOSEBRACE;}
"["                 {return T_LEFTSQUARE;}
"]"                 {return T_RIGHTSQUARE;}
"\n"                {return T_NEWLINE;}
","                 {return T_COMMA;}
";"                 {return T_SEMI;}

{L}({L}|{D})*       {yylval.id = strdup(yytext); return identifier;}
[0-9]+              {yylval.num = atol(yytext); return T_INT;}
0x{H}+              {yylval.num = (int)strtol(yytext, NULL, 0); return T_INT;}
[-+*/=><;(){}[]%]         {return yytext[0];}




\"                          BEGIN(SINGLE_STRING);
<SINGLE_STRING>{
  ([^\\\"]|\\.)*            {yylval.num = strdup(yytext); return(T_STR);}
  \"                        BEGIN(INITIAL);
}


[ \t\v\f]           {}
"//"[^\n]*          {}
"/*"                {comment();}
%%


int yywrap() {
	return(1);
}


void comment() {
	char c, c1;

loop:
	while ((c = input()) != '*' && c != 0) {}

	if ((c1 = input()) != '/' && c != 0) {
		unput(c1);
		goto loop;
	}
}
