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
	vec64 l0, l1, l2, l3;
	while(std::cin.good()) {
		u64 currentIndex = 0;
		std::cin >> currentIndex;
		if (std::cin.good()) {
			auto base = static_cast<u64>(2.2 * pow(10, currentIndex - 1));
			auto end = static_cast<u64>(pow(10, currentIndex));

			auto start0 = base;
			auto end0 = end / 4;
			auto fut0 = std::async(std::launch::async, [len = currentIndex, start = start0, end = end0, &l0]() { return performQuodigiousCheck(len, start, end, l0); });
			auto start1 = end0;
			auto end1 = end / 2;
			auto fut1 = std::async(std::launch::async, [len = currentIndex, start = start1, end = end1, &l1]() { return performQuodigiousCheck(len, start, end, l1); });
			auto start2 = end1;
			auto end2 = static_cast<u64>(0.75 * end);
			auto fut2 = std::async(std::launch::async, [len = currentIndex, start = start2, end = end2, &l2]() { return performQuodigiousCheck(len, start, end, l2); });
			auto start3 = end2;
			auto end3 = end;
			auto fut3 = std::async(std::launch::async, [len = currentIndex, start = start3, end = end3, &l3]() { return performQuodigiousCheck(len, start, end, l3); });
			fut0.get();
			fut1.get();
			fut2.get();
			fut3.get();
			for (auto value : l0) {
				std::cout << value << std::endl;
			}
			for (auto value : l1) {
				std::cout << value << std::endl;
			}
			for (auto value : l2) {
				std::cout << value << std::endl;
			}
			for (auto value : l3) {
				std::cout << value << std::endl;
			}
			std::cout << std::endl;
			l0.clear();
			l1.clear();
			l2.clear();
			l3.clear();
		}
	}
	return 0;
}
