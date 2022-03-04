#include "compile.hpp"

#include <sstream>

#include <cassert>
#include <climits>
#include <filesystem>

#include "../log/log.hpp"
#include "../typechecker.hpp"
#include "../ast.hpp"
#include "../utils/span.hpp"
#include "garbage_collector.hpp"
#include "compiler.hpp"
#include "utils.hpp"
#include "value.hpp"

namespace Compiler {

static void compile_stmt(AST::AST* ast, Compiler& e) {
	compileAny(ast, e);
	if (is_expression(ast))
		e.m_stack.pop_unsafe();
}

void compile(AST::Declaration* ast, Compiler& e) {
	auto ref = e.new_reference(Value {nullptr});
	e.m_stack.push(ref.as_value());
	if (ast->m_value) {
		compileAny(ast->m_value, e);
		auto value = e.m_stack.pop_unsafe();
		ref->m_value = value_of(value);
	}
}

void compile(AST::Program* ast, Compiler& e) {
	auto const& comps = *e.m_declaration_order;
	for (auto const& comp : comps) {
		for (auto decl : comp) {
			auto ref = e.new_reference(e.null());
			e.global_declare_direct(decl->identifier_text(), ref.get());
			compileAny(decl->m_value, e);
			auto value = e.m_stack.pop_unsafe();
			ref->m_value = value_of(value);
		}
	}
}

llvm::Value * compile(AST::NumberLiteral* ast, Compiler& e) {
	return llvm::ConstantFP::get(e.m_context, llvm::APFloat(ast->value()));
}

llvm::Value * compile(AST::IntegerLiteral* ast, Compiler& e) {
	return llvm::ConstantInt::get(e.m_context, llvm::APInt(32, ast->value()));
}

llvm::Constant * compile(AST::StringLiteral* ast, Compiler& e) {
	llvm::Constant *string =
	    llvm::ConstantDataArray::getString(e.m_context, "Error: The head has left the tape.",
	                                 true);
	return string;
	//TODO PUT THIS WHERE THE STRING IS USED
	//llvm::GlobalVariable *aberrormsg = new llvm::GlobalVariable(
	//    *e.m_module,
	//    string->getType(),
	//    true,
	//    llvm::GlobalValue::InternalLinkage,
	//    string,
	//    "aberrormsg");
}

llvm::Value * compile(AST::BooleanLiteral* ast, Compiler& e) {
	// TODO this is hack, I think, it uses a 1 bit number to store the bool
	return llvm::ConstantInt::get(e.m_context, llvm::APInt(1, ast->m_value ? 1 : 0));
}

void compile(AST::NullLiteral* ast, Compiler& e) {
	e.m_stack.push(e.null());
}

void compile(AST::ArrayLiteral* ast, Compiler& e) {
	auto result = e.new_list({});
	result->m_value.reserve(ast->m_elements.size());
	for (auto& element : ast->m_elements) {
		compileAny(element, e);
		auto ref = e.new_reference(Value {nullptr});
		ref->m_value = value_of(e.m_stack.pop_unsafe());
		result->append(ref.get());
	}
	e.m_stack.push(result.as_value());
}

void compile(AST::Identifier* ast, Compiler& e) {

#ifdef DEBUG
	Log::info() << "Identifier " << ast->text();
	if (ast->m_origin == TypedAST::Identifier::Origin::Local ||
	    ast->m_origin == TypedAST::Identifier::Origin::Capture) {
		Log::info() << "is local";
	} else {
		Log::info() << "is global";
	}
#endif

	if (ast->m_origin == AST::Identifier::Origin::Local ||
	    ast->m_origin == AST::Identifier::Origin::Capture) {
		if (ast->m_frame_offset == INT_MIN)
			Log::fatal() << "missing layout for identifier '" << ast->text() << "'";
		e.m_stack.push(e.m_stack.frame_at(ast->m_frame_offset));
	} else {
		e.m_stack.push(Value{e.global_access(ast->text())});
	}
}

void compile(AST::Block* ast, Compiler& e) {
	e.m_stack.start_stack_region();
	for (auto stmt : ast->m_body) {
		compile_stmt(stmt, e);
		if (e.m_returning)
			break;
	}
	e.m_stack.end_stack_region();
}

void compile(AST::ReturnStatement* ast, Compiler& e) {
	// TODO: proper error handling
	compileAny(ast->m_value, e);
	auto value = e.m_stack.pop_unsafe();
	e.save_return_value(value_of(value));
}

auto is_callable_type(ValueTag t) -> bool {
	return t == ValueTag::Function || t == ValueTag::NativeFunction;
}

void compile_call_function(Function* callee, size_t arg_count, Compiler& e) {

	// TODO: error handling ?
	assert(callee->m_def->m_args.size() == arg_count);

	for (auto capture : callee->m_captures)
		e.m_stack.push(Value{capture});

	compileAny(callee->m_def->m_body, e);

}

void compile(AST::CallExpression* ast, Compiler& e) {

	compileAny(ast->m_callee, e);

	// NOTE: keep callee on the stack
	auto callee = value_of(e.m_stack.access(0));
	assert(is_callable_type(callee.type()));

	auto& arglist = ast->m_args;
	size_t arg_count = arglist.size();

	int frame_start = e.m_stack.m_stack_ptr;
	if (callee.type() == ValueTag::Function) {
		for (auto expr : arglist) {
			// put arg on stack
			compileAny(expr, e);

			// wrap arg in reference
			auto ref = e.new_reference(Value {nullptr});
			ref->m_value = value_of(e.m_stack.access(0));
			e.m_stack.access(0) = ref.as_value();
		}
		e.m_stack.start_stack_frame(frame_start);

		compile_call_function(callee.get_cast<Function>(), arg_count, e);

		// pop the result of the function, and clobber the callee
		e.m_stack.frame_at(-1) = e.m_stack.pop_unsafe();
	} else if (callee.type() == ValueTag::NativeFunction) {
		for (auto expr : arglist)
			compileAny(expr, e);
		e.m_stack.start_stack_frame(frame_start);
		auto args = e.m_stack.frame_range(0, arg_count);

		// compute the result of the function, and clobber the callee
		e.m_stack.frame_at(-1) = callee.get_native_func()(args, e);
	} else {
		Log::fatal("Attempted to call a non function at runtime");
	}


	e.m_stack.end_stack_frame();
}

void compile(AST::IndexExpression* ast, Compiler& e) {
	// TODO: proper error handling

	compileAny(ast->m_callee, e);
	compileAny(ast->m_index, e);

	auto index = e.m_stack.pop_unsafe().get_integer();

	auto callee_ptr = e.m_stack.pop_unsafe();
	auto* callee = value_as<Array>(callee_ptr);

	e.m_stack.push(Value{callee->at(index)});
}

void compile(AST::TernaryExpression* ast, Compiler& e) {
	// TODO: proper error handling

	compileAny(ast->m_condition, e);
	auto condition = value_of(e.m_stack.pop_unsafe()).get_boolean();

	if (condition)
		compileAny(ast->m_then_expr, e);
	else
		compileAny(ast->m_else_expr, e);
}

void compile(AST::FunctionLiteral* ast, Compiler& e) {

	CapturesType captures;
	captures.assign(ast->m_captures.size(), nullptr);
	for (auto const& capture : ast->m_captures) {
		assert(capture.second.outer_frame_offset != INT_MIN);
		auto value = e.m_stack.frame_at(capture.second.outer_frame_offset);
		auto offset = capture.second.inner_frame_offset - ast->m_args.size();
		captures[offset] = value.get_cast<Reference>();
	}

	auto result = e.new_function(ast, std::move(captures));
	e.m_stack.push(result.as_value());
}

void compile(AST::AccessExpression* ast, Compiler& e) {
	compileAny(ast->m_target, e);
	auto rec_ptr = e.m_stack.pop_unsafe();
	auto rec = value_as<Record>(rec_ptr);
	e.m_stack.push(Value{rec->m_value[ast->m_member]});
}

void compile(AST::MatchExpression* ast, Compiler& e) {
	// Put the matched-on variant on the top of the stack
	compile(&ast->m_target, e);

	auto variant = value_as<Variant>(e.m_stack.access(0));

	auto constructor = variant->m_constructor;
	auto variant_value = value_of(variant->m_inner_value);

	// We won't pop it, because it is already lined up for the later
	// expressions. Instead, replace the variant with its inner value.
	// We also wrap it in a reference so it can be captured
	auto ref = e.new_reference(Value {nullptr});
	ref->m_value = variant_value;
	e.m_stack.access(0) = ref.as_value();
	
	auto case_it = ast->m_cases.find(constructor);
	// TODO: proper error handling
	assert(case_it != ast->m_cases.end());

	// put the result on the top of the stack
	compileAny(case_it->second.m_expression, e);

	// evil tinkering with the stack internals
	// (we just delete the variant value from behind the result)
	e.m_stack.access(1) = e.m_stack.access(0);
	e.m_stack.pop_unsafe();
}

void compile(AST::ConstructorExpression* ast, Compiler& e) {
	// NOTE: we leave the ctor on the stack for the time being

	compileAny(ast->m_constructor, e);
	auto constructor = value_of(e.m_stack.access(0));

	if (constructor.type() == ValueTag::RecordConstructor) {
		auto record_constructor = constructor.get_cast<RecordConstructor>();

		assert(ast->m_args.size() == record_constructor->m_keys.size());


		// compileAny arguments
		// arguments start at storage_point
		int storage_point = e.m_stack.m_stack_ptr;
		for (size_t i = 0; i < ast->m_args.size(); ++i)
			compileAny(ast->m_args[i], e);

		// store all arguments in record object
		RecordType record;
		for (size_t i = 0; i < ast->m_args.size(); ++i) {
			record[record_constructor->m_keys[i]] =
			    value_of(e.m_stack.m_stack[storage_point + i]);
		}
		
		// promote record object to heap
		auto result = e.m_gc->new_record(std::move(record));

		// pop arguments
		while (e.m_stack.m_stack_ptr > storage_point)
			e.m_stack.pop_unsafe();

		// replace ctor with record
		e.m_stack.access(0) = Value{result.get()};
	} else if (constructor.type() == ValueTag::VariantConstructor) {
		auto variant_constructor = constructor.get_cast<VariantConstructor>();

		assert(ast->m_args.size() == 1);

		compileAny(ast->m_args[0], e);
		auto result = e.m_gc->new_variant(
		    variant_constructor->m_constructor, value_of(e.m_stack.access(0)));

		// replace ctor with variant, and pop value
		e.m_stack.access(1) = Value{result.get()};
		e.m_stack.pop_unsafe();
	}
}

void compile(AST::SequenceExpression* ast, Compiler& e) {
	compile(ast->m_body, e);
	if (!e.m_returning)
		e.save_return_value(Value {});
	e.m_stack.push(e.fetch_return_value());
}

void compile(AST::IfElseStatement* ast, Compiler& e) {
	// TODO: proper error handling

	compileAny(ast->m_condition, e);
	bool condition = value_of(e.m_stack.pop_unsafe()).get_boolean();

	if (condition)
		compile_stmt(ast->m_body, e);
	else if (ast->m_else_body)
		compile_stmt(ast->m_else_body, e);
}

void compile(AST::WhileStatement* ast, Compiler& e) {
	while (1) {
		compileAny(ast->m_condition, e);
		auto condition = value_of(e.m_stack.pop_unsafe()).get_boolean();

		if (!condition)
			break;

		compile_stmt(ast->m_body, e);

		if (e.m_returning)
			break;
	}
}

void compile(AST::StructExpression* ast, Compiler& e) {
	e.push_record_constructor(ast->m_fields);
}

void compile(AST::UnionExpression* ast, Compiler& e) {
	RecordType constructors;
	for(auto& constructor : ast->m_constructors) {
		constructors.insert(
		    {constructor, Value{e.m_gc->new_variant_constructor_raw(constructor)}});
	}
	auto result = e.new_record(std::move(constructors));
	e.m_stack.push(result.as_value());
}

void compile(AST::TypeFunctionHandle* ast, Compiler& e) {
	compileAny(ast->m_syntax, e);
}

void compile(AST::MonoTypeHandle* ast, Compiler& e) {
	TypeFunctionId type_function_header =
	    e.m_tc->m_core.m_mono_core.find_function(ast->m_value);
	int type_function = e.m_tc->m_core.m_tf_core.find_function(type_function_header);
	auto& type_function_data = e.m_tc->m_core.m_type_functions[type_function];
	e.push_record_constructor(type_function_data.fields);
}

void compile(AST::Constructor* ast, Compiler& e) {
	TypeFunctionId tf_header = e.m_tc->m_core.m_mono_core.find_function(ast->m_mono);
	int tf = e.m_tc->m_core.m_tf_core.find_function(tf_header);
	auto& tf_data = e.m_tc->m_core.m_type_functions[tf];

	if (tf_data.tag == TypeFunctionTag::Record) {
		e.push_record_constructor(tf_data.fields);
	} else if (tf_data.tag == TypeFunctionTag::Variant) {
		e.push_variant_constructor(ast->m_id);
	} else {
		Log::fatal("not implemented this type function for construction");
	}
}

void compile(AST::TypeTerm* ast, Compiler& e) {
	compileAny(ast->m_callee, e);
}

llvm::Value* compileValue(AST::AST* ast, Compiler& e) {

#define DISPATCH(type)                                                         \
	case ASTTag::type:                                                    \
		return compile(static_cast<AST::type*>(ast), e)

#ifdef DEBUG
	Log::info() << "case in compileAny: " << typed_ast_string[(int)ast->type()];
#endif

	switch (ast->type()) {
		DISPATCH(NumberLiteral);
		DISPATCH(IntegerLiteral);
		DISPATCH(BooleanLiteral);
	}

	Log::fatal() << "(internal) unhandled case in compileAny: "
	             << ast_string[(int)ast->type()];
}

llvm::Constant* compileConstant(AST::AST* ast, Compiler& e) {

#define DISPATCH(type)                                                         \
	case ASTTag::type:                                                    \
		return compile(static_cast<AST::type*>(ast), e)

#ifdef DEBUG
	Log::info() << "case in compileAny: " << typed_ast_string[(int)ast->type()];
#endif

	switch (ast->type()) {
		DISPATCH(StringLiteral);
	}

	Log::fatal() << "(internal) unhandled case in compileAny: "
	             << ast_string[(int)ast->type()];
}

void compileAny(AST::AST* ast, Compiler& e) {

#define DISPATCH(type)                                                         \
	case ASTTag::type:                                                    \
		return compile(static_cast<AST::type*>(ast), e)

#ifdef DEBUG
	Log::info() << "case in compileAny: " << typed_ast_string[(int)ast->type()];
#endif

	switch (ast->type()) {
		DISPATCH(NullLiteral);
		DISPATCH(ArrayLiteral);
		DISPATCH(FunctionLiteral);

		DISPATCH(Identifier);
		DISPATCH(CallExpression);
		DISPATCH(IndexExpression);
		DISPATCH(TernaryExpression);
		DISPATCH(AccessExpression);
		DISPATCH(MatchExpression);
		DISPATCH(ConstructorExpression);
		DISPATCH(SequenceExpression);

		DISPATCH(Program);
		DISPATCH(Declaration);

		DISPATCH(Block);
		DISPATCH(ReturnStatement);
		DISPATCH(IfElseStatement);
		DISPATCH(WhileStatement);

		DISPATCH(TypeTerm);
		DISPATCH(StructExpression);
		DISPATCH(UnionExpression);
		DISPATCH(TypeFunctionHandle);
		DISPATCH(MonoTypeHandle);
		DISPATCH(Constructor);
	}

	Log::fatal() << "(internal) unhandled case in compileAny: "
	             << ast_string[(int)ast->type()];
}

static bool createExecutableObject(llvm::Module& module) {
	// Initialize the target registry etc.
	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmParsers();
	llvm::InitializeAllAsmPrinters();

	auto targetTriple = llvm::sys::getDefaultTargetTriple();
	module.setTargetTriple(targetTriple);

	std::string error;
	auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);

