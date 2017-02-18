#include <iostream>
#include <cstdint>
#include <cmath>
#include <future>
#include <vector>

using u64 = uint64_t;
using vec64 = std::vector<u64>;

u64 sums100000[100000] = { 0 };
u64 products100000[100000] = { 0 };
bool predicates100000[100000] = { false };
u64 sums10000[10000] = { 0 };
u64 products10000[10000] = { 0 };
bool predicates10000[10000] = { false };
u64 sums1000[1000] = { 0 };
u64 products1000[1000] = { 0 };
bool predicates1000[1000] = { false };

inline void initialize() noexcept {
	// 100000
	for (int w = 0; w < 10; ++w) {
		auto wPred = w >= 2;
		auto wSum = w;
		auto wMul = w;
		auto wInd = (w * 10000);
		for (int y = 0; y < 10; ++y) {
			auto yPred = (y >= 2) && wPred;
			auto ySum = y + wSum;
			auto yMul = y * wMul;
			auto yInd = (y * 1000) + wInd;
			for (int z = 0; z < 10; ++z) {
				auto zPred = z >= 2 && yPred;
				auto zSum = z + ySum;
				auto zMul = z * yMul;
				auto zInd = (z * 100) + yInd ;
				for (int x = 0; x < 10; ++x) {
					auto outerMul = x * zMul;
					auto combinedInd = (x * 10) + zInd;
					auto outerSum = x + zSum;
					auto outerPredicate = ((x >= 2) && zPred);
					sums100000[combinedInd + 0] = outerSum;
					products100000[combinedInd + 0] = 0;
					predicates100000[combinedInd + 0] = false;
					sums100000[combinedInd + 1] = outerSum + 1;
					products100000[combinedInd + 1] = outerMul ;
					predicates100000[combinedInd + 1] = false;
					sums100000[combinedInd + 2] = outerSum + 2;
					products100000[combinedInd + 2] = outerMul * 2;
					predicates100000[combinedInd + 2] = outerPredicate;
					sums100000[combinedInd + 3] = outerSum + 3;
					products100000[combinedInd + 3] = outerMul * 3;
					predicates100000[combinedInd + 3] = outerPredicate; 
					sums100000[combinedInd + 4] = outerSum + 4;
					products100000[combinedInd + 4] = outerMul * 4;
					predicates100000[combinedInd + 4] = outerPredicate; 
					sums100000[combinedInd + 5] = outerSum + 5;
					products100000[combinedInd + 5] = outerMul * 5;
					predicates100000[combinedInd + 5] = outerPredicate;
					sums100000[combinedInd + 6] = outerSum + 6;
					products100000[combinedInd + 6] = outerMul * 6;
					predicates100000[combinedInd + 6] = outerPredicate;
					sums100000[combinedInd + 7] = outerSum + 7;
					products100000[combinedInd + 7] = outerMul * 7;
					predicates100000[combinedInd + 7] = outerPredicate;
					sums100000[combinedInd + 8] = outerSum + 8;
					products100000[combinedInd + 8] = outerMul * 8;
					predicates100000[combinedInd + 8] = outerPredicate;
					sums100000[combinedInd + 9] = outerSum + 9;
					products100000[combinedInd + 9] = outerMul * 9;
					predicates100000[combinedInd + 9] = outerPredicate;
				}
			}
		}
	}
	// 10000
	for (int y = 0; y < 10; ++y) {
		auto yPred = (y >= 2) ;
		auto ySum = y ;
		auto yMul = y ;
		auto yInd = (y * 1000) ;
		for (int z = 0; z < 10; ++z) {
			auto zPred = z >= 2 && yPred;
			auto zSum = z + ySum;
			auto zMul = z * yMul;
			auto zInd = (z * 100) + yInd ;
			for (int x = 0; x < 10; ++x) {
				auto outerMul = x * zMul;
				auto combinedInd = (x * 10) + zInd;
				auto outerSum = x + zSum;
				auto outerPredicate = ((x >= 2) && zPred);
				sums10000[combinedInd + 0] = outerSum;
				products10000[combinedInd + 0] = 0;
				predicates10000[combinedInd + 0] = false;
				sums10000[combinedInd + 1] = outerSum + 1;
				products10000[combinedInd + 1] = outerMul ;
				predicates10000[combinedInd + 1] = false;
				sums10000[combinedInd + 2] = outerSum + 2;
				products10000[combinedInd + 2] = outerMul * 2;
				predicates10000[combinedInd + 2] = outerPredicate;
				sums10000[combinedInd + 3] = outerSum + 3;
				products10000[combinedInd + 3] = outerMul * 3;
				predicates10000[combinedInd + 3] = outerPredicate; 
				sums10000[combinedInd + 4] = outerSum + 4;
				products10000[combinedInd + 4] = outerMul * 4;
				predicates10000[combinedInd + 4] = outerPredicate; 
				sums10000[combinedInd + 5] = outerSum + 5;
				products10000[combinedInd + 5] = outerMul * 5;
				predicates10000[combinedInd + 5] = outerPredicate;
				sums10000[combinedInd + 6] = outerSum + 6;
				products10000[combinedInd + 6] = outerMul * 6;
				predicates10000[combinedInd + 6] = outerPredicate;
				sums10000[combinedInd + 7] = outerSum + 7;
				products10000[combinedInd + 7] = outerMul * 7;
				predicates10000[combinedInd + 7] = outerPredicate;
				sums10000[combinedInd + 8] = outerSum + 8;
				products10000[combinedInd + 8] = outerMul * 8;
				predicates10000[combinedInd + 8] = outerPredicate;
				sums10000[combinedInd + 9] = outerSum + 9;
				products10000[combinedInd + 9] = outerMul * 9;
				predicates10000[combinedInd + 9] = outerPredicate;
			}
		}
	}
	// 1000
	for (int z = 0; z < 10; ++z) {
		auto zPred = z >= 2;
		auto zSum = z;
		auto zMul = z;
		auto zInd = (z * 100);
		for (int x = 0; x < 10; ++x) {
			auto outerMul = x * zMul;
			auto combinedInd = (x * 10) + zInd;
			auto outerSum = x + zSum;
			auto outerPredicate = ((x >= 2) && zPred);
			sums1000[combinedInd + 0] = outerSum;
			products1000[combinedInd + 0] = 0;
			predicates1000[combinedInd + 0] = false;
			sums1000[combinedInd + 1] = outerSum + 1;
			products1000[combinedInd + 1] = outerMul ;
			predicates1000[combinedInd + 1] = false;
			sums1000[combinedInd + 2] = outerSum + 2;
			products1000[combinedInd + 2] = outerMul * 2;
			predicates1000[combinedInd + 2] = outerPredicate;
			sums1000[combinedInd + 3] = outerSum + 3;
			products1000[combinedInd + 3] = outerMul * 3;
			predicates1000[combinedInd + 3] = outerPredicate; 
			sums1000[combinedInd + 4] = outerSum + 4;
			products1000[combinedInd + 4] = outerMul * 4;
			predicates1000[combinedInd + 4] = outerPredicate; 
			sums1000[combinedInd + 5] = outerSum + 5;
			products1000[combinedInd + 5] = outerMul * 5;
			predicates1000[combinedInd + 5] = outerPredicate;
			sums1000[combinedInd + 6] = outerSum + 6;
			products1000[combinedInd + 6] = outerMul * 6;
			predicates1000[combinedInd + 6] = outerPredicate;
			sums1000[combinedInd + 7] = outerSum + 7;
			products1000[combinedInd + 7] = outerMul * 7;
			predicates1000[combinedInd + 7] = outerPredicate;
			sums1000[combinedInd + 8] = outerSum + 8;
			products1000[combinedInd + 8] = outerMul * 8;
			predicates1000[combinedInd + 8] = outerPredicate;
			sums1000[combinedInd + 9] = outerSum + 9;
			products1000[combinedInd + 9] = outerMul * 9;
			predicates1000[combinedInd + 9] = outerPredicate;
		}
	}
}
inline constexpr bool performQCheck(u64 value, u64 sum, u64 prod) noexcept {
	return (value % sum == 0) && (value % prod == 0);
}
template<u64 length>
inline bool isQuodigious10(u64 value) noexcept {
	static constexpr auto divisor = 10u;
	u64 current = value;
	u64 sum = 0;
	u64 prod = 1;
	for (u64 i = 0u; i < length; ++i) {
		auto result = current % divisor;
		if (result < 2) {
			return false;
		} 
		sum += result;
		prod *= result;
		current /= divisor;
	}
	return performQCheck(value, sum, prod);
}


