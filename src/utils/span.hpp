#pragma once

#include <cassert>

template<typename T>
struct Span {
	using iterator = T*;
	using const_iterator = T const*;

	T* m_data;
	int m_length;

	T& operator[] (int i) {
		return m_data[i];
	}

	T& at(int i){
		assert(i >= 0);
		assert(i < m_length);
		return m_data[i];
	}

	[[nodiscard]] [[nodiscard]] int size () const {
		return m_length;
	}

	iterator begin() {
		return m_data;
	}

	iterator end() {
		return m_data + m_length;
	}

	const_iterator cbegin() const {
		return m_data;
	}

	const_iterator cend() const {
		return m_data + m_length;
	}
};

