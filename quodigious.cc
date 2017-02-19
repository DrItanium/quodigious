//  Copyright (c) 2017 Joshua Scoggins
//
//  This software is provided 'as-is', without any express or implied
//  warranty. In no event will the authors be held liable for any damages
//  arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any purpose,
//  including commercial applications, and to alter it and redistribute it
//  freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//  2. Altered source versions must be plainly marked as such, and must not be
//     misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source distribution.

// Perform quodigious checks on numbers using tons of different C++ tricks 
#include <iostream>
#include <cstdint>
#include <cmath>
#include <future>
#include <vector>

using u64 = uint64_t;
using vec64 = std::vector<u64>;

constexpr auto Len7 = 10000000u;
u64 sums[Len7] = { 0 };
u64 productsLen7[Len7] = { 0 };
bool predicatesLen7[Len7] = { false };
constexpr auto Len6 = 1000000u;
u64 productsLen6[Len6] = { 0 };
bool predicatesLen6[Len6] = { false };
constexpr auto Len5 = 100000u;
u64 productsLen5[Len5] = { 0 };
bool predicatesLen5[Len5] = { false };
constexpr auto Len4 = 10000u;
u64 productsLen4[Len4] = { 0 };
bool predicatesLen4[Len4] = { false };
constexpr auto Len3 = 1000u;
u64 productsLen3[Len3] = { 0 };
bool predicatesLen3[Len3] = { false };