template<u64 length>
inline bool isQuodigious(u64 value) noexcept {
	static constexpr auto count = 10000u;
	static constexpr auto digits = 4;
	static constexpr auto remainder = length % digits;
	static constexpr auto len = length - remainder;
	u64 current = value;
	u64 sum = 0;
	u64 product = 1;
	if (remainder == 3) {
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

		tmp = current % 10;
		if (tmp < 2) {
			return false;
		}
		sum += tmp;
		product *= tmp;
		current /= 10;
	} else if (remainder == 2) {
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
		if (!predicates10000[result]) {
			return false;
		} else {
			product *= products10000[result];
			sum += sums10000[result];
			current /= count;
		}
	}
	return performQCheck(value, sum, product);
}

template<>
inline bool isQuodigious<10>(u64 value) noexcept {
	static constexpr auto count = 100000u;
	u64 current = value;
	//5
	auto result = current % count;
	if (!predicates100000[result]) {
		return false;
	} 
	u64 product = products100000[result];
	u64 sum = sums100000[result];
	current /= count;

	//10
	result = current % count;
	if (!predicates100000[result]) {
		return false;
	} 
	product *= products100000[result];
	sum += sums100000[result];

	return performQCheck(value, sum, product);
}

template<>
inline bool isQuodigious<15>(u64 value) noexcept {
	static constexpr auto count = 100000u;
	u64 current = value;
	//5
	auto result = current % count;
	if (!predicates100000[result]) {
		return false;
	} 
	u64 product = products100000[result];
	u64 sum = sums100000[result];
	current /= count;

	//10
	result = current % count;
	if (!predicates100000[result]) {
		return false;
	} 
	product *= products100000[result];
	sum += sums100000[result];
	current /= count;

	//15
	result = current % count;
	if (!predicates100000[result]) {
		return false;
	} 
	product *= products100000[result];
	sum += sums100000[result];

	return performQCheck(value, sum, product);
}