	// Print an error and exit if we couldn't find the requested target.
	// This generally occurs if we've forgotten to initialise the
	// TargetRegistry or we have a bogus target triple.
	if (!target) {
		llvm::errs() << error;
		return false;
	}

	auto cpu = "generic";
	auto features = "";

	llvm::TargetOptions opt;
	auto RM = llvm::Optional<llvm::Reloc::Model>();
	auto targetMachine = target->createTargetMachine(targetTriple, cpu, features, opt, RM);

	module.setDataLayout(targetMachine->createDataLayout());

	std::filesystem::create_directories("./bin");

	if(![&]{
		    auto filename = "bin/output.o";
		    std::error_code EC;
		    llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);

		    if (EC) {
			    llvm::errs() << "Could not open file: " << EC.message();
			    return false;
		    }

		    llvm::legacy::PassManager pass;
		    auto filetype = llvm::CGFT_ObjectFile;

		    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, filetype)) {
			    llvm::errs() << "targetMachine can't emit a file of this type";
			    return false;
		    }

		    pass.run(module);
		    dest.flush();
		    return true;
	    }()) return false;

	if(![&] {
		    auto filename = "bin/assembly.s";
		    std::error_code EC;
		    llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);

		    llvm::legacy::PassManager pass;
		    auto filetype = llvm::CGFT_AssemblyFile;

		    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, filetype)) {
			    llvm::errs() << "targetMachine can't emit a file of this type";
			    return false;
		    }

		    pass.run(module);
		    dest.flush();
		    return true;
	    }()) return false;

	return true;
}

