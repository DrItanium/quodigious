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
		switch (result) {
			case 0:
			case 1:
				return false;
			case 2:
				prod <<= 1;
				break;
			case 3:
				prod = prod + prod + prod;
				break;
			case 4:
				prod <<= 2;
				break;
			case 5:
				prod = prod + prod + prod + prod + prod;
				break;
			case 6:
				prod = prod + prod + prod + prod + prod + prod;
				break;
			case 7:
				prod = prod + prod + prod + prod + prod + prod + prod;
				break;
			case 8:
				prod <<= 3;
				break;
			case 9:
				prod = prod + prod + prod + prod + prod + prod + prod + prod + prod;
				break;
			default:
				throw 0;
		}
		sum += result;
		current /= 10u;
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
void printout(auto& future, vec64& l) noexcept {
	future.get();
	for (auto v : l) {
		std::cout << v << std::endl;
	}
	l.clear();
}
int main() {
	static constexpr auto factor = 2.0 + (2.0 / 9.0);
	vec64 l0, l1, l2, l3, l4, l5, l6, l7;
	while(std::cin.good()) {
		u64 currentIndex = 0;
		std::cin >> currentIndex;
		if (std::cin.good()) {
			auto base = static_cast<u64>(pow(10, currentIndex -1));
			auto st = static_cast<u64>(factor * base);
			auto fut0 = std::async(std::launch::async, [len = currentIndex, start = st, end = 3 * base, &l0]() { return performQuodigiousCheck(len, start, end, l0); });
			auto fut1 = std::async(std::launch::async, [len = currentIndex, start = st + base, end = 4 * base, &l1]() { return performQuodigiousCheck(len, start, end, l1); });
			auto fut2 = std::async(std::launch::async, [len = currentIndex, start = st + (base * 2), end = 5 * base, &l2]() { return performQuodigiousCheck(len, start, end, l2); });
			auto fut3 = std::async(std::launch::async, [len = currentIndex, start = st + (base * 3), end = 6 * base, &l3]() { return performQuodigiousCheck(len, start, end, l3); });
			auto fut4 = std::async(std::launch::async, [len = currentIndex, start = st + (base * 4), end = 7 * base, &l4]() { return performQuodigiousCheck(len, start, end, l4); });
			auto fut5 = std::async(std::launch::async, [len = currentIndex, start = st + (base * 5), end = 8 * base, &l5]() { return performQuodigiousCheck(len, start, end, l5); });
			auto fut6 = std::async(std::launch::async, [len = currentIndex, start = st + (base * 6), end = 9 * base, &l6]() { return performQuodigiousCheck(len, start, end, l6); });
			auto fut7 = std::async(std::launch::async, [len = currentIndex, start = st + (base * 7), end = 10 * base, &l7]() { return performQuodigiousCheck(len, start, end, l7); });
			printout(fut0, l0);
			printout(fut1, l1);
			printout(fut2, l2);
			printout(fut3, l3);
			printout(fut4, l4);
			printout(fut5, l5);
			printout(fut6, l6);
			printout(fut7, l7);
			std::cout << std::endl;
		}
	}
	return 0;
}
