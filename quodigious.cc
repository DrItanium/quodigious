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
#include <functional>
#include "qlib.h"

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
constexpr auto Len1 = 10u;
template<bool includeFives>
inline constexpr bool isLegalDigit(u64 value) noexcept {
    auto baseResult = value >=2;
    if (!includeFives) {
        baseResult = baseResult && value != 5;
    }
    return baseResult;
}
template<u64 value, bool includeFives>
inline constexpr bool isLegalDigitSingular() noexcept {
    static_assert(value<= 9, "Offset shouldn't be larger than 9!");
    return value >= 2 && (includeFives ? true : value != 5);
}
template<u64 offset>
inline constexpr u64 indexOffset(u64 value) noexcept {
    return value * offset;
}
template<u64 offset, bool includeFives, bool updateSums = false>
inline void updateTables(u64 baseOffset, u64 actualSum, u64 actualProduct, bool actualPredicate, u64* sums, u64* product, bool* predicates) noexcept {
    static_assert(offset <= 9, "Offset shouldn't be larger than 9!");
    static constexpr auto innerPredicate = isLegalDigitSingular<offset, includeFives>();
    if (updateSums) {
        sums[baseOffset + offset] = actualSum + offset;
    }
    product[baseOffset + offset] = actualProduct * offset;
    predicates[baseOffset + offset] = innerPredicate && actualPredicate;
}

template<bool includeFives, bool updateSums = false>
inline void updateTables10(u64 offset, u64 sum, u64 product, bool legal, u64* sums, u64* products, bool* predicates) noexcept {
    updateTables<0, includeFives, updateSums>(offset, sum, product, legal, sums, products, predicates);
    updateTables<1, includeFives, updateSums>(offset, sum, product, legal, sums, products, predicates);
    updateTables<2, includeFives, updateSums>(offset, sum, product, legal, sums, products, predicates);
    updateTables<3, includeFives, updateSums>(offset, sum, product, legal, sums, products, predicates);
    updateTables<4, includeFives, updateSums>(offset, sum, product, legal, sums, products, predicates);
    updateTables<5, includeFives, updateSums>(offset, sum, product, legal, sums, products, predicates);
    updateTables<6, includeFives, updateSums>(offset, sum, product, legal, sums, products, predicates);
    updateTables<7, includeFives, updateSums>(offset, sum, product, legal, sums, products, predicates);
    updateTables<8, includeFives, updateSums>(offset, sum, product, legal, sums, products, predicates);
    updateTables<9, includeFives, updateSums>(offset, sum, product, legal, sums, products, predicates);
}

