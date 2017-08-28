// g++ -std=c++14 -O3 -march=native -o quodigious_original quodigious_original.cc
#include <iostream>
#include <cstdint>
#include <cmath>
using number = uint32_t;

constexpr number pow10(number count) noexcept {
	if (count == 0) {
		return 1;
	} else {
		return 10 * pow10(count - 1);
	}
}
bool isQuodigious(number count, number val) noexcept {
	// decompose the digits
	auto current = val;
	auto sum = 0u;
	auto prod = 1u;
	for(auto i = 0u; i < count; ++i) {
		auto result = current % 10L;
		if(result < 2) {
			return false;
		}
		sum += result;
		prod *= result;
		current /= 10L;
	}
	return (val % prod == 0) && (val % sum == 0);
}
int main() {
	while (std::cin.good()) {
		number currentIndex = 0;
		std::cin >> currentIndex;
		if (std::cin.good()) {
			if (currentIndex < 1 || currentIndex > 9) {
				continue;
			}
			for (auto i = pow10(currentIndex - 1); i < pow10(currentIndex); ++i) {
				if (isQuodigious(currentIndex, i)) {
					std::cout << i << std::endl;
				}
			}
			std::cout << std::endl;
		}
	}
	return 0;
}
