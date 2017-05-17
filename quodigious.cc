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

#define buildLengthPrecomputation(title, width) \
	constexpr auto title = fastPow10< width > ; \
	u64 products ## title [ title ] = { 0 }; \
	bool predicates ## title [ title ] = { false }
buildLengthPrecomputation(Len8, 8);
buildLengthPrecomputation(Len7, 7);
buildLengthPrecomputation(Len6, 6);
buildLengthPrecomputation(Len5, 5);
buildLengthPrecomputation(Len4, 4);
buildLengthPrecomputation(Len3, 3);
buildLengthPrecomputation(Len2, 2);
constexpr auto Len1 = fastPow10<1>;

u64 *sums = nullptr;
//u64 sums[Len8] = { 0 };

template<bool includeFives>
constexpr bool isLegalDigit(u64 value) noexcept {
    return value >= 2 && (includeFives ? true : value != 5);
}
template<u64 offset>
constexpr u64 indexOffset(u64 value) noexcept {
    return value * offset;
}

template<>
constexpr u64 indexOffset<1>(u64 value) noexcept {
    return value;
}


template<bool includeFives, bool updateSums = false>
inline void updateTables10(u64 offset, u64 sum, u64 product, bool legal, u64* sums, u64* products, bool* predicates) noexcept {
    if (updateSums) {
        u64* sumPtr = sums + offset;
        *sumPtr = sum;
        ++sumPtr;
        *sumPtr = sum + 1;
        ++sumPtr;
        *sumPtr = sum + 2;
        ++sumPtr;
        *sumPtr = sum + 3;
        ++sumPtr;
        *sumPtr = sum + 4;
        ++sumPtr;
        *sumPtr = sum + 5;
        ++sumPtr;
        *sumPtr = sum + 6;
        ++sumPtr;
        *sumPtr = sum + 7;
        ++sumPtr;
        *sumPtr = sum + 8;
        ++sumPtr;
        *sumPtr = sum + 9;
    }
    bool* predPtr = predicates + offset;
    u64* prodPtr = products + offset;
    // 0
    *predPtr = false;
    *prodPtr = 0;
    // 1
    ++prodPtr;
    ++predPtr;
    *predPtr = false;
    *prodPtr = product;
    // 2
    ++prodPtr;
    ++predPtr;
    *predPtr = legal;
    *prodPtr = product << 1;
    // 3
    ++prodPtr;
    ++predPtr;
    *predPtr = legal;
    *prodPtr = product * 3;
    // 4
    ++prodPtr;
    ++predPtr;
    *predPtr = legal;
    *prodPtr = product << 2;
    // 5
    ++predPtr;
    ++prodPtr;
    *predPtr = includeFives && legal;
    *prodPtr = product * 5;
    // 6
    ++predPtr;
    ++prodPtr;
    *predPtr = legal;
    *prodPtr = product * 6;
    // 7
    ++predPtr;
    ++prodPtr;
    *predPtr = legal;
    *prodPtr = product * 7;
    // 8
    ++predPtr;
    ++prodPtr;
    *predPtr = legal;
    *prodPtr = product << 3;
    // 9
    ++predPtr;
    ++prodPtr;
    *predPtr = legal;
    *prodPtr = product * 9;
}

