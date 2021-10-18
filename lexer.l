%{
#include "parser.h"
void yyerror (char *s);
int yylex();
%}
%%
"print"				   {return print;}
"exit"				   {return exit_command;}
"fn"                   {return function;}
[a-zA-Z]+			   { 
                            yylval.id = calloc(yyleng + 1, sizeof(char));
                            strncpy(yylval.id, yytext, yyleng); 
                            return identifier;
                       }
[0-9]+                 {yylval.num = atoi(yytext); return number;}
[ \t\n]                ;
":="                   {return assign;}
[-+*/;]           	   {return yytext[0];}
.                      {ECHO; yyerror ("unexpected character");}

%%
int yywrap (void) {return 1;}