inline void initialize() noexcept {
	// precompute all of the sums and products for 7 digit numbers and below (not 100 or 10 though!)
	// It is super fast to do and only consumes space. That way when we iterate
	// through numbers we can reduce the number of divides, remainders, adds,
	// and multiplies greatly!.
	// we can also express more digits in terms of smaller digit groups
	//
	// There is only one sum array because those values will not change
	// regardless of number width. However, the product and predicate lists
	// must be separate because the extra digits (like 0) are still there and
	// must be accounted for. For example, the number 222 is legal when there
	// are three digits, however when it is four digits then it is 0222 and
	// thus illegal (also the product becomes zero!). Thus we have separate
	// lists for each number width when dealing with products and predicates
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
							sums[combinedInd + 0] = outerSum;
							productsLen7[combinedInd + 0] = 0;
							predicatesLen7[combinedInd + 0] = false;
							sums[combinedInd + 1] = outerSum + 1;
							productsLen7[combinedInd + 1] = outerMul ;
							predicatesLen7[combinedInd + 1] = false;
							sums[combinedInd + 2] = outerSum + 2;
							productsLen7[combinedInd + 2] = outerMul * 2;
							predicatesLen7[combinedInd + 2] = outerPredicate;
							sums[combinedInd + 3] = outerSum + 3;
							productsLen7[combinedInd + 3] = outerMul * 3;
							predicatesLen7[combinedInd + 3] = outerPredicate; 
							sums[combinedInd + 4] = outerSum + 4;
							productsLen7[combinedInd + 4] = outerMul * 4;
							predicatesLen7[combinedInd + 4] = outerPredicate; 
							sums[combinedInd + 5] = outerSum + 5;
							productsLen7[combinedInd + 5] = outerMul * 5;
							predicatesLen7[combinedInd + 5] = outerPredicate;
							sums[combinedInd + 6] = outerSum + 6;
							productsLen7[combinedInd + 6] = outerMul * 6;
							predicatesLen7[combinedInd + 6] = outerPredicate;
							sums[combinedInd + 7] = outerSum + 7;
							productsLen7[combinedInd + 7] = outerMul * 7;
							predicatesLen7[combinedInd + 7] = outerPredicate;
							sums[combinedInd + 8] = outerSum + 8;
							productsLen7[combinedInd + 8] = outerMul * 8;
							predicatesLen7[combinedInd + 8] = outerPredicate;
							sums[combinedInd + 9] = outerSum + 9;
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
			auto wMul = w * hMul;
			auto wInd = (w * Len4) + hInd;
			for (int y = 0; y < 10; ++y) {
				auto yPred = (y >= 2) && wPred;
				auto yMul = y * wMul;
				auto yInd = (y * Len3) + wInd;
				for (int z = 0; z < 10; ++z) {
					auto zPred = z >= 2 && yPred;
					auto zMul = z * yMul;
					auto zInd = (z * 100) + yInd ;
					for (int x = 0; x < 10; ++x) {
						auto outerMul = x * zMul;
						auto combinedInd = (x * 10) + zInd;
						auto outerPredicate = ((x >= 2) && zPred);
						productsLen6[combinedInd + 0] = 0;
						predicatesLen6[combinedInd + 0] = false;
						productsLen6[combinedInd + 1] = outerMul ;
						predicatesLen6[combinedInd + 1] = false;
						productsLen6[combinedInd + 2] = outerMul * 2;
						predicatesLen6[combinedInd + 2] = outerPredicate;
						productsLen6[combinedInd + 3] = outerMul * 3;
						predicatesLen6[combinedInd + 3] = outerPredicate; 
						productsLen6[combinedInd + 4] = outerMul * 4;
						predicatesLen6[combinedInd + 4] = outerPredicate; 
						productsLen6[combinedInd + 5] = outerMul * 5;
						predicatesLen6[combinedInd + 5] = outerPredicate;
						productsLen6[combinedInd + 6] = outerMul * 6;
						predicatesLen6[combinedInd + 6] = outerPredicate;
						productsLen6[combinedInd + 7] = outerMul * 7;
						predicatesLen6[combinedInd + 7] = outerPredicate;
						productsLen6[combinedInd + 8] = outerMul * 8;
						predicatesLen6[combinedInd + 8] = outerPredicate;
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
		auto wMul = w;
		auto wInd = (w * Len4);
		for (int y = 0; y < 10; ++y) {
			auto yPred = (y >= 2) && wPred;
			auto yMul = y * wMul;
			auto yInd = (y * Len3) + wInd;
			for (int z = 0; z < 10; ++z) {
				auto zPred = z >= 2 && yPred;
				auto zMul = z * yMul;
				auto zInd = (z * 100) + yInd ;
				for (int x = 0; x < 10; ++x) {
					auto outerMul = x * zMul;
					auto combinedInd = (x * 10) + zInd;
					auto outerPredicate = ((x >= 2) && zPred);
					productsLen5[combinedInd + 0] = 0;
					predicatesLen5[combinedInd + 0] = false;
					productsLen5[combinedInd + 1] = outerMul ;
					predicatesLen5[combinedInd + 1] = false;
					productsLen5[combinedInd + 2] = outerMul * 2;
					predicatesLen5[combinedInd + 2] = outerPredicate;
					productsLen5[combinedInd + 3] = outerMul * 3;
					predicatesLen5[combinedInd + 3] = outerPredicate; 
					productsLen5[combinedInd + 4] = outerMul * 4;
					predicatesLen5[combinedInd + 4] = outerPredicate; 
					productsLen5[combinedInd + 5] = outerMul * 5;
					predicatesLen5[combinedInd + 5] = outerPredicate;
					productsLen5[combinedInd + 6] = outerMul * 6;
					predicatesLen5[combinedInd + 6] = outerPredicate;
					productsLen5[combinedInd + 7] = outerMul * 7;
					predicatesLen5[combinedInd + 7] = outerPredicate;
					productsLen5[combinedInd + 8] = outerMul * 8;
					predicatesLen5[combinedInd + 8] = outerPredicate;
					productsLen5[combinedInd + 9] = outerMul * 9;
					predicatesLen5[combinedInd + 9] = outerPredicate;
				}
			}
		}
	}
	// Len4
	for (int y = 0; y < 10; ++y) {
		auto yPred = (y >= 2) ;
		auto yMul = y ;
		auto yInd = (y * Len3) ;
		for (int z = 0; z < 10; ++z) {
			auto zPred = z >= 2 && yPred;
			auto zMul = z * yMul;
			auto zInd = (z * 100) + yInd ;
			for (int x = 0; x < 10; ++x) {
				auto outerMul = x * zMul;
				auto combinedInd = (x * 10) + zInd;
				auto outerPredicate = ((x >= 2) && zPred);
				productsLen4[combinedInd + 0] = 0;
				predicatesLen4[combinedInd + 0] = false;
				productsLen4[combinedInd + 1] = outerMul ;
				predicatesLen4[combinedInd + 1] = false;
				productsLen4[combinedInd + 2] = outerMul * 2;
				predicatesLen4[combinedInd + 2] = outerPredicate;
				productsLen4[combinedInd + 3] = outerMul * 3;
				predicatesLen4[combinedInd + 3] = outerPredicate; 
				productsLen4[combinedInd + 4] = outerMul * 4;
				predicatesLen4[combinedInd + 4] = outerPredicate; 
				productsLen4[combinedInd + 5] = outerMul * 5;
				predicatesLen4[combinedInd + 5] = outerPredicate;
				productsLen4[combinedInd + 6] = outerMul * 6;
				predicatesLen4[combinedInd + 6] = outerPredicate;
				productsLen4[combinedInd + 7] = outerMul * 7;
				predicatesLen4[combinedInd + 7] = outerPredicate;
				productsLen4[combinedInd + 8] = outerMul * 8;
				predicatesLen4[combinedInd + 8] = outerPredicate;
				productsLen4[combinedInd + 9] = outerMul * 9;
				predicatesLen4[combinedInd + 9] = outerPredicate;
			}
		}
	}
	// Len3
	for (int z = 0; z < 10; ++z) {
		auto zPred = z >= 2;
		auto zMul = z;
		auto zInd = (z * 100);
		for (int x = 0; x < 10; ++x) {
			auto outerMul = x * zMul;
			auto combinedInd = (x * 10) + zInd;
			auto outerPredicate = ((x >= 2) && zPred);
			productsLen3[combinedInd + 0] = 0;
			predicatesLen3[combinedInd + 0] = false;
			productsLen3[combinedInd + 1] = outerMul ;
			predicatesLen3[combinedInd + 1] = false;
			productsLen3[combinedInd + 2] = outerMul * 2;
			predicatesLen3[combinedInd + 2] = outerPredicate;
			productsLen3[combinedInd + 3] = outerMul * 3;
			predicatesLen3[combinedInd + 3] = outerPredicate; 
			productsLen3[combinedInd + 4] = outerMul * 4;
			predicatesLen3[combinedInd + 4] = outerPredicate; 
			productsLen3[combinedInd + 5] = outerMul * 5;
			predicatesLen3[combinedInd + 5] = outerPredicate;
			productsLen3[combinedInd + 6] = outerMul * 6;
			predicatesLen3[combinedInd + 6] = outerPredicate;
			productsLen3[combinedInd + 7] = outerMul * 7;
			predicatesLen3[combinedInd + 7] = outerPredicate;
			productsLen3[combinedInd + 8] = outerMul * 8;
			predicatesLen3[combinedInd + 8] = outerPredicate;
			productsLen3[combinedInd + 9] = outerMul * 9;
			predicatesLen3[combinedInd + 9] = outerPredicate;
		}
	}
}
/**
 * This is used all over the place, it is the actual code to check to see if a
 * number is actuall quodigious. All of the work before hand is to get the sum
 * and products (if we get this far).
 */
inline constexpr bool performQCheck(u64 value, u64 sum, u64 prod) noexcept {
	return (value % sum == 0) && (value % prod == 0);
}

template<u64 length>
inline bool isQuodigious(u64 value) noexcept {
	// If not explicitly specified, we use this much slower implementation of
	// quodigious but carving out sections that are 4 digits wide. The problem
	// with this default implementation is that it is SLOW becuase of the loop
	// at the bottom as well as the remainder checks before hand. I have
	// optimized it as much as possible but since it is general purpose it
	// requires more operations to complete compared to the specialized
	// versions.
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
		sum = sums[result];
		product = productsLen3[result]; 
		current /= Len3;
	} else if (remainder == 2) {
		auto tmp = current % 10;
		if (tmp < 2) {
			return false;
		}
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
		} 
		product *= productsLen4[result];
		sum += sums[result];
		current /= count; // there will be an extra wasted divide because it won't be used!
	}
	return performQCheck(value, sum, product);
}


