#include <iostream>
#include <iomanip>
#include <llvm/IR/Value.h>
#include "jasper_number.hpp"

JasperNumber::JasperNumber(llvm::LLVMContext* context, llvm::Value *x) : context(context), value(x) {}

JasperNumber JasperNumber::operator+(const JasperNumber& b) const {
	return binary_operation_floaty(b, [this](llvm::ArrayType::TypeID t, llvm::Value* a, llvm::Value* b) -> JasperNumber {
		switch (t) {
			case FLOATING:
				return { context, llvm::BinaryOperator::Create(llvm::Instruction::FAdd, a, b) };
			case INTEGER:
				return { context, llvm::BinaryOperator::Create(llvm::Instruction::Add, a, b) };
			default:
				throw std::out_of_range(std::to_string(t));
		}
	});
}

JasperNumber JasperNumber::operator-(const JasperNumber& b) const {
	return binary_operation_floaty(b, [this](llvm::ArrayType::TypeID t, llvm::Value* a, llvm::Value* b) -> JasperNumber {
		switch (t) {
			case FLOATING:
				return { context, llvm::BinaryOperator::Create(llvm::Instruction::FSub, a, b) };
			case INTEGER:
				return { context, llvm::BinaryOperator::Create(llvm::Instruction::Sub, a, b) };
			default:
				throw std::out_of_range(std::to_string(t));
		}
	});
}

JasperNumber JasperNumber::operator*(const JasperNumber& b) const {
	return binary_operation_floaty(b, [this](llvm::ArrayType::TypeID t, llvm::Value* a, llvm::Value* b) -> JasperNumber {
		switch (t) {
			case FLOATING:
				return { context, llvm::BinaryOperator::Create(llvm::Instruction::FMul, a, b) };
			case INTEGER:
				return { context, llvm::BinaryOperator::Create(llvm::Instruction::Mul, a, b) };
			default:
				throw std::out_of_range(std::to_string(t));
		}
	});
}

JasperNumber JasperNumber::operator/(const JasperNumber& b) const {
	return binary_operation_floaty(b, [this](llvm::ArrayType::TypeID t, llvm::Value* a, llvm::Value* b) -> JasperNumber {
		switch (t) {
			case FLOATING:
				return { context, llvm::BinaryOperator::Create(llvm::Instruction::FDiv, a, b) };
			case INTEGER:
				return { context, llvm::BinaryOperator::Create(llvm::Instruction::UDiv, a, b) };
			default:
				throw std::out_of_range(std::to_string(t));
		}
	});
}

[[nodiscard]] JasperNumber JasperNumber::binary_operation_floaty(
	const JasperNumber& b,
	const std::function<JasperNumber(llvm::ArrayType::TypeID t, llvm::Value* a, llvm::Value* b)>& f
) const {
	switch (value->getType()->getTypeID()) {
		case FLOATING: {
			switch (b.value->getType()->getTypeID()) {
				case FLOATING:
					return f(FLOATING, value, b.value);
				case llvm::ArrayType::TypeID::IntegerTyID:
					return f(FLOATING, value, cast_to_float());
				default:
					throw std::out_of_range(std::to_string(b.value->getType()->getTypeID()));
			}
		}
		case INTEGER: {
			switch (b.value->getType()->getTypeID()) {
				case FLOATING:
					return f(FLOATING, cast_to_float(), b.value);
				case INTEGER:
					return f(INTEGER, value, b.value);
				default:
					throw std::out_of_range(std::to_string(b.value->getType()->getTypeID()));
			}
		}
		default:
			throw std::out_of_range(std::to_string(b.value->getType()->getTypeID()));
	}
}

[[nodiscard]] llvm::Value *JasperNumber::cast_to_float() const {
	return new llvm::SIToFPInst(value, llvm::Type::getFloatTy(*context));
}

std::ostream& operator << (std::ostream& outs, const JasperNumber & number) {
	if(number.value) {
		switch (number.value->getType()->getTypeID()) {
			case JasperNumber::INTEGER:
				outs << "I";
				break;
			case JasperNumber::FLOATING:
				outs << "F";
				break;
		}

		return outs << " " << number.value;
	}

	return outs << "NULL";
}