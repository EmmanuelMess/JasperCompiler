%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.5.1"
%defines

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
#include <string>
#include <llvm/IR/IRBuilder.h>
#include "jasper_number.hpp"
#include "jasper_function.hpp"
  class driver;
}

// The parsing context.
%param { driver& drv }

%locations

%define parse.trace
%define parse.error verbose

%code {
#include "driver.hpp"

std::string convert(std::string s);
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
  LBRACE  "{"
  RBRACE  "}"
  STATEMENT_END ";"
  FUNCTION_START "fn"
  RETURN_STATEMENT "return"
  FUNCTION_IMMEDIATE  "=>"
  INVOKE_ID "__invoke"
;

%token <std::string> IDENTIFIER "identifier"
%token <std::string> STRING "string"
%nterm <llvm::Value*> string_expression
%token <float> FLOAT_NUMBER "float_number"
%token <unsigned int> INTEGER_NUMBER "integer_number"
%nterm <JasperNumber> exp
%nterm <JasperFunction> function
%nterm <std::vector<std::string>> function_arguments

//%printer { yyo << $$; } <*>;

%%
%start unit;
unit: assignments { };

assignments : %empty                 {}
            | assignments assignment {};

assignment : "identifier" ":=" exp ";"               { drv.variables[$1] = $3; } // TODO have variable names
           | "identifier" ":=" string_expression ";" { drv.string_variables[$1] = $3; } // TODO have variable names
           | "identifier" ":=" function ";"          {
                                                        llvm::FunctionType *functionType = llvm::FunctionType::get($3.returnType, $3.argumentTypes, false);
                                                        llvm::Function *function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, $1, drv.module.get());

                                                        auto it = $3.argumentNames.begin();
                                                        for (auto &arg : function->args()) {
                                                            arg.setName(*it);
                                                            ++it;
                                                        }
                                                     }
           | "__invoke" ":=" function ";"            { llvm::Function *function = drv.module->getFunction("__invoke"); }

%left "+" "-";
%left "*" "/";
exp : "integer_number"  { $$ = JasperNumber { &drv.context, llvm::ConstantInt::get(drv.context, { 16, $1, true }) }; }
    | "float_number"    { $$ = JasperNumber { &drv.context, llvm::ConstantFP::get(drv.context, llvm::APFloat($1)) }; }
    | "identifier"      { $$ = drv.variables[$1]; } // TODO fix
    | exp "+" exp       { $$ = $1 + $3; }
    | exp "-" exp       { $$ = $1 - $3; }
    | exp "*" exp       { $$ = $1 * $3; }
    | exp "/" exp       { $$ = $1 / $3; }
    | "(" exp ")"       { $$ = $2; }

string_expression : "string"                                  {
                                                                //TODO $$ = $1;//drv.builder.CreateGlobalStringPtr($1);
                                                              }
                  | "identifier"                              { $$ = drv.string_variables[$1]; }
                  | string_expression "+" string_expression   {
                                                                //TODO $$ = $1;//TODO
                                                              }
                  | "(" string_expression ")"                 { $$ = $2; }

function : "fn" "(" function_arguments ")" function_body      { $$ = JasperFunction { .returnType = llvm::Type::getVoidTy(drv.context) }; }
         | "fn" "(" function_arguments ")" "=>" exp           { $$ = JasperFunction { .returnType = llvm::Type::getVoidTy(drv.context) }; }

function_arguments : %empty                                   { $$ = std::vector<std::string>(); }
                   | function_arguments "identifier"          { $1.emplace_back($2); $$ = $1; }

function_body : "{" assignments "return" exp ";" "}"

%%

void yy::parser::error (const location_type& l, const std::string& m) {
  std::cerr << l << ": " << m << '\n';
}

/*
llvm::Function *function = drv.module->getFunction($1);
llvm::BasicBlock *block = llvm::BasicBlock::Create(drv.context, "entry", $3);
Builder.SetInsertPoint(block);
functionVariables.emplace_back();
for (auto &arg : function->args()) {
    functionVariables.back()[arg.getName()] = &arg;
}
*/