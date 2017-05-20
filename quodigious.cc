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


template<bool includeFives>
inline void updateTables10NoSum(u64 offset, u64 product, bool legal, u64* products, bool* predicates) noexcept {
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
    *prodPtr = (product << 1) + product;
    // 4
    ++prodPtr;
    ++predPtr;
    *predPtr = legal;
    *prodPtr = product << 2;
    // 5
    ++predPtr;
    ++prodPtr;
    *predPtr = includeFives && legal;
    *prodPtr = (product << 2) + product;
    // 6
    ++predPtr;
    ++prodPtr;
    *predPtr = legal;
    *prodPtr = (product << 2) + (product << 1);
    // 7
    ++predPtr;
    ++prodPtr;
    *predPtr = legal;
    *prodPtr = (product << 2) + (product << 1) + product;
    // 8
    ++predPtr;
    ++prodPtr;
    *predPtr = legal;
    *prodPtr = product << 3;
    // 9
    ++predPtr;
    ++prodPtr;
    *predPtr = legal;
    *prodPtr = (product << 3) + product;
}
template<bool includeFives>
inline void updateTables10(u64 offset, u64 sum, u64 product, bool legal, u64* sums, u64* products, bool* predicates) noexcept {
    u64* sumPtr = sums + offset;
    //we only have to do this 10 times so unroll it and do it by hand :D
    *sumPtr = sum;
    ++sumPtr;
    ++sum;
    *sumPtr = sum;
    ++sumPtr;
    ++sum;
    *sumPtr = sum;
    ++sumPtr;
    ++sum;
    *sumPtr = sum;
    ++sumPtr;
    ++sum;
    *sumPtr = sum;
    ++sumPtr;
    ++sum;
    *sumPtr = sum;
    ++sumPtr;
    ++sum;
    *sumPtr = sum;
    ++sumPtr;
    ++sum;
    *sumPtr = sum;
    ++sumPtr;
    ++sum;
    *sumPtr = sum;
    ++sumPtr;
    ++sum;
    *sumPtr = sum;
    updateTables10NoSum<includeFives>(offset, product, legal, products, predicates);
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
    auto len8 = std::async(std::launch::async, []() {
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
                                updateTables10<includeFives>(zInd, zSum, 0, false, sums, productsLen8, predicatesLen8);
                                updateTables10<includeFives>(zInd + indexOffset<Len1>(1), zSum + 1, zMul, false, sums, productsLen8, predicatesLen8);
	    						for (int x = 2; x < 10; ++x) {
	    							auto outerMul = x * zMul;
	    							auto combinedInd = indexOffset<Len1>(x) + zInd;
	    							auto outerSum = x + zSum;
	    							auto outerPredicate = zPred && isLegalDigit<includeFives>(x);
	    							updateTables10<includeFives>(combinedInd, outerSum, outerMul, outerPredicate, sums, productsLen8, predicatesLen8);
	    						}
	    					}
	    				}
	    			}
	    		}
	    	}
	    }
        return 0;
    });
	auto innerMostBodyNoSumUpdate  = [](auto oMul, auto oInd, auto oPred, auto prods, auto preds) noexcept {
        updateTables10NoSum<includeFives>(oInd, 0, false, prods, preds);
        updateTables10NoSum<includeFives>(oInd + indexOffset<Len1>(1), oPred, false, prods, preds);
		for (int x = 2; x < 10; ++x) {
			updateTables10NoSum<includeFives>(indexOffset<Len1>(x) + oInd, x * oMul, oPred && isLegalDigit<includeFives>(x), prods, preds);
		}
	};
    auto len7 = std::async(std::launch::async, [innerMostBodyNoSumUpdate]() {
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
						innerMostBodyNoSumUpdate(zMul, zInd, zPred, productsLen7, predicatesLen7);
					}
				}
			}
		}
	}
    return 0;
    });
    auto len6 = std::async(std::launch::async, [innerMostBodyNoSumUpdate]() {
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
	    				innerMostBodyNoSumUpdate(zMul, zInd, zPred, productsLen6, predicatesLen6);
	    			}
	    		}
	    	}
	    }
        return 0;
    });
    auto len5 = std::async(std::launch::async, [innerMostBodyNoSumUpdate]() {
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
				innerMostBodyNoSumUpdate(zMul, zInd, zPred, productsLen5, predicatesLen5);
			}
		}
	}
    return 0;
    });
	// Len4
	for (int y = 0; y < 10; ++y) {
		auto yPred = isLegalDigit<includeFives>(y);
		auto yMul = y ;
		auto yInd = indexOffset<Len3>(y);
		for (int z = 0; z < 10; ++z) {
			auto zPred = yPred && isLegalDigit<includeFives>(z);
			auto zMul = z * yMul;
			auto zInd = indexOffset<Len2>(z) + yInd;
			innerMostBodyNoSumUpdate(zMul, zInd, zPred, productsLen4, predicatesLen4);
		}
	}
	// Len3
	for (int z = 0; z < 10; ++z) {
        auto zPred = isLegalDigit<includeFives>(z);
		auto zMul = z;
        auto zInd = indexOffset<Len2>(z);
		innerMostBodyNoSumUpdate(zMul, zInd, zPred, productsLen3, predicatesLen3);
	}
	// Len2
	for (int x = 0; x < 10; ++x) {
        updateTables10NoSum<includeFives>(indexOffset<Len1>(x), x, isLegalDigit<includeFives>(x), productsLen2, predicatesLen2);
	}
    len5.get();
    len6.get();
    len7.get();
    len8.get();
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