template<> inline bool isQuodigious<1>(u64 value) noexcept { return value >= 2; }
template<> 
inline bool isQuodigious<2>(u64 value) noexcept { 
	static constexpr auto divisor = 10u;
	auto result = value % divisor;
	if (result < 2) {
		return false;
	}
	u64 sum = result;
	u64 prod = result;
	u64 current = value / divisor;
	result = current % divisor;
	if (result < 2) {
		return false;
	}
	sum += result;
	prod *= result;
	return performQCheck(value, sum, prod);
}

template<> inline bool isQuodigious<3>(u64 value) noexcept { return predicates1000[value] && performQCheck(value, sums1000[value], products1000[value]); }
template<> inline bool isQuodigious<4>(u64 value) noexcept { return predicates10000[value] && performQCheck(value, sums10000[value], products10000[value]); }
template<> inline bool isQuodigious<5>(u64 value) noexcept { return predicates100000[value] && performQCheck(value, sums100000[value], products100000[value]); }


template<> 
inline bool isQuodigious<6>(u64 value) noexcept { 
	static constexpr auto divisor = 1000u;
	auto result = value % divisor;
	if (!predicates1000[result]) {
		return false;
	}
	u64 sum = sums1000[result];
	u64 product = products1000[result];
	u64 current = value / divisor;

	result = current % divisor;
	return predicates1000[result] && performQCheck(value, sum + sums1000[result], product * products1000[result]);
}

template<> 
inline bool isQuodigious<7>(u64 value) noexcept { 
	static constexpr auto divisor = 10u;
	auto result = value % divisor;
	if (result < 2) {
		return false;
	}
	u64 sum = result;
	u64 prod = result;
	u64 current = value / divisor;

	result = current % divisor;
	if (result < 2) {
		return false;
	}
	sum += result;
	prod *= result;
	current /= divisor;

	result = current % divisor;
	if (result < 2) {
		return false;
	}
	sum += result;
	prod *= result;
	current /= divisor;

	result = current % divisor;
	if (result < 2) {
		return false;
	}
	sum += result;
	prod *= result;
	current /= divisor;

	result = current % divisor;
	if (result < 2) {
		return false;
	}
	sum += result;
	prod *= result;
	current /= divisor;

	result = current % divisor;
	if (result < 2) {
		return false;
	}
	sum += result;
	prod *= result;
	current /= divisor;

	result = current % divisor;
	return result >= 2 && performQCheck(value, sum + result, prod * result);
}

template<> inline bool isQuodigious<8>(u64 value) noexcept { 
	static constexpr auto divisor = 10000u;
	auto result = value % divisor;
	if (!predicates10000[result]) {
		return false;
	}
	u64 sum = sums10000[result];
	u64 product = products10000[result];
	u64 current = value / divisor;

	result = current % divisor;
	return predicates10000[result] && performQCheck(value, sum + sums10000[result], product * products10000[result]);
}
template<> inline bool isQuodigious<9>(u64 value) noexcept { return isQuodigious10<9>(value); }

template<u64 length>
inline int performQuodigiousCheck(u64 start, u64 end, vec64& results) noexcept {
	for (auto value = start; value < end; ++value) {
		if (isQuodigious<length>(value)) {
			results.emplace_back(value);
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
	static constexpr auto skip5 = length > 8 && length < 14;
	auto base = static_cast<u64>(pow(10, length - 1));
	auto st = static_cast<u64>(factor * base);

	auto fut0 = std::async(std::launch::async, [start = st, end = 3 * base]() { return performQuodigiousCheck<length>(start, end, l0); });
	auto fut1 = std::async(std::launch::async, [start = st + base, end = 4 * base]() { return performQuodigiousCheck<length>(start, end, l1); });
	auto fut2 = std::async(std::launch::async, [start = st + (base * 2), end = 5 * base]() { return performQuodigiousCheck<length>( start, end, l2); });

	auto fut3 = std::async(std::launch::async, [start = st + (base * 3), end = 6 * base]() { return skip5 ? 0 : performQuodigiousCheck<length>( start, end, l3); });

	auto fut4 = std::async(std::launch::async, [start = st + (base * 4), end = 7 * base]() { return performQuodigiousCheck<length>( start, end, l4); });
	auto fut5 = std::async(std::launch::async, [start = st + (base * 5), end = 8 * base]() { return performQuodigiousCheck<length>( start, end, l5); });
	auto fut6 = std::async(std::launch::async, [start = st + (base * 6), end = 9 * base]() { return performQuodigiousCheck<length>( start, end, l6); });
	performQuodigiousCheck<length>(st + (base * 7), 10 * base, l7);
	fut0.get();
	fut1.get();
	fut2.get();
	fut3.get();
	fut4.get();
	fut5.get();
	fut6.get();
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
	initialize();
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
