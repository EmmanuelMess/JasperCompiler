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
  class Driver;
}

// The parsing context.
%param { Driver& driver }

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

assignment : "identifier" ":=" exp ";"               { driver.variables[$1] = $3; } // TODO have variable names
           | "identifier" ":=" string_expression ";" { driver.string_variables[$1] = $3; } // TODO have variable names
           | "identifier" ":=" function ";"          {
                                                        llvm::FunctionType *functionType = llvm::FunctionType::get($3.returnType, $3.argumentTypes, false);
                                                        llvm::Function *function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, $1, driver.module.get());

                                                        auto it = $3.argumentNames.begin();
                                                        for (auto &arg : function->args()) {
                                                            arg.setName(*it);
                                                            ++it;
                                                        }
                                                     }

%left "+" "-";
%left "*" "/";
exp : "integer_number"  { $$ = JasperNumber { &driver.context, llvm::ConstantInt::get(driver.context, { 16, $1, true }) }; }
    | "float_number"    { $$ = JasperNumber { &driver.context, llvm::ConstantFP::get(driver.context, llvm::APFloat($1)) }; }
    | "identifier"      { $$ = driver.variables[$1]; } // TODO fix
    | exp "+" exp       { $$ = $1 + $3; }
    | exp "-" exp       { $$ = $1 - $3; }
    | exp "*" exp       { $$ = $1 * $3; }
    | exp "/" exp       { $$ = $1 / $3; }
    | "(" exp ")"       { $$ = $2; }

string_expression : "string"                                  {
                                                                //TODO $$ = $1;//driver.builder.CreateGlobalStringPtr($1);
                                                              }
                  | "identifier"                              { $$ = driver.string_variables[$1]; }
                  | string_expression "+" string_expression   {
                                                                //TODO $$ = $1;//TODO
                                                              }
                  | "(" string_expression ")"                 { $$ = $2; }

function : "fn" "(" function_arguments ")" function_body      { $$ = JasperFunction { .returnType = llvm::Type::getVoidTy(driver.context) }; }
         | "fn" "(" function_arguments ")" "=>" exp           { $$ = JasperFunction { .returnType = llvm::Type::getVoidTy(driver.context) }; }

function_arguments : %empty                                   { $$ = std::vector<std::string>(); }
                   | function_arguments "identifier"          { $1.emplace_back($2); $$ = $1; }

function_body : "{" assignments "return" exp ";" "}"

%%

void yy::parser::error (const location_type& l, const std::string& m) {
  std::cerr << l << ": " << m << '\n';
}

/*
llvm::Function *function = driver.module->getFunction($1);
llvm::BasicBlock *block = llvm::BasicBlock::Create(driver.context, "entry", $3);
Builder.SetInsertPoint(block);
functionVariables.emplace_back();
for (auto &arg : function->args()) {
    functionVariables.back()[arg.getName()] = &arg;
}
*/