template<u64 width, u64 value>
constexpr u64 computeSum() noexcept {
    return (value % 10) + computeSum<width - 1, value / 10>();
}
template<> constexpr u64 computeSum<1, 0>() noexcept { return 0; }
template<> constexpr u64 computeSum<1, 1>() noexcept { return 1; }
template<> constexpr u64 computeSum<1, 2>() noexcept { return 2; }
template<> constexpr u64 computeSum<1, 3>() noexcept { return 3; }
template<> constexpr u64 computeSum<1, 4>() noexcept { return 4; }
template<> constexpr u64 computeSum<1, 5>() noexcept { return 5; }
template<> constexpr u64 computeSum<1, 6>() noexcept { return 6; }
template<> constexpr u64 computeSum<1, 7>() noexcept { return 7; }
template<> constexpr u64 computeSum<1, 8>() noexcept { return 8; }
template<> constexpr u64 computeSum<1, 9>() noexcept { return 9; }
template<> constexpr u64 computeSum<0, 0>() noexcept { return 0; }
template<> constexpr u64 computeSum<0, 1>() noexcept { return 0; }
template<> constexpr u64 computeSum<0, 2>() noexcept { return 0; }
template<> constexpr u64 computeSum<0, 3>() noexcept { return 0; }
template<> constexpr u64 computeSum<0, 4>() noexcept { return 0; }
template<> constexpr u64 computeSum<0, 5>() noexcept { return 0; }
template<> constexpr u64 computeSum<0, 6>() noexcept { return 0; }
template<> constexpr u64 computeSum<0, 7>() noexcept { return 0; }
template<> constexpr u64 computeSum<0, 8>() noexcept { return 0; }
template<> constexpr u64 computeSum<0, 9>() noexcept { return 0; }

template<u64 width, u64 value>
constexpr u64 computeProduct() noexcept {
    return (value % 10) * computeProduct<width - 1, value / 10>();
}

template<> constexpr u64 computeProduct<1, 0>() noexcept { return 0; }
template<> constexpr u64 computeProduct<1, 1>() noexcept { return 1; }
template<> constexpr u64 computeProduct<1, 2>() noexcept { return 2; }
template<> constexpr u64 computeProduct<1, 3>() noexcept { return 3; }
template<> constexpr u64 computeProduct<1, 4>() noexcept { return 4; }
template<> constexpr u64 computeProduct<1, 5>() noexcept { return 5; }
template<> constexpr u64 computeProduct<1, 6>() noexcept { return 6; }
template<> constexpr u64 computeProduct<1, 7>() noexcept { return 7; }
template<> constexpr u64 computeProduct<1, 8>() noexcept { return 8; }
template<> constexpr u64 computeProduct<1, 9>() noexcept { return 9; }
template<> constexpr u64 computeProduct<0, 0>() noexcept { return 0; }
template<> constexpr u64 computeProduct<0, 1>() noexcept { return 0; }
template<> constexpr u64 computeProduct<0, 2>() noexcept { return 0; }
template<> constexpr u64 computeProduct<0, 3>() noexcept { return 0; }
template<> constexpr u64 computeProduct<0, 4>() noexcept { return 0; }
template<> constexpr u64 computeProduct<0, 5>() noexcept { return 0; }
template<> constexpr u64 computeProduct<0, 6>() noexcept { return 0; }
template<> constexpr u64 computeProduct<0, 7>() noexcept { return 0; }
template<> constexpr u64 computeProduct<0, 8>() noexcept { return 0; }
template<> constexpr u64 computeProduct<0, 9>() noexcept { return 0; }

