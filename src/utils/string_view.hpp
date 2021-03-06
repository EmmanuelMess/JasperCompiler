#pragma once

#include <iosfwd>

struct string_view {
	char const* m_data {nullptr};
	size_t m_size {0};

	string_view() = default;
	string_view(char const*);
	string_view(const std::string&);
	string_view(char const*, size_t size);

	[[nodiscard]] int size() const {
		return m_size;
	}
	[[nodiscard]] char const* cbegin() const {
		return m_data;
	}
	[[nodiscard]] char const* cend() const {
		return m_data + m_size;
	}
	[[nodiscard]] char const* begin() const {
		return cbegin();
	}
	[[nodiscard]] char const* end() const {
		return cend();
	}
};

std::ostream& operator<<(std::ostream&, string_view const&);