template<bool includeFives = false>
inline void initialize() noexcept {
    if(sums == nullptr) {
        sums = new u64[Len8];
    }
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
	//
	// Using numeric analysis of the results, I noticed that all of the values
	// are even! The only exceptions are 3,5,7,9,735
	//

	// Len8
	for (int i = 0; i < 10; ++i) {
		auto iPred = isLegalDigit<includeFives>(i);
		auto iSum = i;
		auto iMul = i;
		auto iInd = indexOffset<Len7>(i);
		for (int k = 0; k < 10; ++k) {
			auto kPred = iPred && isLegalDigit<includeFives>(k);
			auto kSum = k + iSum;
			auto kMul = k * iMul;
			auto kInd = indexOffset<Len6>(k) + iInd;
			for (int h = 0; h < 10; ++h) {
				auto hPred = kPred && isLegalDigit<includeFives>(h);
				auto hSum = h + kSum;
				auto hMul = h * kMul;
				auto hInd = indexOffset<Len5>(h) + kInd;
				for (int w = 0; w < 10; ++w) {
					auto wPred = hPred && isLegalDigit<includeFives>(w);
					auto wSum = w + hSum;
					auto wMul = w * hMul;
					auto wInd = indexOffset<Len4>(w) + hInd;
					for (int y = 0; y < 10; ++y) {
						auto yPred = wPred && isLegalDigit<includeFives>(y);
						auto ySum = y + wSum;
						auto yMul = y * wMul;
						auto yInd = indexOffset<Len3>(y) + wInd;
						for (int z = 0; z < 10; ++z) {
							auto zPred = yPred && isLegalDigit<includeFives>(z);
							auto zSum = z + ySum;
							auto zMul = z * yMul;
							auto zInd = indexOffset<Len2>(z) + yInd;
							for (int x = 0; x < 10; ++x) {
								auto outerMul = x == 0 ? 0 : x * zMul;
								auto combinedInd = indexOffset<Len1>(x) + zInd;
								auto outerSum = x + zSum;
								auto outerPredicate = zPred && isLegalDigit<includeFives>(x);
								updateTables10<includeFives, true>(combinedInd, outerSum, outerMul, outerPredicate, sums, productsLen8, predicatesLen8);
							}
						}
					}
				}
			}
		}
	}
	auto innerMostBodyNoSumUpdate  = [](auto oMul, auto oSum, auto oInd, auto oPred, auto prods, auto preds) noexcept {
		for (int x = 0; x < 10; ++x) {
			updateTables10<includeFives>(indexOffset<Len1>(x) + oInd, oSum, x * oMul, oPred && isLegalDigit<includeFives>(x), sums, prods, preds);
		}
	};
	// Len7
	for (int k = 0; k < 10; ++k) {
		auto kPred = isLegalDigit<includeFives>(k);
		auto kMul = k;
		auto kInd = indexOffset<Len6>(k);
		for (int h = 0; h < 10; ++h) {
			auto hPred = kPred && isLegalDigit<includeFives>(h);
			auto hMul = h * kMul;
			auto hInd = indexOffset<Len5>(h) + kInd;
			for (int w = 0; w < 10; ++w) {
				auto wPred = hPred && isLegalDigit<includeFives>(w);
				auto wMul = w * hMul;
				auto wInd = indexOffset<Len4>(w) + hInd;
				for (int y = 0; y < 10; ++y) {
                    auto yPred = wPred && isLegalDigit<includeFives>(y);
					auto yMul = y * wMul;
					auto yInd = indexOffset<Len3>(y) + wInd;
					for (int z = 0; z < 10; ++z) {
						auto zPred = yPred && isLegalDigit<includeFives>(z);
						auto zMul = z * yMul;
						auto zInd = indexOffset<Len2>(z) + yInd;
						innerMostBodyNoSumUpdate(zMul, 0u, zInd, zPred, productsLen7, predicatesLen7);
					}
				}
			}
		}
	}
	// Len6
	for (int h = 0; h < 10; ++h) {
        auto hPred = isLegalDigit<includeFives>(h);
		auto hSum = h;
		auto hMul = h;
		auto hInd = indexOffset<Len5>(h);
		for (int w = 0; w < 10; ++w) {
			auto wPred = hPred && isLegalDigit<includeFives>(w);
			auto wMul = w * hMul;
			auto wInd = indexOffset<Len4>(w) + hInd;
			for (int y = 0; y < 10; ++y) {
                auto yPred = wPred && isLegalDigit<includeFives>(y);
				auto yMul = y * wMul;
				auto yInd = indexOffset<Len3>(y) + wInd;
				for (int z = 0; z < 10; ++z) {
                    auto zPred = yPred && isLegalDigit<includeFives>(z);
					auto zMul = z * yMul;
					auto zInd = indexOffset<Len2>(z) + yInd;
					innerMostBodyNoSumUpdate(zMul, 0u,  zInd, zPred, productsLen6, predicatesLen6);
				}
			}
		}
	}
	// Len5
	for (int w = 0; w < 10; ++w) {
		auto wPred = isLegalDigit<includeFives>(w);
		auto wMul = w;
		auto wInd = indexOffset<Len4>(w);
		for (int y = 0; y < 10; ++y) {
			auto yPred = wPred && isLegalDigit<includeFives>(y);
			auto yMul = y * wMul;
			auto yInd = indexOffset<Len3>(y) + wInd;
			for (int z = 0; z < 10; ++z) {
				auto zPred = yPred && isLegalDigit<includeFives>(z);
				auto zMul = z * yMul;
				auto zInd = indexOffset<Len2>(z) + yInd;
				innerMostBodyNoSumUpdate(zMul, 0u, zInd, zPred, productsLen5, predicatesLen5);
			}
		}
	}
	// Len4
	for (int y = 0; y < 10; ++y) {
		auto yPred = isLegalDigit<includeFives>(y);
		auto yMul = y ;
		auto yInd = indexOffset<Len3>(y);
		for (int z = 0; z < 10; ++z) {
			auto zPred = yPred && isLegalDigit<includeFives>(z);
			auto zMul = z * yMul;
			auto zInd = indexOffset<Len2>(z) + yInd;
			innerMostBodyNoSumUpdate(zMul, 0u, zInd, zPred, productsLen4, predicatesLen4);
		}
	}
	// Len3
	for (int z = 0; z < 10; ++z) {
        auto zPred = isLegalDigit<includeFives>(z);
		auto zMul = z;
        auto zInd = indexOffset<Len2>(z);
		innerMostBodyNoSumUpdate(zMul, 0u, zInd, zPred, productsLen3, predicatesLen3);
	}
	// Len2
	for (int x = 0; x < 10; ++x) {
        updateTables10<includeFives>(indexOffset<Len1>(x), 0u, x, isLegalDigit<includeFives>(x), sums, productsLen2, predicatesLen2);
	}
}

