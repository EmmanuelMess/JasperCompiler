#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <llvm/IR/Value.h>

#include "../utils/interned_string.hpp"
#include "../utils/span.hpp"
#include "value_tag.hpp"
#include "gc_cell.hpp"

namespace AST {
struct FunctionLiteral;
}

namespace Compiler {

struct Compiler;
struct Reference;
struct Value;

using Identifier = InternedString;
using StringType = std::string;
using RecordType = std::unordered_map<Identifier, Value>;
using ArrayType = std::vector<Reference*>;
using FunctionType = AST::FunctionLiteral*;
using NativeFunction = auto(Span<Value>, Compiler&) -> Value;
using CapturesType = std::vector<Reference*>;

inline bool is_heap_type(ValueTag tag) {
	return tag != ValueTag::Null && tag != ValueTag::Boolean &&
	       tag != ValueTag::Integer && tag != ValueTag::Float &&
		   tag != ValueTag::NativeFunction;
}

struct Value {
	explicit Value(GcCell* ptr)
	    : tag {ptr ? ptr->type() : ValueTag::Null}
	    , ptr {ptr} {}

	explicit Value(std::nullptr_t)
	    : tag {ValueTag::Null}
	    , ptr {nullptr} {}

        explicit Value(llvm::Value* value)
            : tag {ValueTag::LlvmValue}
              , as_llvm_value {value} {}

	explicit Value(bool boolean)
	    : tag {ValueTag::Boolean}
	    , as_boolean {boolean} {}

	explicit Value(int integer)
	    : tag {ValueTag::Integer}
	    , as_integer {integer} {}

	explicit Value(float number)
	    : tag {ValueTag::Float}
	    , as_float {number} {}

	explicit Value(NativeFunction* func)
	    : tag {ValueTag::NativeFunction}
	    , as_native_func {func} {}

	Value()
	    : tag {ValueTag::Null}
	    , ptr {nullptr} {}

	GcCell& operator*() const {
		assert(is_heap_type(tag));
		return *ptr;
	};

	[[nodiscard]] GcCell* get() const {
		assert(is_heap_type(tag));
		return ptr;
	}

	template <typename T>
	T* get_cast();

        [[nodiscard]] llvm::Value* get_llvm_value() const {
          assert(tag == ValueTag::LlvmValue);
          return as_llvm_value;
        }

	[[nodiscard]] int get_integer() const {
		assert(tag == ValueTag::Integer);
		return as_integer;
	}

	[[nodiscard]] float get_float() const {
		assert(tag == ValueTag::Float);
		return as_float;
	}

	[[nodiscard]] bool get_boolean() const {
		assert(tag == ValueTag::Boolean);
		return as_boolean;
	}

	[[nodiscard]] NativeFunction* get_native_func() const {
		assert(tag == ValueTag::NativeFunction);
		return as_native_func;
	}

	[[nodiscard]] ValueTag type() const {
		if (is_heap_type(tag)) {
			assert(ptr);
			assert(ptr->type() == tag);
		}
		return tag;
	}

	ValueTag tag;
	union {
	GcCell* ptr;
	llvm::Value* as_llvm_value;
        bool as_boolean;
	int as_integer;
	float as_float;
	NativeFunction* as_native_func;
	};
};

void print(Value v, int d = 0);

struct String : GcCell {
	std::string m_value = "";

	String();
	String(std::string s);
};

struct Array : GcCell {
	ArrayType m_value;

	Array();
	Array(ArrayType l);

	void append(Reference* v);
	Reference* at(int position);
};

struct Record : GcCell {
	RecordType m_value;

	Record();
	Record(RecordType);

	void addMember(Identifier const& id, Value v);
	Value getMember(Identifier const& id);
};

struct Variant : GcCell {
	InternedString m_constructor;
	Value m_inner_value {nullptr}; // empty constructor

	Variant(InternedString constructor);
	Variant(InternedString constructor, Value v);
};

struct Function : GcCell {
	FunctionType m_def;
	CapturesType m_captures;

	Function(FunctionType, CapturesType);
};

struct Reference : GcCell {
	Value m_value;

	Reference(Value value);
};

struct VariantConstructor : GcCell {
	InternedString m_constructor;

	VariantConstructor(InternedString constructor);
};

struct RecordConstructor : GcCell {
	std::vector<InternedString> m_keys;

	RecordConstructor(std::vector<InternedString> keys);
};

template<typename T>
struct type_data;

template<> struct type_data<String> { static constexpr auto tag = ValueTag::String; };
template<> struct type_data<Array> { static constexpr auto tag = ValueTag::Array; };
template<> struct type_data<Record> { static constexpr auto tag = ValueTag::Record; };
template<> struct type_data<Variant> { static constexpr auto tag = ValueTag::Variant; };
template<> struct type_data<Function> { static constexpr auto tag = ValueTag::Function; };
template<> struct type_data<Reference> { static constexpr auto tag = ValueTag::Reference; };
template<> struct type_data<VariantConstructor> { static constexpr auto tag = ValueTag::VariantConstructor; };
template<> struct type_data<RecordConstructor> { static constexpr auto tag = ValueTag::RecordConstructor; };

template <typename T>
inline T* Value::get_cast() {
	static_assert(std::is_base_of<GcCell, T>::value, "T is not a subclass of GcCell");
	assert(is_heap_type(tag));
	assert(tag == type_data<T>::tag);
	assert(ptr);
	return static_cast<T*>(ptr);
}

} // namespace Interpreter