template<bool includeFives = false>
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
    static constexpr auto check = isLegalDigit<includeFives>;
	for (int k = 0; k < 10; ++k) {
		auto kPred = check(k);
		auto kSum = k;
		auto kMul = k;
		auto kInd = indexOffset<Len6>(k);
		for (int h = 0; h < 10; ++h) {
			auto hPred = check(h) && kPred;
			auto hSum = h + kSum;
			auto hMul = h * kMul;
			auto hInd = indexOffset<Len5>(h) + kInd;
			for (int w = 0; w < 10; ++w) {
				auto wPred = check(w) && hPred;
				auto wSum = w + hSum;
				auto wMul = w * hMul;
				auto wInd = indexOffset<Len4>(w) + hInd;
				for (int y = 0; y < 10; ++y) {
                    auto yPred = check(y) && wPred;
					auto ySum = y + wSum;
					auto yMul = y * wMul;
					auto yInd = indexOffset<Len3>(y) + wInd;
					for (int z = 0; z < 10; ++z) {
						auto zPred = check(z) && yPred;
						auto zSum = z + ySum;
						auto zMul = z * yMul;
						auto zInd = indexOffset<Len2>(z) + yInd;
						for (int x = 0; x < 10; ++x) {
							auto outerMul = x * zMul;
							auto combinedInd = indexOffset<Len1>(x) + zInd;
							auto outerSum = x + zSum;
							auto outerPredicate = check(x) && zPred;
                            updateTables10<includeFives, true>(combinedInd, outerSum, outerMul, outerPredicate, sums, productsLen7, predicatesLen7);
						}
					}
				}
			}
		}
	}
	// Len6
	for (int h = 0; h < 10; ++h) {
        auto hPred = check(h);
		auto hSum = h;
		auto hMul = h;
		auto hInd = indexOffset<Len5>(h);
		for (int w = 0; w < 10; ++w) {
			auto wPred = check(w) && hPred;
			auto wMul = w * hMul;
			auto wInd = indexOffset<Len4>(w) + hInd;
			for (int y = 0; y < 10; ++y) {
                auto yPred = check(y) && wPred;
				auto yMul = y * wMul;
				auto yInd = indexOffset<Len3>(y) + wInd;
				for (int z = 0; z < 10; ++z) {
                    auto zPred = check(z) && yPred;
					auto zMul = z * yMul;
					auto zInd = indexOffset<Len2>(z) + yInd;
					for (int x = 0; x < 10; ++x) {
						auto outerMul = x * zMul;
						auto combinedInd = indexOffset<Len1>(x) + zInd;
                        auto outerPredicate = check(x) && zPred;
                        updateTables10<includeFives>(combinedInd, 0u, outerMul, outerPredicate, sums, productsLen6, predicatesLen6);
					}
				}
			}
		}
	}
	// Len5
	for (int w = 0; w < 10; ++w) {
		auto wPred = check(w);
		auto wMul = w;
		auto wInd = indexOffset<Len4>(w);
		for (int y = 0; y < 10; ++y) {
			auto yPred = check(y) && wPred;
			auto yMul = y * wMul;
			auto yInd = indexOffset<Len3>(y) + wInd;
			for (int z = 0; z < 10; ++z) {
				auto zPred = check(z) && yPred;
				auto zMul = z * yMul;
				auto zInd = indexOffset<Len2>(z) + yInd;
				for (int x = 0; x < 10; ++x) {
					auto outerMul = x * zMul;
					auto combinedInd = indexOffset<Len1>(x) + zInd;
					auto outerPredicate = check(x) && zPred;
                    updateTables10<includeFives>(combinedInd, 0u, outerMul, outerPredicate, sums, productsLen5, predicatesLen5);
				}
			}
		}
	}
	// Len4
	for (int y = 0; y < 10; ++y) {
		auto yPred = check(y);
		auto yMul = y ;
		auto yInd = indexOffset<Len3>(y);
		for (int z = 0; z < 10; ++z) {
			auto zPred = check(z) && yPred;
			auto zMul = z * yMul;
			auto zInd = indexOffset<Len2>(z) + yInd;
			for (int x = 0; x < 10; ++x) {
				auto outerMul = x * zMul;
				auto combinedInd = indexOffset<Len1>(x) + zInd;
				auto outerPredicate = check(x) && zPred;
                updateTables10<includeFives>(combinedInd, 0u, outerMul, outerPredicate, sums, productsLen4, predicatesLen4);
			}
		}
	}
	// Len3
	for (int z = 0; z < 10; ++z) {
        auto zPred = check(z);
		auto zMul = z;
        auto zInd = indexOffset<Len2>(z);
		for (int x = 0; x < 10; ++x) {
			auto outerMul = x * zMul;
			auto combinedInd = indexOffset<Len1>(x) + zInd;
			auto outerPredicate = check(x) && zPred;
            updateTables10<includeFives>(combinedInd, 0u, outerMul, outerPredicate, sums, productsLen3, predicatesLen3);
		}
	}
	// Len2
	for (int x = 0; x < 10; ++x) {
		auto outerMul = x;
		auto combinedInd = indexOffset<Len1>(x);
		auto outerPredicate = check(x);
        updateTables10<includeFives>(combinedInd, 0u, outerMul, outerPredicate, sums, productsLen2, predicatesLen2);
	}
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
		case 0: return 0;
		case 1: return x;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7: return sums[x];
		case 8: return getSum<1>(x % fastPow10<1>()) + getSum<7>(x / fastPow10<1>());
		case 9: return getSum<2>(x % fastPow10<2>()) + getSum<7>(x / fastPow10<2>());
		case 10: return getSum<3>(x % fastPow10<3>()) + getSum<7>(x / fastPow10<3>());
		case 11: return getSum<4>(x % fastPow10<4>()) + getSum<7>(x / fastPow10<4>());
		case 12: return getSum<5>(x % fastPow10<5>()) + getSum<7>(x / fastPow10<5>());
		case 13: return getSum<6>(x % fastPow10<6>()) + getSum<7>(x / fastPow10<6>());
		case 14: return getSum<7>(x % fastPow10<7>()) + getSum<7>(x / fastPow10<7>());
		case 15: return getSum<1>(x % fastPow10<1>()) + getSum<14>(x / fastPow10<1>());
		case 16: return getSum<2>(x % fastPow10<2>()) + getSum<14>(x / fastPow10<2>());
		case 17: return getSum<3>(x % fastPow10<3>()) + getSum<14>(x / fastPow10<3>());
		case 18: return getSum<4>(x % fastPow10<4>()) + getSum<14>(x / fastPow10<4>());
		case 19: return getSum<5>(x % fastPow10<5>()) + getSum<14>(x / fastPow10<5>());
		default: throw "Illegal width requested!";
	}
}