template<> inline bool isQuodigious<8>(u64 value) noexcept { 
	// 4 + 4
	auto result = value % Len4;
	if (!predicatesLen4[result]) {
		return false;
	}
	auto sum = sums[result];
	auto product = productsLen4[result];
	auto previous = result;
	auto current = value / Len4;

	result = current % Len4;
	if (previous == result) {
		return performQCheck(value, sum + sum, product * product);
	} else {
		return predicatesLen4[result] && performQCheck(value, sum + sums[result], product * productsLen4[result]);
	}
}

template<> inline bool isQuodigious<9>(u64 value) noexcept { 
	// 6 + 3
	auto result = value % Len3;
	if (!predicatesLen3[result]) {
		return false;
	}
	auto sum = sums[result];
	auto product = productsLen3[result];
	auto current = value / Len3;
	result = current % Len6;
	return predicatesLen6[result] && performQCheck(value, sum + sums[result], product * productsLen6[result]);
}

template<>
inline bool isQuodigious<10>(u64 value) noexcept {
	// 5 + 5
	static constexpr auto count = Len5;
	//5
	auto result = value % count;
	if (!predicatesLen5[result]) {
		return false;
	} 
	auto product = productsLen5[result];
	auto sum = sums[result];
	auto previous = result;
	auto current = value / count;

	//10
	result = current % count;
	if (previous == result) {
		// if the two halves are the same and we got this far then we don't
		// need to check again!
		return performQCheck(value, sum + sum, product * product);
	} else {
		// if they aren't equal, then perform actions as normal
		return predicatesLen5[result] && performQCheck(value, sum + sums[result], product * productsLen5[result]);
	}
}