template<u64 width>
inline bool legalValue(u64 x) noexcept {
	static_assert(width < 9, "Too large of a legal value check!");
	switch(width) {
		case 1: return x >= 2u && x != 5;
		case 2: return predicatesLen2[x];
		case 3: return predicatesLen3[x];
		case 4: return predicatesLen4[x];
		case 5: return predicatesLen5[x];
		case 6: return predicatesLen6[x];
		case 7: return predicatesLen7[x];
		case 8: return predicatesLen8[x];
		default: return false;
	}
}

template<u64 width>
inline u64 getProduct(u64 x) noexcept;

template<u64 partA, u64 partB>
inline u64 getInnerProduct(u64 x) noexcept {
	static_assert((partA + partB) < 20, "Can't express numbers 20 digits or higher!");
	static constexpr auto divisor = fastPow10<partA>;
	return getProduct<partA>(x % divisor) * getProduct<partB>(x / divisor);
}
template<u64 width>
inline u64 getProduct(u64 x) noexcept {
	static_assert(width < 20, "Too large of a product value!");
	switch(width) {
		case 0: return 1;
		case 1: return x;
		case 2: return productsLen2[x];
		case 3: return productsLen3[x];
		case 4: return productsLen4[x];
		case 5: return productsLen5[x];
		case 6: return productsLen6[x];
		case 7: return productsLen7[x];
		case 8: return productsLen8[x];
		case 9: return getInnerProduct<1, 8>(x);
		case 10: return getInnerProduct<2, 8>(x);
		case 11: return getInnerProduct<3, 8>(x);
		case 12: return getInnerProduct<4, 8>(x);
		case 13: return getInnerProduct<5, 8>(x);
		case 14: return getInnerProduct<6, 8>(x);
		case 15: return getInnerProduct<7, 8>(x);
		case 16: return getInnerProduct<8, 8>(x);
		case 17: return getInnerProduct<1, 16>(x);
		case 18: return getInnerProduct<2, 16>(x);
		case 19: return getInnerProduct<3, 16>(x);
		default: return x;
	}
}

template<u64 width>
inline u64 getSum(u64) noexcept;

