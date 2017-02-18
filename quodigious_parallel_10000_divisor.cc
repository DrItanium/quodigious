#include <iostream>
#include <cstdint>
#include <cmath>
#include <future>
#include <vector>

using u64 = uint64_t;
using vec64 = std::vector<u64>;


constexpr auto Len7 = 10000000u;
u64 sumsLen7[Len7] = { 0 };
u64 productsLen7[Len7] = { 0 };
bool predicatesLen7[Len7] = { false };
constexpr auto Len6 = 1000000u;
u64 sumsLen6[Len6] = { 0 };
u64 productsLen6[Len6] = { 0 };
bool predicatesLen6[Len6] = { false };
constexpr auto Len5 = 100000u;
u64 sumsLen5[Len5] = { 0 };
u64 productsLen5[Len5] = { 0 };
bool predicatesLen5[Len5] = { false };
constexpr auto Len4 = 10000u;
u64 sumsLen4[Len4] = { 0 };
u64 productsLen4[Len4] = { 0 };
bool predicatesLen4[Len4] = { false };
constexpr auto Len3 = 1000u;
u64 sumsLen3[Len3] = { 0 };
u64 productsLen3[Len3] = { 0 };
bool predicatesLen3[Len3] = { false };

inline void initialize() noexcept {
	// Len7
	for (int k = 0; k < 10; ++k) {
		auto kPred = k >= 2;
		auto kSum = k;
		auto kMul = k;
		auto kInd = (k * Len6);
		for (int h = 0; h < 10; ++h) {
			auto hPred = (h >= 2) && kPred ;
			auto hSum = h + kSum;
			auto hMul = h * kMul;
			auto hInd = (h * Len5) + kInd;
			for (int w = 0; w < 10; ++w) {
				auto wPred = (w >= 2) && hPred;
				auto wSum = w + hSum;
				auto wMul = w * hMul;
				auto wInd = (w * Len4) + hInd;
				for (int y = 0; y < 10; ++y) {
					auto yPred = (y >= 2) && wPred;
					auto ySum = y + wSum;
					auto yMul = y * wMul;
					auto yInd = (y * Len3) + wInd;
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
							sumsLen7[combinedInd + 0] = outerSum;
							productsLen7[combinedInd + 0] = 0;
							predicatesLen7[combinedInd + 0] = false;
							sumsLen7[combinedInd + 1] = outerSum + 1;
							productsLen7[combinedInd + 1] = outerMul ;
							predicatesLen7[combinedInd + 1] = false;
							sumsLen7[combinedInd + 2] = outerSum + 2;
							productsLen7[combinedInd + 2] = outerMul * 2;
							predicatesLen7[combinedInd + 2] = outerPredicate;
							sumsLen7[combinedInd + 3] = outerSum + 3;
							productsLen7[combinedInd + 3] = outerMul * 3;
							predicatesLen7[combinedInd + 3] = outerPredicate; 
							sumsLen7[combinedInd + 4] = outerSum + 4;
							productsLen7[combinedInd + 4] = outerMul * 4;
							predicatesLen7[combinedInd + 4] = outerPredicate; 
							sumsLen7[combinedInd + 5] = outerSum + 5;
							productsLen7[combinedInd + 5] = outerMul * 5;
							predicatesLen7[combinedInd + 5] = outerPredicate;
							sumsLen7[combinedInd + 6] = outerSum + 6;
							productsLen7[combinedInd + 6] = outerMul * 6;
							predicatesLen7[combinedInd + 6] = outerPredicate;
							sumsLen7[combinedInd + 7] = outerSum + 7;
							productsLen7[combinedInd + 7] = outerMul * 7;
							predicatesLen7[combinedInd + 7] = outerPredicate;
							sumsLen7[combinedInd + 8] = outerSum + 8;
							productsLen7[combinedInd + 8] = outerMul * 8;
							predicatesLen7[combinedInd + 8] = outerPredicate;
							sumsLen7[combinedInd + 9] = outerSum + 9;
							productsLen7[combinedInd + 9] = outerMul * 9;
							predicatesLen7[combinedInd + 9] = outerPredicate;
						}
					}
				}
			}
		}
	}
	// Len6
	for (int h = 0; h < 10; ++h) {
		auto hPred = h >= 2;
		auto hSum = h;
		auto hMul = h;
		auto hInd = (h * Len5);
		for (int w = 0; w < 10; ++w) {
			auto wPred = (w >= 2) && hPred;
			auto wSum = w + hSum;
			auto wMul = w * hMul;
			auto wInd = (w * Len4) + hInd;
			for (int y = 0; y < 10; ++y) {
				auto yPred = (y >= 2) && wPred;
				auto ySum = y + wSum;
				auto yMul = y * wMul;
				auto yInd = (y * Len3) + wInd;
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
						sumsLen6[combinedInd + 0] = outerSum;
						productsLen6[combinedInd + 0] = 0;
						predicatesLen6[combinedInd + 0] = false;
						sumsLen6[combinedInd + 1] = outerSum + 1;
						productsLen6[combinedInd + 1] = outerMul ;
						predicatesLen6[combinedInd + 1] = false;
						sumsLen6[combinedInd + 2] = outerSum + 2;
						productsLen6[combinedInd + 2] = outerMul * 2;
						predicatesLen6[combinedInd + 2] = outerPredicate;
						sumsLen6[combinedInd + 3] = outerSum + 3;
						productsLen6[combinedInd + 3] = outerMul * 3;
						predicatesLen6[combinedInd + 3] = outerPredicate; 
						sumsLen6[combinedInd + 4] = outerSum + 4;
						productsLen6[combinedInd + 4] = outerMul * 4;
						predicatesLen6[combinedInd + 4] = outerPredicate; 
						sumsLen6[combinedInd + 5] = outerSum + 5;
						productsLen6[combinedInd + 5] = outerMul * 5;
						predicatesLen6[combinedInd + 5] = outerPredicate;
						sumsLen6[combinedInd + 6] = outerSum + 6;
						productsLen6[combinedInd + 6] = outerMul * 6;
						predicatesLen6[combinedInd + 6] = outerPredicate;
						sumsLen6[combinedInd + 7] = outerSum + 7;
						productsLen6[combinedInd + 7] = outerMul * 7;
						predicatesLen6[combinedInd + 7] = outerPredicate;
						sumsLen6[combinedInd + 8] = outerSum + 8;
						productsLen6[combinedInd + 8] = outerMul * 8;
						predicatesLen6[combinedInd + 8] = outerPredicate;
						sumsLen6[combinedInd + 9] = outerSum + 9;
						productsLen6[combinedInd + 9] = outerMul * 9;
						predicatesLen6[combinedInd + 9] = outerPredicate;
					}
				}
			}
		}
	}
	// Len5
	for (int w = 0; w < 10; ++w) {
		auto wPred = w >= 2;
		auto wSum = w;
		auto wMul = w;
		auto wInd = (w * Len4);
		for (int y = 0; y < 10; ++y) {
			auto yPred = (y >= 2) && wPred;
			auto ySum = y + wSum;
			auto yMul = y * wMul;
			auto yInd = (y * Len3) + wInd;
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
					sumsLen5[combinedInd + 0] = outerSum;
					productsLen5[combinedInd + 0] = 0;
					predicatesLen5[combinedInd + 0] = false;
					sumsLen5[combinedInd + 1] = outerSum + 1;
					productsLen5[combinedInd + 1] = outerMul ;
					predicatesLen5[combinedInd + 1] = false;
					sumsLen5[combinedInd + 2] = outerSum + 2;
					productsLen5[combinedInd + 2] = outerMul * 2;
					predicatesLen5[combinedInd + 2] = outerPredicate;
					sumsLen5[combinedInd + 3] = outerSum + 3;
					productsLen5[combinedInd + 3] = outerMul * 3;
					predicatesLen5[combinedInd + 3] = outerPredicate; 
					sumsLen5[combinedInd + 4] = outerSum + 4;
					productsLen5[combinedInd + 4] = outerMul * 4;
					predicatesLen5[combinedInd + 4] = outerPredicate; 
					sumsLen5[combinedInd + 5] = outerSum + 5;
					productsLen5[combinedInd + 5] = outerMul * 5;
					predicatesLen5[combinedInd + 5] = outerPredicate;
					sumsLen5[combinedInd + 6] = outerSum + 6;
					productsLen5[combinedInd + 6] = outerMul * 6;
					predicatesLen5[combinedInd + 6] = outerPredicate;
					sumsLen5[combinedInd + 7] = outerSum + 7;
					productsLen5[combinedInd + 7] = outerMul * 7;
					predicatesLen5[combinedInd + 7] = outerPredicate;
					sumsLen5[combinedInd + 8] = outerSum + 8;
					productsLen5[combinedInd + 8] = outerMul * 8;
					predicatesLen5[combinedInd + 8] = outerPredicate;
					sumsLen5[combinedInd + 9] = outerSum + 9;
					productsLen5[combinedInd + 9] = outerMul * 9;
					predicatesLen5[combinedInd + 9] = outerPredicate;
				}
			}
		}
	}
	// Len4
	for (int y = 0; y < 10; ++y) {
		auto yPred = (y >= 2) ;
		auto ySum = y ;
		auto yMul = y ;
		auto yInd = (y * Len3) ;
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
				sumsLen4[combinedInd + 0] = outerSum;
				productsLen4[combinedInd + 0] = 0;
				predicatesLen4[combinedInd + 0] = false;
				sumsLen4[combinedInd + 1] = outerSum + 1;
				productsLen4[combinedInd + 1] = outerMul ;
				predicatesLen4[combinedInd + 1] = false;
				sumsLen4[combinedInd + 2] = outerSum + 2;
				productsLen4[combinedInd + 2] = outerMul * 2;
				predicatesLen4[combinedInd + 2] = outerPredicate;
				sumsLen4[combinedInd + 3] = outerSum + 3;
				productsLen4[combinedInd + 3] = outerMul * 3;
				predicatesLen4[combinedInd + 3] = outerPredicate; 
				sumsLen4[combinedInd + 4] = outerSum + 4;
				productsLen4[combinedInd + 4] = outerMul * 4;
				predicatesLen4[combinedInd + 4] = outerPredicate; 
				sumsLen4[combinedInd + 5] = outerSum + 5;
				productsLen4[combinedInd + 5] = outerMul * 5;
				predicatesLen4[combinedInd + 5] = outerPredicate;
				sumsLen4[combinedInd + 6] = outerSum + 6;
				productsLen4[combinedInd + 6] = outerMul * 6;
				predicatesLen4[combinedInd + 6] = outerPredicate;
				sumsLen4[combinedInd + 7] = outerSum + 7;
				productsLen4[combinedInd + 7] = outerMul * 7;
				predicatesLen4[combinedInd + 7] = outerPredicate;
				sumsLen4[combinedInd + 8] = outerSum + 8;
				productsLen4[combinedInd + 8] = outerMul * 8;
				predicatesLen4[combinedInd + 8] = outerPredicate;
				sumsLen4[combinedInd + 9] = outerSum + 9;
				productsLen4[combinedInd + 9] = outerMul * 9;
				predicatesLen4[combinedInd + 9] = outerPredicate;
			}
		}
	}
	// Len3
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
			sumsLen3[combinedInd + 0] = outerSum;
			productsLen3[combinedInd + 0] = 0;
			predicatesLen3[combinedInd + 0] = false;
			sumsLen3[combinedInd + 1] = outerSum + 1;
			productsLen3[combinedInd + 1] = outerMul ;
			predicatesLen3[combinedInd + 1] = false;
			sumsLen3[combinedInd + 2] = outerSum + 2;
			productsLen3[combinedInd + 2] = outerMul * 2;
			predicatesLen3[combinedInd + 2] = outerPredicate;
			sumsLen3[combinedInd + 3] = outerSum + 3;
			productsLen3[combinedInd + 3] = outerMul * 3;
			predicatesLen3[combinedInd + 3] = outerPredicate; 
			sumsLen3[combinedInd + 4] = outerSum + 4;
			productsLen3[combinedInd + 4] = outerMul * 4;
			predicatesLen3[combinedInd + 4] = outerPredicate; 
			sumsLen3[combinedInd + 5] = outerSum + 5;
			productsLen3[combinedInd + 5] = outerMul * 5;
			predicatesLen3[combinedInd + 5] = outerPredicate;
			sumsLen3[combinedInd + 6] = outerSum + 6;
			productsLen3[combinedInd + 6] = outerMul * 6;
			predicatesLen3[combinedInd + 6] = outerPredicate;
			sumsLen3[combinedInd + 7] = outerSum + 7;
			productsLen3[combinedInd + 7] = outerMul * 7;
			predicatesLen3[combinedInd + 7] = outerPredicate;
			sumsLen3[combinedInd + 8] = outerSum + 8;
			productsLen3[combinedInd + 8] = outerMul * 8;
			predicatesLen3[combinedInd + 8] = outerPredicate;
			sumsLen3[combinedInd + 9] = outerSum + 9;
			productsLen3[combinedInd + 9] = outerMul * 9;
			predicatesLen3[combinedInd + 9] = outerPredicate;
		}
	}
}
inline constexpr bool performQCheck(u64 value, u64 sum, u64 prod) noexcept {
	return (value % sum == 0) && (value % prod == 0);
}

