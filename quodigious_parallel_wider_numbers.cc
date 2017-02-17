#include <iostream>
#include <cstdint>
#include <cmath>
#include <future>
#include <vector>

using u64 = uint64_t;
using vec64 = std::vector<u64>;
inline bool isQuodigious10(u64 value, u64 length) noexcept {
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

inline bool isQuodigious1000(u64 value, u64 length) noexcept {
	static constexpr auto count = 1000u;
	static constexpr auto digits = 3;
	static bool init = true;
	static bool predicates[count] = { 0 };
	static u64 sums[count] = { 0 };
	static u64 products[count] = { 0 };
	if (init) {
		init = false;
		for (int z = 0; z < 10; ++z) {
			for (int x = 0; x < 10; ++x) {
				for (int y = 0; y < 10; ++y) {
					auto index = (z * 100) + (x * 10) + y;
					sums[index] = x + y + z;
					products[index] = x * y * z;
					predicates[index] = ((x >= 2) && (y >= 2)) && (z >= 2);
				}
			}
		}
	}
	u64 len = length;
	u64 current = value;
	u64 sum = 0;
	u64 product = 1;
	auto remainder = length % digits;
	len -= remainder;
	for (u64 i = 0; i < remainder; ++i) {
		auto tmp = current % 10;
		if (tmp < 2) {
			return false;
		}
		sum += tmp;
		product *= tmp;
		current /= 10;
	}
	for (u64 i =0 ; i < len; i += digits) {
		u64 result = current % count;
		if (!predicates[result]) {
			return false;
		} else {
			product *= products[result];
			sum += sums[result];
			current /= count;
		}
	}
	return ((value % sum) == 0) && ((value % product) == 0);
}
int performQuodigiousCheck(u64 length, u64 start, u64 end, vec64& results) noexcept {
	auto fn = length > 8 ? isQuodigious1000 : isQuodigious10;
	for (auto value = start; value < end; ++value) {
		if (fn(value, length)) {
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
