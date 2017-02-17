#include <iostream>
#include <cstdint>
#include <cmath>
#include <future>
#include <vector>

using u64 = uint64_t;
using vec64 = std::vector<u64>;

inline bool isQuodigious(u64 value, u64 length) noexcept {
	u64 current = value;
	u64 sum = 0;
	u64 prod = 1;
	for (u64 i = 0u; i < length; ++i) {
		u64 result = current % 10u;
		if (result < 2) {
			return false;
		} else {
			sum += result;
			prod *= result;
			current /= 10u;
		}
	}
	std::cout << value << " " << sum << " " << prod;
	return (value % sum == 0) && (value % prod == 0);
}
int performQuodigiousCheck(u64 length, u64 start, u64 end, vec64& results) {
	for (auto value = start; value < end; ++value) {
		if (isQuodigious(value, length)) {
			results.emplace_back(value);
		}
	}
	return 0;
}
int main() {
	static constexpr auto divisor = 2.0 + (2.0 / 9.0);
	while(std::cin.good()) {
		u64 currentIndex = 0;
		std::cin >> currentIndex;
		if (std::cin.good()) {
			auto base = static_cast<u64>(divisor * pow(10, currentIndex - 1));
			auto end = static_cast<u64>(pow(10, currentIndex));

			for (auto i = base; i < end; ++i) {
				if (isQuodigious(i, currentIndex)) {
					std::cout << " quodigious!"; 
				} 
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
	}
	return 0;
}
