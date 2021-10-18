%{
void yyerror (char *s);
int yylex();
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>

struct variable {
    char *name;
    int type;
    union {
        int integer;
        char *string;
        float floating;
    };
};

struct variable_list {
    struct variable *start;
    struct variable *end;
    struct variable *empty;
};

struct variable_list symbols = { .start = NULL, .end = NULL, .empty = NULL };
int symbolVal(char *name);
void updateSymbolVal(char *name, int val);
%}

%union {int num; char *id;}         /* Yacc definitions */
%start line
%token print
%token exit_command
%token function
%token assign
%token <num> number
%token <id> identifier
%type <num> line exp term 
%type <id> assignment

%%

/* descriptions of expected inputs     corresponding actions (in C) */

line    : command		{;}
        | line command 	{;}
        ;

command : assignment ';'          		{;}
        | exit_command ';'        		{exit(EXIT_SUCCESS);}
        | print exp ';'           		{printf("Printing %d\n", $2);}

assignment : identifier assign exp  { updateSymbolVal($1,$3); }
            ;
exp        : term                	{$$ = $1;}
           | exp '+' term        	{$$ = $1 + $3;}
           | exp '-' term        	{$$ = $1 - $3;}
           | exp '*' term        	{$$ = $1 * $3;}
           | exp '/' term        	{$$ = $1 / $3;}
           | function_definition	{;}
           ;

function_definition : function '(' ')' '{' line '}' 						{;}
					| function '(' identifier_list ')' '{' line '}' 		{;}

identifier_list : identifier						{;}
				| identifier_list ',' identifier	{;}


term       : number                {$$ = $1;}
           | identifier            {$$ = symbolVal($1); free($1); $1 = NULL; }
           ;

%%

struct variable* computeVariable(char *name) {
    for(struct variable *i = symbols.start; i < symbols.empty; i++) {
        if(strcmp(i->name, name) == 0) {
            return i;
        }
    }
    
    return NULL;
} 

/* returns the value of a given symbol */
int symbolVal(char *name) {
    struct variable *var = computeVariable(name);
    
    if(var == NULL) {
        yyerror ("No such var!");
    }

    return var->integer;
}

/* updates the value of a given symbol */
void updateSymbolVal(char *name, int val) {
    struct variable* var = computeVariable(name);

    if(var == NULL) {
        if(symbols.empty == symbols.end) {
            size_t oldSize = symbols.end - symbols.start;
            symbols.start = reallocarray(symbols.start, oldSize * 2, sizeof(struct variable));
            symbols.empty = symbols.start + oldSize;
            symbols.end = symbols.start + oldSize * 2;
        }
        var = symbols.empty;
        symbols.empty++;
    }

    var->name = name;
    var->integer = val;
}

int main() {
    /* init symbol table */
    symbols.start = reallocarray(symbols.start, 2, sizeof(struct variable));
    symbols.empty = symbols.start;
    symbols.end = symbols.start + 2;
    return yyparse ( );
}

void yyerror (char *s) {
    fprintf (stderr, "%s\n", s);
}
