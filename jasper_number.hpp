#ifndef JASPERCOMPILER_JASPER_NUMBER_HPP
#define JASPERCOMPILER_JASPER_NUMBER_HPP

#include <functional>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/ADT/APInt.h>

class JasperNumber {
public:
	static const auto INTEGER = llvm::ArrayType::TypeID::IntegerTyID;
	static const auto FLOATING = llvm::ArrayType::TypeID::FloatTyID;

	llvm::LLVMContext* context;
	llvm::Value* value;

	explicit JasperNumber() = default;
	JasperNumber(llvm::LLVMContext* context, llvm::Value *x);

	[[nodiscard]] JasperNumber operator+(const JasperNumber& b) const;
	[[nodiscard]] JasperNumber operator-(const JasperNumber& b) const;
	[[nodiscard]] JasperNumber operator*(const JasperNumber& b) const;
	[[nodiscard]] JasperNumber operator/(const JasperNumber& b) const;
	[[nodiscard]] JasperNumber binary_operation_floaty(
		const JasperNumber& b,
		const std::function<JasperNumber(llvm::ArrayType::TypeID t, llvm::Value* a, llvm::Value* b)>& f
	) const;
	[[nodiscard]] llvm::Value *cast_to_float() const;

	friend std::ostream& operator << (std::ostream& outs, const JasperNumber & number);
};


#endif //JASPERCOMPILER_JASPER_NUMBER_HPP