template<u64 partA, u64 partB>
inline u64 getInnerSum(u64 x) noexcept {
	static_assert((partA + partB) < 20, "Can't express numbers 20 digits or higher!");
	static constexpr auto divisor = fastPow10<partA>;
	return getSum<partA>(x % divisor) + getSum<partB>(x / divisor);
}

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
		case 7:
		case 8: return sums[x];
		case 9: return getInnerSum<1, 8>(x);
		case 10: return getInnerSum<2, 8>(x);
		case 11: return getInnerSum<3, 8>(x);
		case 12: return getInnerSum<4, 8>(x);
		case 13: return getInnerSum<5, 8>(x);
		case 14: return getInnerSum<6, 8>(x);
		case 15: return getInnerSum<7, 8>(x);
		case 16: return getInnerSum<8, 8>(x);
		case 17: return getInnerSum<1, 16>(x);
		case 18: return getInnerSum<2, 16>(x);
		case 19: return getInnerSum<3, 16>(x);
		default: return x;
	}
}

template<u64 length>
constexpr auto startIndex = static_cast<u64>(shaveFactor * fastPow10<length - 1>);

template<> constexpr u64 startIndex<19u> = 2222222222222222222u;
template<> constexpr u64 startIndex<18u> = 222222222222222222u;
template<> constexpr u64 startIndex<17u> = 22222222222222222u;
template<> constexpr u64 startIndex<16u> = 2222222222222222u;
template<> constexpr u64 startIndex<15u> = 222222222222222u;

constexpr bool isEven(u64 value) noexcept {
	return (value == ((value >> 1) << 1));
}

template<u64 section, u64 digitCount, u64 k>
inline void singleDigitInnerLoop(u64 product, u64 sum, u64 value, vec64& results) noexcept {
	static_assert(k != 0, "Can't have a legal digit which is 0. Ever!");
	static_assert(k != 1, "Can't have a legal digit which is 1. Ever!");
	if (isEven(k) && legalValue<digitCount>(k)) {
		auto ns = sum + getSum<digitCount>(k);
		auto nv = indexOffset<section>(k) + value;
		if (componentQuodigious(nv, ns)) {
			// only compute the product if the sum is divisible
			auto np = product * getProduct<digitCount>(k);
			if (componentQuodigious(nv, np)) {
				results.emplace_back(nv);
			}
		}
	}
}
template<>
inline void singleDigitInnerLoop<1u, 1u, 2u>(u64 product, u64 sum, u64 value, vec64& results) noexcept {
	auto ns = sum + 2u;
	auto nv = indexOffset<1u>(2u) + value;
	if (componentQuodigious(nv, ns)) {
		auto np = product << 1; // shift left by 1 is the same as multiplying by two
		if (componentQuodigious(nv, np)) {
			results.emplace_back(nv);
		}
	}
}

template<>
inline void singleDigitInnerLoop<1u, 1u, 6u>(u64 product, u64 sum, u64 value, vec64& results) noexcept {
    auto ns = sum + 6u;
    auto nv = indexOffset<1u>(6u) + value;
    if (componentQuodigious(nv, ns)) {
        auto np = (product << 2) + (product << 1);
        if (componentQuodigious(nv, np)) {
            results.emplace_back(nv);
        }
    }
}

template<>
inline void singleDigitInnerLoop<1u, 1u, 4u>(u64 product, u64 sum, u64 value, vec64& results) noexcept {
	auto ns = sum + 4u;
	auto nv = indexOffset<1u>(4u) + value;
	if (componentQuodigious(nv, ns)) {
		auto np = product << 2; // shift left by 1 is the same as multiplying by two
		if (componentQuodigious(nv, np)) {
			results.emplace_back(nv);
		}
	}
}

template<>
inline void singleDigitInnerLoop<1u, 1u, 8u>(u64 product, u64 sum, u64 value, vec64& results) noexcept {
	auto ns = sum + 8u;
	auto nv = indexOffset<1u>(8u) + value;
	if (componentQuodigious(nv, ns)) {
		auto np = product << 3; // shift left by 1 is the same as multiplying by two
		if (componentQuodigious(nv, np)) {
			results.emplace_back(nv);
		}
	}
}

