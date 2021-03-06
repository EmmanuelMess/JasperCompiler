#pragma once

#include "value_tag.hpp"

namespace Compiler {

struct GcCell {
  protected:
	ValueTag m_tag;

  public:
	bool m_visited = false;
	int m_cpp_refcount = 0;

	GcCell(ValueTag type)
	    : m_tag(type) {}

	[[nodiscard]] ValueTag type() const {
		return m_tag;
	}

	void visit();

	virtual ~GcCell() = default;
};

} // namespace Interpreter
