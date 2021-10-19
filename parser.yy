%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.5.1"
%defines

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
#include <string>
#include "jasper_number.hpp"
  class driver;
}

// The parsing context.
%param { driver& drv }

%locations

%define parse.trace
%define parse.error verbose

%code {
#include "driver.hpp"
}

%define api.token.prefix {TOK_}
%token
  END  0  "end of file"
  ASSIGN  ":="
  MINUS   "-"
  PLUS    "+"
  STAR    "*"
  SLASH   "/"
  LPAREN  "("
  RPAREN  ")"
  STATEMENT_END ";"
;

%token <std::string> IDENTIFIER "identifier"
%token <JasperNumber> NUMBER "number"
%nterm <JasperNumber> exp

%printer { yyo << $$; } <*>;

%%
%start unit;
unit: assignments { };

assignments : %empty                 {}
            | assignments assignment {};

assignment: "identifier" ":=" exp ";" { drv.variables[$1] = $3; };

%left "+" "-";
%left "*" "/";
exp : "number"
    | "identifier"  { $$ = drv.variables[$1]; }
    | exp "+" exp   { $$ = $1 + $3; }
    | exp "-" exp   { $$ = $1 - $3; }
    | exp "*" exp   { $$ = $1 * $3; }
    | exp "/" exp   { $$ = $1 / $3; }
    | "(" exp ")"   { $$ = $2; }
%%

void yy::parser::error (const location_type& l, const std::string& m) {
  std::cerr << l << ": " << m << '\n';
}