template<u64 section, u64 digitCount>
inline void threeDigitBody(u64 sum, u64 product, u64 index, vec64& results) noexcept {
	singleDigitInnerLoop<section, digitCount, 222>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 224>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 226>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 228>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 232>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 234>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 236>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 238>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 242>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 244>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 246>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 248>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 252>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 254>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 256>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 258>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 262>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 264>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 266>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 268>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 272>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 274>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 276>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 278>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 282>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 284>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 286>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 288>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 292>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 294>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 296>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 298>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 322>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 324>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 326>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 328>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 332>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 334>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 336>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 338>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 342>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 344>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 346>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 348>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 352>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 354>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 356>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 358>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 362>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 364>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 366>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 368>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 372>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 374>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 376>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 378>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 382>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 384>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 386>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 388>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 392>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 394>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 396>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 398>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 422>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 424>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 426>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 428>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 432>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 434>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 436>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 438>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 442>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 444>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 446>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 448>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 452>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 454>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 456>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 458>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 462>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 464>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 466>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 468>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 472>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 474>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 476>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 478>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 482>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 484>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 486>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 488>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 492>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 494>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 496>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 498>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 522>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 524>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 526>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 528>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 532>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 534>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 536>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 538>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 542>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 544>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 546>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 548>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 552>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 554>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 556>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 558>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 562>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 564>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 566>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 568>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 572>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 574>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 576>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 578>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 582>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 584>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 586>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 588>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 592>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 594>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 596>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 598>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 622>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 624>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 626>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 628>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 632>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 634>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 636>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 638>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 642>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 644>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 646>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 648>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 652>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 654>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 656>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 658>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 662>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 664>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 666>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 668>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 672>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 674>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 676>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 678>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 682>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 684>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 686>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 688>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 692>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 694>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 696>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 698>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 722>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 724>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 726>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 728>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 732>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 734>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 736>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 738>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 742>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 744>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 746>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 748>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 752>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 754>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 756>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 758>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 762>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 764>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 766>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 768>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 772>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 774>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 776>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 778>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 782>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 784>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 786>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 788>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 792>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 794>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 796>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 798>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 822>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 824>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 826>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 828>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 832>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 834>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 836>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 838>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 842>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 844>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 846>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 848>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 852>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 854>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 856>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 858>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 862>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 864>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 866>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 868>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 872>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 874>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 876>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 878>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 882>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 884>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 886>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 888>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 892>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 894>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 896>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 898>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 922>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 924>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 926>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 928>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 932>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 934>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 936>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 938>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 942>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 944>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 946>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 948>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 952>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 954>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 956>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 958>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 962>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 964>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 966>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 968>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 972>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 974>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 976>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 978>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 982>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 984>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 986>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 988>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 992>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 994>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 996>(product, sum, index, results);
	singleDigitInnerLoop<section, digitCount, 998>(product, sum, index, results);
}

template<u64 start, u64 end, u64 digitCount, u64 section>
inline void innermostLoopBody(u64 sum, u64 product, u64 index, vec64& results) noexcept {
	if (digitCount == 1) {
		singleDigitInnerLoop<section, digitCount, 2>(product, sum, index, results);
		//singleDigitInnerLoop<section, digitCount, 3>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 4>(product, sum, index, results);
		//singleDigitInnerLoop<section, digitCount, 5>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 6>(product, sum, index, results);
		//singleDigitInnerLoop<section, digitCount, 7>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 8>(product, sum, index, results);
		//singleDigitInnerLoop<section, digitCount, 9>(product, sum, index, results);
	} else if (digitCount == 2) {
		singleDigitInnerLoop<section, digitCount, 22>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 24>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 26>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 28>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 32>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 34>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 36>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 38>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 42>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 44>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 46>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 48>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 52>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 54>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 56>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 58>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 62>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 64>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 66>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 68>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 72>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 74>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 76>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 78>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 82>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 84>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 86>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 88>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 92>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 94>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 96>(product, sum, index, results);
		singleDigitInnerLoop<section, digitCount, 98>(product, sum, index, results);
	} else if (digitCount == 3) {
		threeDigitBody<section, digitCount>(sum, product, index, results);
	} else {
		for (auto k = start; k < end; ++k) {
			if (isEven(k) && legalValue<digitCount>(k)) {
				auto l1Sum = sum + getSum<digitCount>(k);
				auto l1Value = indexOffset<section>(k) + index;
				if (componentQuodigious(l1Value, l1Sum)) {
					auto l1Product = product * getProduct<digitCount>(k);
					if (componentQuodigious(l1Value, l1Product)) {
						results.emplace_back(l1Value);
					}
				}
			}
		}
	}
}


