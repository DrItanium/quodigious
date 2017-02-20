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
#include <cmath>
#include <future>
#include <cstdint>
#include <vector>

using u64 = uint64_t;
using vec64 = std::vector<u64>;
template<u64 width> 
struct NotationDescription { 
	static constexpr u64 level3Digits = (width - 1) / 2;
	static constexpr u64 level2Digits = (width - 1) - level3Digits; // whats left over?
	static constexpr u64 level1Digits = 1;
	static_assert(width == (level3Digits + level2Digits + level1Digits), "Not enough digits defined!");
};
template<u64 width>
constexpr u64 level3Digits() noexcept {
	return NotationDescription<width>::level3Digits;
}
template<u64 width>
constexpr u64 level2Digits() noexcept {
	return NotationDescription<width>::level2Digits;
}
template<u64 width>
constexpr u64 level1Digits() noexcept {
	return NotationDescription<width>::level1Digits;
}
#define X(width, l3, l2, l1) \
	template<> \
struct NotationDescription< width > { \
	static constexpr u64 level3Digits = l3 ; \
	static constexpr u64 level2Digits = l2 ; \
	static constexpr u64 level1Digits = l1 ; \
};
#include "notations.def"
#undef X

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
constexpr auto Len2 = 100u;
u64 productsLen2[Len2] = { 0 };
bool predicatesLen2[Len2] = { false };

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
		auto kPred = k >= 2 && k != 5;
		auto kSum = k;
		auto kMul = k;
		auto kInd = (k * Len6);
		for (int h = 0; h < 10; ++h) {
			auto hPred = (h >= 2) && h != 5 && kPred ;
			auto hSum = h + kSum;
			auto hMul = h * kMul;
			auto hInd = (h * Len5) + kInd;
			for (int w = 0; w < 10; ++w) {
				auto wPred = (w >= 2) && w != 5&& hPred;
				auto wSum = w + hSum;
				auto wMul = w * hMul;
				auto wInd = (w * Len4) + hInd;
				for (int y = 0; y < 10; ++y) {
					auto yPred = (y >= 2) && y != 5&& wPred;
					auto ySum = y + wSum;
					auto yMul = y * wMul;
					auto yInd = (y * Len3) + wInd;
					for (int z = 0; z < 10; ++z) {
						auto zPred = z >= 2 && z != 5 && yPred;
						auto zSum = z + ySum;
						auto zMul = z * yMul;
						auto zInd = (z * 100) + yInd ;
						for (int x = 0; x < 10; ++x) {
							auto outerMul = x * zMul;
							auto combinedInd = (x * 10) + zInd;
							auto outerSum = x + zSum;
							auto outerPredicate = ((x >= 2) && x != 5 && zPred);
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
		auto wPred = w >= 2 && w != 5;
		auto wMul = w;
		auto wInd = (w * Len4);
		for (int y = 0; y < 10; ++y) {
			auto yPred = (y >= 2) && y != 5 && wPred;
			auto yMul = y * wMul;
			auto yInd = (y * Len3) + wInd;
			for (int z = 0; z < 10; ++z) {
				auto zPred = (z >= 2) && z != 5  && yPred;
				auto zMul = z * yMul;
				auto zInd = (z * 100) + yInd ;
				for (int x = 0; x < 10; ++x) {
					auto outerMul = x * zMul;
					auto combinedInd = (x * 10) + zInd;
					auto outerPredicate = ((x >= 2) && x != 5 && zPred);
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
		auto yPred = (y >= 2)  && y != 5;
		auto yMul = y ;
		auto yInd = (y * Len3) ;
		for (int z = 0; z < 10; ++z) {
			auto zPred = z >= 2 && yPred && z != 5;
			auto zMul = z * yMul;
			auto zInd = (z * 100) + yInd ;
			for (int x = 0; x < 10; ++x) {
				auto outerMul = x * zMul;
				auto combinedInd = (x * 10) + zInd;
				auto outerPredicate = ((x >= 2) && zPred && z != 5);
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
		auto zPred = z >= 2 && z != 5;
		auto zMul = z;
		auto zInd = (z * 100);
		for (int x = 0; x < 10; ++x) {
			auto outerMul = x * zMul;
			auto combinedInd = (x * 10) + zInd;
			auto outerPredicate = ((x >= 2) && x != 5 && zPred);
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
	// Len2
	for (int x = 0; x < 10; ++x) {
		auto outerMul = x;
		auto combinedInd = (x * 10);
		auto outerPredicate = ((x >= 2) && x != 5);
		productsLen2[combinedInd + 0] = 0;
		predicatesLen2[combinedInd + 0] = false;
		productsLen2[combinedInd + 1] = outerMul ;
		predicatesLen2[combinedInd + 1] = false;
		productsLen2[combinedInd + 2] = outerMul * 2;
		predicatesLen2[combinedInd + 2] = outerPredicate;
		productsLen2[combinedInd + 3] = outerMul * 3;
		predicatesLen2[combinedInd + 3] = outerPredicate; 
		productsLen2[combinedInd + 4] = outerMul * 4;
		predicatesLen2[combinedInd + 4] = outerPredicate; 
		productsLen2[combinedInd + 5] = outerMul * 5;
		predicatesLen2[combinedInd + 5] = outerPredicate;
		productsLen2[combinedInd + 6] = outerMul * 6;
		predicatesLen2[combinedInd + 6] = outerPredicate;
		productsLen2[combinedInd + 7] = outerMul * 7;
		predicatesLen2[combinedInd + 7] = outerPredicate;
		productsLen2[combinedInd + 8] = outerMul * 8;
		predicatesLen2[combinedInd + 8] = outerPredicate;
		productsLen2[combinedInd + 9] = outerMul * 9;
		predicatesLen2[combinedInd + 9] = outerPredicate;
	}
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
inline bool legalValue(u64 _) noexcept {
	static_assert(width < 9, "Too large of a legal value check!");
	return false;
}
template<> inline bool legalValue<1>(u64 x) noexcept { return x >= 2u && x != 5; }
template<> inline bool legalValue<2>(u64 x) noexcept { return predicatesLen2[x]; }
template<> inline bool legalValue<3>(u64 x) noexcept { return predicatesLen3[x]; }
template<> inline bool legalValue<4>(u64 x) noexcept { return predicatesLen4[x]; }
template<> inline bool legalValue<5>(u64 x) noexcept { return predicatesLen5[x]; }
template<> inline bool legalValue<6>(u64 x) noexcept { return predicatesLen6[x]; }
template<> inline bool legalValue<7>(u64 x) noexcept { return predicatesLen7[x]; }
template<> inline bool legalValue<8>(u64 x) noexcept { return legalValue<1>(x % 10u) && predicatesLen7[x / 10u]; }

template<u64 width>
inline u64 getProduct(u64 value) noexcept {
	static_assert(width < 9, "Too large of a product value!");
	return value;
}
template<> inline u64 getProduct<1>(u64 x) noexcept { return x; }
template<> inline u64 getProduct<2>(u64 x) noexcept { return productsLen2[x]; }
template<> inline u64 getProduct<3>(u64 x) noexcept { return productsLen3[x]; } 
template<> inline u64 getProduct<4>(u64 x) noexcept { return productsLen4[x]; } 
template<> inline u64 getProduct<5>(u64 x) noexcept { return productsLen5[x]; } 
template<> inline u64 getProduct<6>(u64 x) noexcept { return productsLen6[x]; } 
template<> inline u64 getProduct<7>(u64 x) noexcept { return productsLen7[x]; } 
template<> inline u64 getProduct<8>(u64 x) noexcept { return (x % 10u) * (getProduct<7>(x / 10u)); }

template<u64 width>
inline u64 getSum(u64 x) noexcept {
	static_assert(width < 20, "Can't express numbers 20 digits or higher!");
	switch(width) {
		case 0:
			return 0;
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
/**
 * This is used all over the place, it is the actual code to check to see if a
 * number is actuall quodigious. All of the work before hand is to get the sum
 * and products (if we get this far).
 */
inline constexpr bool performQCheck(u64 value, u64 sum, u64 prod) noexcept {
	return (value % sum == 0) && (value % prod == 0);
}

template<u64 length>
inline int performQuodigiousCheck(u64 start, u64 end, vec64& results) noexcept {
	// assume that we start at 2.222222222222
	// skip over the 9th and 10th numbers from this position!

	static constexpr auto outerDigits = level3Digits<length>();
	static constexpr auto innerDigits = level2Digits<length>();
	static constexpr auto innerMostDigits = level1Digits<length>();
	static constexpr auto innerMostShift = 0u;
	static constexpr auto lowerShift = innerMostDigits;
	static constexpr auto upperShift = innerDigits + innerMostDigits;
	static_assert(length == (outerDigits + innerDigits + innerMostDigits), "Defined digit layout does not encompass all digits of the given width, make sure that outer, inner, and innerMost equal the digit width!");
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
template<u64 width>
inline void printDigitalLayout() noexcept {
	std::cout << width << ":" << level3Digits<length>() << ":" << level2Digits<length>() << ":" << level1Digits<length>() << std::endl;
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
	static constexpr auto skip5 = length > 4;
	// this is not going to change ever!
	static constexpr auto base = fastPow10<length - 1>();
	static constexpr auto st = static_cast<u64>(factor * base);
	printDigitalLayout<length>();
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
	DefSimpleBody(4)
	DefSimpleBody(5)
	DefSimpleBody(6)
#undef DefSimpleBody
template<>
inline void body<3>() noexcept {
	// this is the only other qudigious range I know of which has a 5 in it
	// (besides 1) so just hard code it
	std::cout << 224 << std::endl;
	std::cout << 432 << std::endl;
	std::cout << 624 << std::endl;
	std::cout << 735 << std::endl;
}

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
						 //case 16: body<16>(); break;
						 //case 17: body<17>(); break;
						 //case 18: body<18>(); break;
						 //case 19: body<19>(); break;
				default:
						 std::cerr << "Illegal index " << currentIndex << std::endl;
						 return 1;
			}
		}
	}
	return 0;
}