template<>
inline bool isQuodigious<11>(u64 value) noexcept {
	// 6 + 5
	auto result = value % Len6;
	if (!predicatesLen6[result]) {
		return false;
	}
	auto product = productsLen6[result];
	auto sum = sums[result];
	auto current = value / Len6;

	result = current % Len5;
	return predicatesLen5[result] && performQCheck(value, sum + sums[result], product * productsLen5[result]);
}

template<>
inline bool isQuodigious<12>(u64 value) noexcept {
	// 6 + 6
	static constexpr auto count = Len6;
	//6
	auto result = value % count;
	if (!predicatesLen6[result]) {
		return false;
	} 
	auto product = productsLen6[result];
	auto sum = sums[result];
	auto previous = result;
	auto current = value / count;

	//12
	result = current % count;
	if (previous == result) {
		return performQCheck(value, sum + sum, product * product);
	} else {
		return predicatesLen6[result] && performQCheck(value, sum + sums[result], product * productsLen6[result]);
	}
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
	auto sum = sums[result];
	auto current = value / Len6;

	// 13
	result = current % Len7;
	return predicatesLen7[result] && performQCheck(value, sum + sums[result], product * productsLen7[result]);
}

template<>
inline bool isQuodigious<14>(u64 value) noexcept {
	// 7 + 7
	//7
	auto result = value % Len7;
	if (!predicatesLen7[result]) {
		return false;
	} 
	auto product = productsLen7[result];
	auto sum = sums[result];
	auto current = value / Len7;

	// 14
	result = current % Len7;
	return predicatesLen7[result] && performQCheck(value, sum + sums[result], product * productsLen7[result]);
}

template<>
inline bool isQuodigious<15>(u64 value) noexcept {
	//5 + 5 + 5
	auto result = value % Len5;
	if (!predicatesLen5[result]) {
		return false;
	}
	u64 product = productsLen5[result];
	u64 sum = sums[result];
	u64 current = value / Len5;
	//8
	result = current % Len5;
	if (!predicatesLen5[result]) {
		return false;
	} 
	product *= productsLen5[result];
	sum += sums[result];
	current /= Len5;

	//15
	result = current % Len5;
	return predicatesLen5[result] && performQCheck(value, sum + sums[result], product * productsLen5[result]);
}

template<u64 length>
inline int performQuodigiousCheck(u64 start, u64 end, vec64& results) noexcept {
	// assume that we start at 2.222222222222
	// skip over the 9th and 10th numbers from this position!
	auto fn = [&results](u64 value) { if (isQuodigious<length>(value)) { results.emplace_back(value); }};
	for (auto value = start; value < end; value += 10) {
		// if we can compute the upper portion of the number ahead of time and
		// see if it is legal then we could save a ton on checking digits
		fn(value+0);
		fn(value+1);
		fn(value+2);
		fn(value+3);
		fn(value+4);
		fn(value+5);
		fn(value+6);
		fn(value+7);
	}
	return 0;
}
template<u64 length>
inline constexpr u64 fastPow10() noexcept {
	return fastPow10<length - 1>() * 10;
}
template<>
inline constexpr u64 fastPow10<0>() noexcept {
	return 1;
}
template<u64 width>
inline bool legalValue(u64 value) noexcept {
	static_assert(width < 9, "Too large of a legal value check!");
	static_assert(width != 0, "Zero not allowed!");
	return false;
}
template<> inline bool legalValue<1>(u64 x) noexcept { return x >= 2u; }
template<> inline bool legalValue<2>(u64 x) noexcept { return x > 21u && ((x % 10u) >= 2u); }
template<> inline bool legalValue<3>(u64 x) noexcept { return predicatesLen3[x]; }
template<> inline bool legalValue<4>(u64 x) noexcept { return predicatesLen4[x]; }
template<> inline bool legalValue<5>(u64 x) noexcept { return predicatesLen5[x]; }
template<> inline bool legalValue<6>(u64 x) noexcept { return predicatesLen6[x]; }
template<> inline bool legalValue<7>(u64 x) noexcept { return predicatesLen7[x]; }
template<> inline bool legalValue<8>(u64 x) noexcept { return legalValue<1>(x % 10u) && predicatesLen7[x / 10u]; }