template<u64 length, u64 start, u64 end>
inline int performQuodigiousCheck(vec64& results) noexcept {
	// precompute the fuck out of all of this!
	// Compilers hate me, I am the TEMPLATE MASTER
	static constexpr auto l3Digits = level3Digits<length>;
	static constexpr auto l2Digits = level2Digits<length>;
	static constexpr auto l1Digits = level1Digits<length>;
	static_assert((l3Digits + l2Digits + l1Digits) == length, "Illegal digit layout!");
	static constexpr auto l1Shift = 0u;
	static constexpr auto l2Shift = l1Digits;
	static constexpr auto l3Shift = l2Digits + l1Digits;
	static constexpr auto l3Factor = fastPow10<l3Digits>;
	static constexpr auto l2Factor = fastPow10<l2Digits>;
	static constexpr auto l1Factor = fastPow10<l1Digits>;
	static constexpr auto l3Section = fastPow10<l3Shift>;
	static constexpr auto l2Section = fastPow10<l2Shift>;
	static constexpr auto l1Section = fastPow10<l1Shift>;
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
					innermostLoopBody<startL1, endL1, l1Digits, l1Section>(l2Sum, l2Product, l2Index, results);
				}
			}
		}
	}
	return 0;
}

#ifdef DEBUG
template<u64 width>
inline void printDigitalLayout() noexcept {
	std::cout << width << ":" << level3Digits<width> << ":" << level2Digits<width> << ":" << level1Digits<width> << std::endl;
}
#endif


void printout(vec64& l) noexcept {
	for (auto v : l) {
		std::cout << v << std::endl;
	}
	l.clear();
}

template<u64 length, u64 base, u64 st, u64 startFactor, u64 endFactor>
inline int doQuodigious(vec64& input) noexcept {
	return performQuodigiousCheck<length, (st + (base * startFactor)), base * endFactor>(input);
}

template<u64 length>
inline void body() noexcept {
	static vec64 l0, l1, l2, l3, l4, l5, l6, l7;
	static constexpr auto skip5 = length > 4;
	// this is not going to change ever!
	static constexpr auto base = fastPow10<length - 1>;
	static constexpr auto st = startIndex<length>;
#ifdef DEBUG
	printDigitalLayout<length>();
#endif
	auto fut0 = std::async(std::launch::async, []() { return doQuodigious<length, base, st, 0, 3>(l0); });
	auto fut1 = std::async(std::launch::async, []() { return doQuodigious<length, base, st, 1, 4>(l1); });
	auto fut2 = std::async(std::launch::async, []() { return doQuodigious<length, base, st, 2, 5>(l2); });
	auto fut3 = std::async(std::launch::async, []() { return skip5 ? 0 : doQuodigious<length, base, st, 3, 6>(l3); });
	auto fut4 = std::async(std::launch::async, []() { return doQuodigious<length, base, st, 4, 7 >(l4); });
	auto fut5 = std::async(std::launch::async, []() { return doQuodigious<length, base, st, 5, 8 >(l5); });
	auto fut6 = std::async(std::launch::async, []() { return doQuodigious<length, base, st, 6, 9 >(l6); });
	auto fut7 = std::async(std::launch::async, []() { return doQuodigious<length, base, st, 7, 10>(l7); });
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

template<u64 length>
constexpr auto endIndex = fastPow10<length>;

template<u64 length>
inline void singleThreadedSimpleBody() noexcept {
    for (auto value = startIndex<length>; value < endIndex<length>; ++value) {
		if (legalValue<length>(value) && isQuodigious(value, getSum<length>(value), getProduct<length>(value))) {
			std::cout << value << std::endl;
		}
	}
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
    if (sums != nullptr) {
        delete[] sums;
        sums = nullptr;
    }
	return 0;
}
