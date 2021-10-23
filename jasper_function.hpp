#ifndef JASPERCOMPILER_JASPER_FUNCTION_HPP
#define JASPERCOMPILER_JASPER_FUNCTION_HPP

#include <llvm/IR/Type.h>

struct JasperFunction {
	std::vector<llvm::Type*> argumentTypes;
	std::vector<std::string> argumentNames;
	llvm::Type* returnType;
};


#endif //JASPERCOMPILER_JASPER_FUNCTION_HPP