template<u64 length>
constexpr u64 startIndex() noexcept {
	return static_cast<u64>(shaveFactor * fastPow10<length - 1>());
}
template<u64 length>
constexpr u64 endIndex() noexcept {
	return fastPow10<length>();
}
template<u64 length, u64 start, u64 end>
inline int performQuodigiousCheck(vec64& results) noexcept {
	// assume that we start at 2.222222222222
	// skip over the 9th and 10th numbers from this position!
	if (length == 7) {
		for (auto value = start; value < end; ++value) {
			if (predicatesLen7[value] && isQuodigious(value, sums[value], productsLen7[value])) {
				results.emplace_back(value);
			}
		}
	} else {
		// precompute the fuck out of all of this!
		// Compilers hate me, I am the TEMPLATE MASTER
		static constexpr auto l3Digits = level3Digits<length>();
		static constexpr auto l2Digits = level2Digits<length>();
		static constexpr auto l1Digits = level1Digits<length>();
		static constexpr auto l1Shift = 0u;
		static constexpr auto l2Shift = l1Digits;
		static constexpr auto l3Shift = l2Digits + l1Digits;
		static_assert(length == (l3Digits + l2Digits + l1Digits), "Defined digit layout does not encompass all digits of the given width, make sure that outer, inner, and innerMost equal the digit width!");
		static constexpr auto l3Factor = fastPow10<l3Digits>();
		static constexpr auto l2Factor = fastPow10<l2Digits>();
		static constexpr auto l1Factor = fastPow10<l1Digits>();
		static constexpr auto l3Section = fastPow10<l3Shift>();
		static constexpr auto l2Section = fastPow10<l2Shift>();
		static constexpr auto l1Section = fastPow10<l1Shift>();
		static constexpr auto startL1 = start % l1Factor;
		static constexpr auto startL2 = (start / l1Factor) % l2Factor;
		static constexpr auto startL3 = ((start / l1Factor) / l2Factor) % l3Factor;
		static constexpr auto attemptEndL1 = end % l1Factor;
		static constexpr auto endL1 = attemptEndL1 == 0 ? l1Factor : attemptEndL1;
		static constexpr auto attemptEndL2 = ((end / l1Factor) % l2Factor);
		static constexpr auto endL2 = attemptEndL2 == 0 ? l2Factor : attemptEndL2;
		static constexpr auto attemptEndL3 = ((end / l1Factor) / l2Factor) % l3Factor;
		static constexpr auto endL3 = attemptEndL3 == 0 ? l3Factor : attemptEndL3;

		for (auto i = startL3; i < endL3; ++i) {
			if (legalValue<l3Digits>(i)) {
				auto l3Sum = getSum<l3Digits>(i);
				auto l3Product = getProduct<l3Digits>(i);
				auto l3Index = indexOffset<l3Section>(i);
				for (auto j = startL2; j < endL2; ++j) {
					if (legalValue<l2Digits>(j)) {
						auto l2Sum = getSum<l2Digits>(j) + l3Sum;
						auto l2Product = getProduct<l2Digits>(j) * l3Product;
						auto l2Index = indexOffset<l2Section>(j) + l3Index;
						for (auto k = startL1; k < endL1; ++k) {
							if (legalValue<l1Digits>(k)) {
								auto l1Product = l2Product * getProduct<l1Digits>(k);
								auto l1Sum = l2Sum + getSum<l1Digits>(k);
                                auto l1Value = indexOffset<l1Section>(k) + l2Index;
								if (isQuodigious(l1Value, l1Sum, l1Product)) {
									results.emplace_back(l1Value);
								}
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

#ifdef DEBUG
template<u64 width>
inline void printDigitalLayout() noexcept {
	std::cout << width << ":" << level3Digits<width>() << ":" << level2Digits<width>() << ":" << level1Digits<width>() << std::endl;
}
#endif


void printout(vec64& l) noexcept {
	for (auto v : l) {
		std::cout << v << std::endl;
	}
	l.clear();
}

template<u64 length>
inline void singleThreadedSimpleBody() noexcept {
	for (auto value = static_cast<u64>(shaveFactor* fastPow10<length - 1>()); value < fastPow10<length>(); ++value) {
		if (legalValue<length>(value) && isQuodigious(value, getSum<length>(value), getProduct<length>(value))) {
			std::cout << value << std::endl;
		}
	}
	std::cout << std::endl;
}

template<u64 length>
inline void body() noexcept {
	static vec64 l0, l1, l2, l3, l4, l5, l6, l7;
	static constexpr auto skip5 = length > 4;
	// this is not going to change ever!
	static constexpr auto base = fastPow10<length - 1>();
	static constexpr auto st = static_cast<u64>(shaveFactor * base);
#ifdef DEBUG
	printDigitalLayout<length>();
#endif
	auto fut0 = std::async(std::launch::async, []() { return performQuodigiousCheck<length, st, 3 * base>(l0); });
	auto fut1 = std::async(std::launch::async, []() { return performQuodigiousCheck<length, st + base, 4 * base>(l1); });
	auto fut2 = std::async(std::launch::async, []() { return performQuodigiousCheck<length, st + (base * 2), 5 * base>(l2); });
	auto fut3 = std::async(std::launch::async, []() { return skip5 ? 0 : performQuodigiousCheck<length, st + (base * 3), 6 * base>(l3); });
	auto fut4 = std::async(std::launch::async, []() { return performQuodigiousCheck<length, st + (base * 4), 7 * base>(l4); });
	auto fut5 = std::async(std::launch::async, []() { return performQuodigiousCheck<length, st + (base * 5), 8 * base>(l5); });
	auto fut6 = std::async(std::launch::async, []() { return performQuodigiousCheck<length, st + (base * 6), 9 * base>(l6); });
	performQuodigiousCheck<length, st + (base * 7), 10 * base>(l7);
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
	DefSimpleBody(7);
#undef DefSimpleBody
template<>
inline void body<3>() noexcept {
	// this is the only other qudigious range I know of which has a 5 in it
	// (besides 1) so just hard code it
	std::cout << 224 << std::endl;
	std::cout << 432 << std::endl;
	std::cout << 624 << std::endl;
	std::cout << 735 << std::endl;
	std::cout << std::endl;
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