void preCompileSteps(Compiler& e) {
	const auto name = "BrainF";
	e.m_module = std::make_unique<llvm::Module>(name, e.m_context);
}

void postCompileSteps(Compiler& e) {
	if (verifyModule(*e.m_module)) {
		llvm::errs() << "Error: module failed verification.  This shouldn't happen.\n";
		abort();
	}

	e.m_module->print(llvm::errs(), nullptr);

	createExecutableObject(*e.m_module);

	llvm::llvm_shutdown();
}


void compile(AST::AST* ast, Compiler& e) {

	preCompileSteps(e);

	compileAny(ast, e);

	//define i32 @main(i32 %argc, i8 **%argv)
	llvm::FunctionType *main_func_fty = llvm::FunctionType::get(
	    llvm::Type::getInt32Ty(e.m_module->getContext()),
	    {llvm::Type::getInt32Ty(e.m_module->getContext()),
	     llvm::Type::getInt8Ty(e.m_module->getContext())->getPointerTo()->getPointerTo()},
	    false);
	llvm::Function *main_func =
	    llvm::Function::Create(main_func_fty, llvm::Function::ExternalLinkage, "main", e.m_module.get());

	{
		llvm::Function::arg_iterator args = main_func->arg_begin();
		llvm::Value *arg_0 = &*args++;
		arg_0->setName("argc");
		llvm::Value *arg_1 = &*args++;
		arg_1->setName("argv");
	}

	//main.0:
	llvm::BasicBlock *bb = llvm::BasicBlock::Create(e.m_module->getContext(), "main.0", main_func);

	//ret i32 0
	llvm::ReturnInst::Create(e.m_module->getContext(),
	                         llvm::ConstantInt::get(e.m_module->getContext(), llvm::APInt(32, 0)), bb);

	postCompileSteps(e);
}

} // namespace Interpreter