template<u64 length>
inline bool isQuodigious(u64 value) noexcept {
	static constexpr auto count = Len4;
	static constexpr auto digits = 4;
	static constexpr auto remainder = length % digits;
	static constexpr auto len = length - remainder;
	u64 current = value;
	u64 sum = 0;
	u64 product = 1;
	if (remainder == 3) {
		auto result = current % Len3;
		if (!predicatesLen3[result]) {
			return false;
		}
		sum = sumsLen3[result];
		product = productsLen3[result]; 
		current /= Len3;
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
		if (!predicatesLen4[result]) {
			return false;
		} else {
			product *= productsLen4[result];
			sum += sumsLen4[result];
			current /= count;
		}
	}
	return performQCheck(value, sum, product);
}

template<>
inline bool isQuodigious<10>(u64 value) noexcept {
	static constexpr auto count = Len5;
	//5
	auto result = value % count;
	if (!predicatesLen5[result]) {
		return false;
	} 
	auto product = productsLen5[result];
	auto sum = sumsLen5[result];
	auto current = value / count;

	//10
	result = current % count;
	return predicatesLen5[result] && performQCheck(value, sum + sumsLen5[result], product * productsLen5[result]);
}


template<>
inline bool isQuodigious<11>(u64 value) noexcept {
	// 6 + 5
	auto result = value % Len6;
	if (!predicatesLen6[result]) {
		return false;
	}
	auto product = productsLen6[result];
	auto sum = sumsLen6[result];
	auto current = value / Len6;

	result = current % Len5;
	return predicatesLen5[result] && performQCheck(value, sum + sumsLen5[result], product * productsLen5[result]);
}

