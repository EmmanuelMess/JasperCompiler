#pragma once

#include <llvm/ADT/APInt.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/ADT/Optional.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/Host.h>
#include <llvm/Target/TargetMachine.h>

#include "gc_ptr.hpp"
#include "stack.hpp"
#include "value.hpp"

#include <map>

namespace AST {
struct Declaration;
}

namespace TypeChecker {
struct TypeChecker;
}

namespace Compiler {

struct GC;
struct Error;

struct Scope {
	std::map<InternedString, Reference*> m_declarations;

	void declare(const Identifier& i, Reference* v);
	Reference* access(const Identifier& i);
};

struct Compiler {
	llvm::LLVMContext m_context;
	std::unique_ptr<llvm::Module> m_module;
        std::unique_ptr<llvm::IRBuilder<>> m_builder;
        //TODO actually optimize
        std::unique_ptr<llvm::legacy::FunctionPassManager> m_function_pass_manager;

	Stack m_stack;
	TypeChecker::TypeChecker* m_tc;
	GC* m_gc;
	std::vector<std::vector<AST::Declaration*>> const* m_declaration_order;
	int m_gc_size_on_last_pass {64};
	bool m_returning{false};
	Value m_return_value {nullptr};
	Scope m_global_scope;

	Compiler(
	    TypeChecker::TypeChecker* tc,
	    GC* gc,
	    std::vector<std::vector<AST::Declaration*>> const* declaration_order)
	    : m_tc {tc}
	    , m_gc {gc}
	    , m_declaration_order {declaration_order} {}

        void save_function(llvm::Function*);
	void save_return_value(Value);
	Value fetch_return_value();

	void run_gc();
	void run_gc_if_needed();

	// Binds a global name to the given reference
	void global_declare_direct(const Identifier& i, Reference* v);
	void global_declare(const Identifier& i, Value v);
	Reference* global_access(const Identifier& i);
	void assign(Value dst, Value src);

	auto null() -> Value;
        void push_llvm_value(llvm::Value*);
	void push_integer(int);
	void push_float(float);
	void push_boolean(bool);
	void push_string(std::string);
	void push_variant_constructor(InternedString constructor);
	void push_record_constructor(std::vector<InternedString>);
	auto new_list(ArrayType) -> gc_ptr<Array>;
	auto new_record(RecordType) -> gc_ptr<Record>;
	auto new_function(FunctionType, CapturesType) -> gc_ptr<Function>;
	auto new_error(std::string) -> gc_ptr<Error>;
	auto new_reference(Value) -> gc_ptr<Reference>;
};

} // namespace Interpreter