template<u64 width>
inline u64 getProduct(u64 value) noexcept {
	static_assert(width < 9, "Too large of a product value!");
	static_assert(width != 0, "Zero not allowed!");
	return value;
}
template<> inline u64 getProduct<1>(u64 x) noexcept { return x; }
template<> inline u64 getProduct<2>(u64 x) noexcept { return (x % 10u) * (x / 10u); }
template<> inline u64 getProduct<3>(u64 x) noexcept { return productsLen3[x]; } 
template<> inline u64 getProduct<4>(u64 x) noexcept { return productsLen4[x]; } 
template<> inline u64 getProduct<5>(u64 x) noexcept { return productsLen5[x]; } 
template<> inline u64 getProduct<6>(u64 x) noexcept { return productsLen6[x]; } 
template<> inline u64 getProduct<7>(u64 x) noexcept { return productsLen7[x]; } 
template<> inline u64 getProduct<8>(u64 x) noexcept { return (x % 10u) * (getProduct<7>(x / 10u)); }

template<u64 width>
inline u64 getSum(u64 x) noexcept {
	static_assert(width != 0, "Can't have a zero width number!");
	static_assert(width < 20, "Can't express numbers 20 digits or higher!");
	switch(width) {
		case 1:
			return x;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			return sums[x];
		case 8:
			return getSum<1>(x % fastPow10<1>()) + getSum<7>(x / fastPow10<1>());
		case 9:
			return getSum<2>(x % fastPow10<2>()) + getSum<7>(x / fastPow10<2>());
		case 10:
			return getSum<3>(x % fastPow10<3>()) + getSum<7>(x / fastPow10<3>());
		case 11:
			return getSum<4>(x % fastPow10<4>()) + getSum<7>(x / fastPow10<4>());
		case 12:
			return getSum<5>(x % fastPow10<5>()) + getSum<7>(x / fastPow10<5>());
		case 13:
			return getSum<6>(x % fastPow10<6>()) + getSum<7>(x / fastPow10<6>());
		case 14:
			return getSum<7>(x % fastPow10<7>()) + getSum<7>(x / fastPow10<7>());
		case 15:
			return getSum<1>(x % fastPow10<1>()) + getSum<14>(x / fastPow10<1>());
		case 16:
			return getSum<2>(x % fastPow10<2>()) + getSum<14>(x / fastPow10<2>());
		case 17:
			return getSum<3>(x % fastPow10<3>()) + getSum<14>(x / fastPow10<3>());
		case 18:
			return getSum<4>(x % fastPow10<4>()) + getSum<14>(x / fastPow10<4>());
		case 19:
			return getSum<5>(x % fastPow10<5>()) + getSum<14>(x / fastPow10<5>());
		default:
			throw "Illegal width requested!";
	}
}

