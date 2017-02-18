#include <iostream>
#include <cstdint>
#include <cmath>
#include <future>
#include <vector>

using u64 = uint64_t;
using vec64 = std::vector<u64>;
template<u64 length>
inline bool isQuodigious10(u64 value) noexcept {
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

template<u64 length>
inline bool isQuodigious1000(u64 value) noexcept {
	static constexpr auto count = 1000u;
	static constexpr auto digits = 3;
	static constexpr auto remainder = length % digits;
	static constexpr auto len = length - remainder;
	static bool init = true;
	static bool predicates[count] = { 0 };
	static u64 sums[count] = { 0 };
	static u64 products[count] = { 0 };
	if (init) {
		init = false;
		for (int z = 0; z < 10; ++z) {
			auto zInd = z * 100;
			auto zPred = z >= 2;
			for (int x = 0; x < 10; ++x) {
				auto outerMul = z * x;
				auto combinedInd = zInd + (x * 10);
				auto outerSum = z + x;
				auto outerPredicate = ((x >= 2) && zPred);
				sums[combinedInd + 0] = outerSum;
				products[combinedInd + 0] = 0;
				predicates[combinedInd + 0] = false;
				sums[combinedInd + 1] = outerSum + 1;
				products[combinedInd + 1] = outerMul ;
				predicates[combinedInd + 1] = false;
				sums[combinedInd + 2] = outerSum + 2;
				products[combinedInd + 2] = outerMul * 2;
				predicates[combinedInd + 2] = outerPredicate;
				sums[combinedInd + 3] = outerSum + 3;
				products[combinedInd + 3] = outerMul * 3;
				predicates[combinedInd + 3] = outerPredicate; 
				sums[combinedInd + 4] = outerSum + 4;
				products[combinedInd + 4] = outerMul * 4;
				predicates[combinedInd + 4] = outerPredicate; 
				sums[combinedInd + 5] = outerSum + 5;
				products[combinedInd + 5] = outerMul * 5;
				predicates[combinedInd + 5] = outerPredicate;
				sums[combinedInd + 6] = outerSum + 6;
				products[combinedInd + 6] = outerMul * 6;
				predicates[combinedInd + 6] = outerPredicate;
				sums[combinedInd + 7] = outerSum + 7;
				products[combinedInd + 7] = outerMul * 7;
				predicates[combinedInd + 7] = outerPredicate;
				sums[combinedInd + 8] = outerSum + 8;
				products[combinedInd + 8] = outerMul * 8;
				predicates[combinedInd + 8] = outerPredicate;
				sums[combinedInd + 9] = outerSum + 9;
				products[combinedInd + 9] = outerMul * 9;
				predicates[combinedInd + 9] = outerPredicate;
			}
		}
	}
	u64 current = value;
	u64 sum = 0;
	u64 product = 1;
	// maximum of two right now
	if (remainder == 2) {
		auto tmp = current % 10;
		if (tmp < 2) {
			return false;
		}
		sum = tmp;
		product = tmp;
		current /= 10;

		tmp = current % 10;
		if (tmp < 2) {
			return false;
		}
		sum += tmp;
		product *= tmp;
		current /= 10;

	} else if (remainder == 1) {
		auto tmp = current % 10;
		if (tmp < 2) {
			return false;
		}
		sum = tmp;
		product = tmp;
		current /= 10;
	}
	for (u64 i = 0 ; i < len; i += digits) {
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

template<u64 length>
inline int performQuodigiousCheck(u64 start, u64 end, vec64& results) noexcept {
	if (length > 8) {
		for (auto value = start; value < end; ++value) {
			if (isQuodigious1000<length>(value)) {
				results.emplace_back(value);
			}
		}
	} else {
		for (auto value = start; value < end; ++value) {
			if (isQuodigious10<length>(value)) {
				results.emplace_back(value);
			}
		}
	}
	return 0;
}

void printout(vec64& l) noexcept {
	for (auto v : l) {
		std::cout << v << std::endl;
	}
	l.clear();
}

template<u64 length>
inline void body() noexcept {
	static vec64 l0, l1, l2, l3, l4, l5, l6, l7;
	static constexpr auto factor = 2.0 + (2.0 / 9.0);
	auto base = static_cast<u64>(pow(10, length - 1));
	auto st = static_cast<u64>(factor * base);

	auto fut0 = std::async(std::launch::async, [start = st, end = 3 * base]() { return performQuodigiousCheck<length>(start, end, l0); });
	auto fut1 = std::async(std::launch::async, [start = st + base, end = 4 * base]() { return performQuodigiousCheck<length>(start, end, l1); });
	auto fut2 = std::async(std::launch::async, [start = st + (base * 2), end = 5 * base]() { return performQuodigiousCheck<length>( start, end, l2); });

	auto fut3 = std::async(std::launch::async, [start = st + (base * 3), end = 6 * base]() { return (length > 8 && length < 14) ? 0 : performQuodigiousCheck<length>( start, end, l3); });

	auto fut4 = std::async(std::launch::async, [start = st + (base * 4), end = 7 * base]() { return performQuodigiousCheck<length>( start, end, l4); });
	auto fut5 = std::async(std::launch::async, [start = st + (base * 5), end = 8 * base]() { return performQuodigiousCheck<length>( start, end, l5); });
	auto fut6 = std::async(std::launch::async, [start = st + (base * 6), end = 9 * base]() { return performQuodigiousCheck<length>( start, end, l6); });
	auto fut7 = std::async(std::launch::async, [start = st + (base * 7), end = 10 * base]() { return performQuodigiousCheck<length>( start, end, l7); });
	fut0.get();
	fut1.get();
	fut2.get();
	fut3.get();
	fut4.get();
	fut5.get();
	fut6.get();
	fut7.get();
	printout(l0);
	printout(l1);
	printout(l2);
	printout(l3);
	printout(l4);
	printout(l5);
	printout(l6);
	printout(l7);
	std::cout << std::endl;
}

int main() {
	while(std::cin.good()) {
		u64 currentIndex = 0;
		std::cin >> currentIndex;
		if (std::cin.good()) {
			switch(currentIndex) {
				case 1: body<1>(); break;
				case 2: body<2>(); break;
				case 3: body<3>(); break;
				case 4: body<4>(); break;
				case 5: body<5>(); break;
				case 6: body<6>(); break;
				case 7: body<7>(); break;
				case 8: body<8>(); break;
				case 9: body<9>(); break;
				case 10: body<10>(); break;
				case 11: body<11>(); break;
				case 12: body<12>(); break;
				case 13: body<13>(); break;
				case 14: body<14>(); break;
				case 15: body<15>(); break;
				case 16: body<16>(); break;
				case 17: body<17>(); break;
				case 18: body<18>(); break;
				case 19: body<19>(); break;
				default:
					std::cerr << "Illegal index " << currentIndex << std::endl;
					return 1;
			}
		}
	}
	return 0;
}