template<>
inline bool isQuodigious<12>(u64 value) noexcept {
	static constexpr auto count = Len6;
	//6
	auto result = value % count;
	if (!predicatesLen6[result]) {
		return false;
	} 
	auto product = productsLen6[result];
	auto sum = sumsLen6[result];
	auto current = value / count;

	//12
	result = current % count;
	return predicatesLen6[result] && performQCheck(value, sum + sumsLen6[result], product * productsLen6[result]);
}

template<>
inline bool isQuodigious<13>(u64 value) noexcept {
	// 6 + 7
	//6
	auto result = value % Len6;
	if (!predicatesLen6[result]) {
		return false;
	} 
	auto product = productsLen6[result];
	auto sum = sumsLen6[result];
	auto current = value / Len6;

	// 13
	result = current % Len7;
	return predicatesLen7[result] && performQCheck(value, sum + sumsLen7[result], product * productsLen7[result]);
}

template<>
inline bool isQuodigious<15>(u64 value) noexcept {
	static constexpr auto count = Len5;
	u64 current = value;
	//5
	auto result = current % count;
	if (!predicatesLen5[result]) {
		return false;
	} 
	u64 product = productsLen5[result];
	u64 sum = sumsLen5[result];
	current /= count;

	//10
	result = current % count;
	if (!predicatesLen5[result]) {
		return false;
	} 
	product *= productsLen5[result];
	sum += sumsLen5[result];
	current /= count;

	//15
	result = current % count;
	return predicatesLen5[result] && performQCheck(value, sum + sumsLen5[result], product * productsLen5[result]);
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
	return (result >= 2) &&performQCheck(value, sum + result, prod * result);
}