template<u64 width, u64 outerDigits, u64 innerDigits, u64 innerMostDigits = 1, u64 upperShift = innerDigits + innerMostDigits, u64 lowerShift = 1, u64 innerMostShift = 0>
inline void quodigiousCheckBody(u64 start, u64 end, vec64& results) noexcept {
	static_assert(width == (outerDigits + innerDigits + innerMostDigits), "Defined digit layout does not encompass all digits of the given width, make sure that outer, inner, and innerMost equal the digit width!");
	static constexpr auto outerFactor = fastPow10<outerDigits>();
	static constexpr auto innerFactor = fastPow10<innerDigits>();
	static constexpr auto innerMostFactor = fastPow10<innerMostDigits>();
	static constexpr auto upperSection = fastPow10<upperShift>();
	static constexpr auto lowerSection = fastPow10<lowerShift>();
	static constexpr auto innerMostSection = fastPow10<innerMostShift>();
	auto startInnerMost = start % innerMostFactor;
	auto current = start / innerMostFactor;
	auto startInner = current % innerFactor;
	current /= innerFactor;
	auto startOuter = current % outerFactor;

	auto endInnerMost = end % innerMostFactor;
	if (endInnerMost == 0) {
		endInnerMost = innerMostFactor;
	}

	auto endInner = ((end / innerMostFactor) % innerFactor);
	if (endInner == 0) {
		endInner = innerFactor;
	}


	auto endOuter = ((end / innerMostFactor) / innerFactor) % outerFactor;
	if (endOuter == 0) {
		endOuter = outerFactor;
	}
	for (auto i = startOuter; i < endOuter; ++i) {
		if (legalValue<outerDigits>(i)) {
			auto upperSum = getSum<outerDigits>(i);
			auto upperProduct = getProduct<outerDigits>(i);
			auto upperIndex = i * upperSection;
			for (auto j = startInner; j < endInner; ++j) {
				if (legalValue<innerDigits>(j)) {
					auto innerSum = getSum<innerDigits>(j) + upperSum;
					auto innerProduct = getProduct<innerDigits>(j) * upperProduct;
					auto innerIndex = (j * lowerSection) + upperIndex;
					for (auto k = startInnerMost; k < endInnerMost; ++k) {
						if (legalValue<innerMostDigits>(k)) {
							auto product = innerProduct * getProduct<innerMostDigits>(k);
							auto sum = innerSum + getSum<innerMostDigits>(k);
							auto value = innerIndex + (k * innerMostSection);
							if (performQCheck(value, sum, product)) {
								results.emplace_back(value);
							}
						}
					}
				}
			}
		}
	}
}

template<u64 width, 
	u64 level4Digits, 
	u64 level3Digits, 
	u64 level2Digits, 
	u64 level1Digits = 1, 
	u64 level4Shift = level3Digits + level2Digits + level1Digits, 
	u64 level3Shift = level2Digits + level1Digits,
	u64 level2Shift = level1Digits,
	u64 level1Shift = 0>
inline void quodigiousCheckBody4Levels(u64 start, u64 end, vec64& results) noexcept {
	static_assert(width == (level4Digits + level3Digits + level2Digits + level1Digits), "Defined digit layout does not encompass all digits of the given width, make sure that l4, l3, l2, and l1 digits equal the digits width!");
	static constexpr auto l4Factor = fastPow10<level4Digits>();
	static constexpr auto l4Section = fastPow10<level4Shift>();
	static constexpr auto l3Factor = fastPow10<level3Digits>();
	static constexpr auto l3Section = fastPow10<level3Shift>();
	static constexpr auto l2Factor = fastPow10<level2Digits>();
	static constexpr auto l2Section = fastPow10<level2Shift>();
	static constexpr auto l1Factor = fastPow10<level1Digits>();
	static constexpr auto l1Section = fastPow10<level1Shift>();
	auto startL1 = start % l1Factor;
	auto current = start / l1Factor;
	auto startL2 = current % l2Factor;
	current /= l2Factor;
	auto startL3 = current % l3Factor;
	current /= l3Factor;
	auto startL4 = current % l4Factor;

	auto endL1 = end % l1Factor;
	if (endL1 == 0) {
		endL1 = l1Factor;
	}
	current = end / l1Factor;

	auto endL2 = ((current) % l2Factor);
	if (endL2 == 0) {
		endL2 = l2Factor;
	}
	current /= l2Factor;


	auto endL3 = current % l3Factor;
	if (endL3 == 0) {
		endL3 = l3Factor;
	}
	current /= l4Factor;
	auto endL4 = current % l4Factor;
	if (endL4 == 0) {
		endL4 = l4Factor;
	}
	for (auto l4 = startL4; l4 < endL4; ++l4) {
		if (legalValue<level4Digits>(l4)) {
			auto l4Sum = getSum<level4Digits>(l4);
			auto l4Product = getProduct<level4Digits>(l4);
			auto l4Index = l4 * l4Section;
			for (auto l3 = startL3 ; l3 < endL3 ; ++l3) {
				if (legalValue<level3Digits>(l3)) {
					auto l3Sum = getSum<level3Digits>(l3) + l4Sum;
					auto l3Product = getProduct<level3Digits>(l3) * l4Product;
					auto l3Index = (l3 * l3Section) + l4Index;
					for (auto l2 = startL2 ; l2 < endL2; ++l2) {
						if (legalValue<level2Digits>(l2)) {
							auto l2Sum = getSum<level2Digits>(l2) + l3Sum;
							auto l2Product = getProduct<level2Digits>(l2) * l3Product;
							auto l2Index = (l2 * l2Section) + l3Index;
							for (auto l1 = startL1; l1 < endL1; ++l1) {
								if (legalValue<level1Digits>(l1)) {
									auto product = l2Product * getProduct<level1Digits>(l1);
									auto sum = l2Sum + getSum<level1Digits>(l1);
									auto value = l2Index + (l1 * l1Section);
									if (performQCheck(value, sum, product)) {
										results.emplace_back(value);
									}
								}
							}
						}
					}
				}
			}
		}
	}

}