template<u64 section, u64 digitCount, u64 k>
inline void singleDigitInnerLoop(u64 product, u64 sum, u64 value, vec64& results) noexcept {
	static_assert(k != 0, "Can't have a legal digit which is 0. Ever!");
	static_assert(k != 1, "Can't have a legal digit which is 1. Ever!");
	if (isEven(k) && legalValue<digitCount>(k)) {
		auto ns = sum + computeSum<digitCount, k>();
		auto nv = indexOffset<section>(k) + value;
		if (componentQuodigious(nv, ns)) {
			// only compute the product if the sum is divisible
			auto np = product * computeProduct<digitCount, k>();
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

constexpr u64 computeBodyOffset(u64 offset, u64 add) noexcept {
    return (offset << 3) + (offset << 1) + add;
}

template<u64 section, u64 digitCount, u64 offset = 0>
inline void oneDigitBody(u64 sum, u64 product, u64 index, vec64& results) noexcept {
    singleDigitInnerLoop<section, digitCount, computeBodyOffset(offset, 2)>(product, sum, index, results);
    singleDigitInnerLoop<section, digitCount, computeBodyOffset(offset, 4)>(product, sum, index, results);
    singleDigitInnerLoop<section, digitCount, computeBodyOffset(offset, 6)>(product, sum, index, results);
    singleDigitInnerLoop<section, digitCount, computeBodyOffset(offset, 8)>(product, sum, index, results);
}
template<u64 section, u64 digitCount, u64 offset = 0>
inline void twoDigitBody(u64 sum, u64 product, u64 index, vec64& results) noexcept {
    oneDigitBody<section, digitCount, computeBodyOffset(offset, 2)>(sum, product, index, results);
    oneDigitBody<section, digitCount, computeBodyOffset(offset, 3)>(sum, product, index, results);
    oneDigitBody<section, digitCount, computeBodyOffset(offset, 4)>(sum, product, index, results);
    oneDigitBody<section, digitCount, computeBodyOffset(offset, 5)>(sum, product, index, results);
    oneDigitBody<section, digitCount, computeBodyOffset(offset, 6)>(sum, product, index, results);
    oneDigitBody<section, digitCount, computeBodyOffset(offset, 7)>(sum, product, index, results);
    oneDigitBody<section, digitCount, computeBodyOffset(offset, 8)>(sum, product, index, results);
    oneDigitBody<section, digitCount, computeBodyOffset(offset, 9)>(sum, product, index, results);
}
template<u64 section, u64 digitCount, u64 offset = 0>
inline void threeDigitBody(u64 sum, u64 product, u64 index, vec64& results) noexcept {
    twoDigitBody<section, digitCount, computeBodyOffset(offset, 2)>(sum, product, index, results);
    twoDigitBody<section, digitCount, computeBodyOffset(offset, 3)>(sum, product, index, results);
    twoDigitBody<section, digitCount, computeBodyOffset(offset, 4)>(sum, product, index, results);
    twoDigitBody<section, digitCount, computeBodyOffset(offset, 5)>(sum, product, index, results);
    twoDigitBody<section, digitCount, computeBodyOffset(offset, 6)>(sum, product, index, results);
    twoDigitBody<section, digitCount, computeBodyOffset(offset, 7)>(sum, product, index, results);
    twoDigitBody<section, digitCount, computeBodyOffset(offset, 8)>(sum, product, index, results);
    twoDigitBody<section, digitCount, computeBodyOffset(offset, 9)>(sum, product, index, results);
}

template<u64 start, u64 end, u64 digitCount, u64 section>
inline void innermostLoopBody(u64 sum, u64 product, u64 index, vec64& results) noexcept {
	if (digitCount == 1) {
        oneDigitBody<section, digitCount>(sum, product, index, results);
	} else if (digitCount == 2) {
        twoDigitBody<section, digitCount>(sum, product, index, results);
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