template<> inline bool isQuodigious<3>(u64 value) noexcept { return predicatesLen3[value]    && performQCheck(value, sumsLen3[value],    productsLen3[value]); }
template<> inline bool isQuodigious<4>(u64 value) noexcept { return predicatesLen4[value]   && performQCheck(value, sumsLen4[value],   productsLen4[value]); }
template<> inline bool isQuodigious<5>(u64 value) noexcept { return predicatesLen5[value]  && performQCheck(value, sumsLen5[value],  productsLen5[value]); }
template<> inline bool isQuodigious<6>(u64 value) noexcept { return predicatesLen6[value] && performQCheck(value, sumsLen6[value], productsLen6[value]); }
template<> inline bool isQuodigious<7>(u64 value) noexcept { return predicatesLen7[value] && performQCheck(value, sumsLen7[value], productsLen7[value]); }


template<> inline bool isQuodigious<8>(u64 value) noexcept { 
	static constexpr auto divisor = Len4;
	auto result = value % divisor;
	if (!predicatesLen4[result]) {
		return false;
	}
	u64 sum = sumsLen4[result];
	u64 product = productsLen4[result];
	u64 current = value / divisor;

	result = current % divisor;
	return predicatesLen4[result] && performQCheck(value, sum + sumsLen4[result], product * productsLen4[result]);
}
template<> inline bool isQuodigious<9>(u64 value) noexcept { 
	// 6 + 3
	auto result = value % Len6;
	if (!predicatesLen6[result]) {
		return false;
	}
	auto sum = sumsLen6[result];
	auto product = productsLen6[result];
	auto current = value / Len6;
	result = current % Len3;
	return predicatesLen3[result] && performQCheck(value, sum + sumsLen3[result], product * productsLen3[result]);
}

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