template<>
inline int performQuodigiousCheck<7>(u64 start, u64 end, vec64& results) noexcept {
	for (auto value = start; value < end; ++value) {
		if (predicatesLen7[value] && performQCheck(value, sums[value], productsLen7[value])) {
			results.emplace_back(value);
		}
	}
	return 0;
}

template<>
inline int performQuodigiousCheck<10>(u64 start, u64 end, vec64& results) noexcept {
	// compute the upper, lower, and 1 bit locations
	// this should always be at position 2!
	quodigiousCheckBody<10, 5, 4>(start, end, results);
	return 0;
}


template<>
inline int performQuodigiousCheck<11>(u64 start, u64 end, vec64& results) noexcept {
	// -------------
	// | 5 | 6 | 1 |
	// -------------
	quodigiousCheckBody<11, 5, 5>(start, end, results);
	return 0;
}



template<>
inline int performQuodigiousCheck<12>(u64 start, u64 end, vec64& results) noexcept {
	// -------------
	// | 5 | 6 | 1 |
	// -------------
	quodigiousCheckBody<12, 5, 6>(start, end, results);
	return 0;
}

template<>
inline int performQuodigiousCheck<13>(u64 start, u64 end, vec64& results) noexcept {
	// -------------
	// | 6 | 6 | 1 |
	// -------------
	quodigiousCheckBody<13, 6, 6>(start, end, results);
	return 0;
}

template<>
inline int performQuodigiousCheck<14>(u64 start, u64 end, vec64& results) noexcept {
	// -------------
	// | 6 | 7 | 1 |
	// -------------
	quodigiousCheckBody<14, 6, 7>(start, end, results);
	return 0;
}




void printout(vec64& l) noexcept {
	for (auto v : l) {
		std::cout << v << std::endl;
	}
	l.clear();
}

template<u64 length>
inline void singleThreadedSimpleBody() noexcept {
	static constexpr auto factor = 2.0 + (2.0 / 9.0);
	for (auto value = (factor * fastPow10<length - 1>()); value < fastPow10<length>(); ++value) {
		if (legalValue<length>(value) && performQCheck(value, getSum<length>(value), getProduct<length>(value))) {
			std::cout << value << std::endl;
		}
	}
	std::cout << std::endl;
}

template<u64 length>
inline void body() noexcept {
	static vec64 l0, l1, l2, l3, l4, l5, l6, l7;
	static constexpr auto factor = 2.0 + (2.0 / 9.0);
	static constexpr auto skip5 = length > 4 && length < 14;
	// this is not going to change ever!
	static constexpr auto base = fastPow10<length - 1>();
	static constexpr auto st = static_cast<u64>(factor * base);

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

#define DefSimpleBody(ind) template<> inline void body< ind >() noexcept { singleThreadedSimpleBody< ind >(); }
	DefSimpleBody(2)
	DefSimpleBody(3)
	DefSimpleBody(4)
	DefSimpleBody(5)
	DefSimpleBody(6)
#undef DefSimpleBody

template<>
inline void body<1>() noexcept {
	std::cout << 2 << std::endl;
	std::cout << 3 << std::endl;
	std::cout << 4 << std::endl;
	std::cout << 5 << std::endl;
	std::cout << 6 << std::endl;
	std::cout << 7 << std::endl;
	std::cout << 8 << std::endl;
	std::cout << 9 << std::endl;
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
