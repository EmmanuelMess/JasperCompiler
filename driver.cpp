#include "driver.hpp"
#include "parser.hpp"

static const std::string MAIN_FUNCTION = "main";

Driver::Driver () : trace_parsing (false), trace_scanning (false) {
}

static llvm::Function *createMainFunction(Driver& driver) noexcept {
	llvm::FunctionType *functionType = llvm::FunctionType::get(llvm::Type::getInt32Ty(driver.context), std::vector<llvm::Type*>(), false);
	llvm::Function *function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, "main", driver.module.get());
	return function;
}

int Driver::parse (const std::string &f) {
	file = f;
	location.initialize(&file);

	functions[MAIN_FUNCTION] = createMainFunction(*this);
	currentFunction = MAIN_FUNCTION;

	llvm::BasicBlock* block = llvm::BasicBlock::Create(context, "entry", functions[MAIN_FUNCTION]);
	builder.SetInsertPoint(block);

	scan_begin();
	yy::parser parse(*this);
	parse.set_debug_level(trace_parsing);
	int res = parse();
	scan_end();

	builder.CreateRet(llvm::ConstantInt::get(context, { 32, 1, true }));
	verifyFunction(*functions[MAIN_FUNCTION]);
	currentFunction = "";

	return res;
}