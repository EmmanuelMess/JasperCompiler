#include <iostream>
#include <iomanip>
#include "jasper_number.hpp"

JasperNumber JasperNumber::operator+(const JasperNumber& b) const {
	return binary_operation_floaty(b, [](JasperNumber::NumberType t, const JasperNumber& a, const JasperNumber& b){
		switch (t) {
			case JasperNumber::FLOATING:
				return JasperNumber {a.floating + b.floating};
			case JasperNumber::INTEGER:
				return JasperNumber {a.integer + b.integer};
		}
	});
}

JasperNumber JasperNumber::operator-(const JasperNumber& b) const {
	return binary_operation_floaty(b, [](JasperNumber::NumberType t, const JasperNumber& a, const JasperNumber& b){
		switch (t) {
			case JasperNumber::FLOATING:
				return JasperNumber {a.floating - b.floating};
			case JasperNumber::INTEGER:
				return JasperNumber {a.integer - b.integer};
		}
	});
}

JasperNumber JasperNumber::operator*(const JasperNumber& b) const {
	return binary_operation_floaty(b, [](JasperNumber::NumberType t, const JasperNumber& a, const JasperNumber& b){
		switch (t) {
			case JasperNumber::FLOATING:
				return JasperNumber {a.floating * b.floating};
			case JasperNumber::INTEGER:
				return JasperNumber {a.integer * b.integer};
		}
	});
}

JasperNumber JasperNumber::operator/(const JasperNumber& b) const {
	return binary_operation_floaty(b, [](JasperNumber::NumberType t, const JasperNumber& a, const JasperNumber& b){
		switch (t) {
			case JasperNumber::FLOATING:
				return JasperNumber {a.floating / b.floating};
			case JasperNumber::INTEGER:
				return JasperNumber {a.integer / b.integer};
		}
	});
}

[[nodiscard]] JasperNumber JasperNumber::binary_operation_floaty(
	const JasperNumber& b,
	const std::function<JasperNumber(JasperNumber::NumberType tag, const JasperNumber& a, const JasperNumber& b)>& f
	) const {
	switch (tag) {
		case JasperNumber::FLOATING: {
			float v = floating;
			switch (b.tag) {
				case JasperNumber::FLOATING:
					return f(JasperNumber::FLOATING, v, b.floating);
				case JasperNumber::INTEGER:
					return f(JasperNumber::FLOATING, v, static_cast<float>(b.integer));
			}
		}
		case JasperNumber::INTEGER: {
			int v = integer;

			switch (b.tag) {
				case JasperNumber::FLOATING:
					return f(JasperNumber::FLOATING, static_cast<float>(v), b.floating);
				case JasperNumber::INTEGER:
					return f(JasperNumber::INTEGER, v, b.integer);
			}
		}
	}
}

std::ostream& operator << (std::ostream& outs, const JasperNumber & number) {
	switch (number.tag) {
		case JasperNumber::FLOATING:
			return outs << "F" << number.floating;
		case JasperNumber::INTEGER:
			return outs << "I" << number.integer;
	}
}