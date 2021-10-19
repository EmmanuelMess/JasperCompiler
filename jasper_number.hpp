#ifndef JASPERCOMPILER_JASPER_NUMBER_HPP
#define JASPERCOMPILER_JASPER_NUMBER_HPP

#include <functional>

class JasperNumber {
public:
	enum NumberType {INTEGER, FLOATING} tag;
	union {
		int integer;
		float floating;
	};

	explicit JasperNumber() = default;
	JasperNumber(int x) : integer(x), tag(INTEGER) {}
	JasperNumber(float x) : floating(x), tag(FLOATING) {}

	[[nodiscard]] JasperNumber operator+(const JasperNumber& b) const;
	[[nodiscard]] JasperNumber operator-(const JasperNumber& b) const;
	[[nodiscard]] JasperNumber operator*(const JasperNumber& b) const;
	[[nodiscard]] JasperNumber operator/(const JasperNumber& b) const;
	[[nodiscard]] JasperNumber binary_operation_floaty(
		const JasperNumber& b,
		const std::function<JasperNumber(JasperNumber::NumberType t, const JasperNumber& a, const JasperNumber& b)>& f
	) const;

	friend std::ostream& operator << (std::ostream& outs, const JasperNumber & number);
};


#endif //JASPERCOMPILER_JASPER_NUMBER_HPP
