#ifndef DRIVER_HEADER
#define DRIVER_HEADER
#include <string>
#include <map>
#include <memory>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include "parser.hpp"
#include "jasper_number.hpp"

// Give Flex the prototype of yylex we want ...
# define YY_DECL yy::parser::symbol_type yylex (Driver& driver)
// ... and declare it for the parser's sake.
YY_DECL;

// Conducting the whole scanning and parsing of Calc++.
class Driver
{
public:
    Driver ();
	llvm::LLVMContext context;
	llvm::IRBuilder<> builder = llvm::IRBuilder<>(context);
	std::unique_ptr<llvm::Module> module  = std::make_unique<llvm::Module>("Basic module", context);
	std::map<std::string, JasperNumber> variables;
	std::map<std::string, llvm::Value*> string_variables;
	std::map<std::string, llvm::Function*> functions;
	std::string currentFunction;
	std::vector<std::map<std::string, llvm::Value*>> functionVariables;

	// Run the parser on file F.  Return 0 on success.
	int parse (const std::string& f);
	// The name of the file being parsed.
	std::string file;
	// Whether to generate parser debug traces.
	bool trace_parsing;

	// Handling the scanner.
	void scan_begin ();
	void scan_end ();
	// Whether to generate scanner debug traces.
	bool trace_scanning;
	// The token's location used by the scanner.
	yy::location location;
};
#endif // ! DRIVER_HEADER
