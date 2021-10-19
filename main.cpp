#include <iostream>
#include "driver.hpp"
#include "jasper_number.hpp"

int main (int argc, char *argv[]) {
	int res = 0;
	driver drv;
	for (int i = 1; i < argc; ++i) {
		if (argv[i] == std::string("-p")) {
			drv.trace_parsing = true;
		} else if (argv[i] == std::string("-s")) {
			drv.trace_scanning = true;
		} else if (!drv.parse(argv[i])) {
			for (const auto & variable : drv.variables) {
				std::cout << variable.first << ": " << variable.second << '\n';
			}
		} else {
			res = 1;
		}
	}
	return res;
}
