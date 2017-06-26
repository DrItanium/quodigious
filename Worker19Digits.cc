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
#include <cstdint>
#include <sstream>
#include <functional>
#include <future>
#include <map>
#include "qlib.h"

inline constexpr bool checkValue(u64 sum) noexcept {
	return (sum % 2 == 0) || (sum % 3 == 0);
}
inline constexpr bool innerMostBody(u64 sum, u64 product, u64 value) noexcept {
    return checkValue(sum) && isQuodigious(value, sum, product);
}
template<u64 width>
constexpr int numberOfDigitsForGivenWidth() noexcept {
    static_assert(width >= 0, "Negative width doesn't make sense");
    return 7 * numberOfDigitsForGivenWidth<width - 1>();
}
template<> constexpr int numberOfDigitsForGivenWidth<0>() noexcept { return 1; }
template<u64 width>
constexpr auto numElements = numberOfDigitsForGivenWidth<width>();

template<u64 width>
inline u64* getSums() noexcept {
    static_assert(width >= 2 && width < 9, "Illegal width!");
    static u64 sums[numElements<width>] = { 0 };
    return sums;
}
template<u64 width>
inline u64* getProducts() noexcept {
    static_assert(width >= 2 && width < 9, "Illegal width!");
    static u64 products[numElements<width>] = { 0 };
    return products;
}
template<u64 width>
inline u64* getNumbers() noexcept {
    static_assert(width >= 2 && width < 9, "Illegal width!");
    static u64 numbers[numElements<width>] = { 0 };
    return numbers;
}

template<u64 width>
constexpr u64 makeDigitAt(u64 input) noexcept {
    static_assert(width >= 0, "Can't have negative width!");
    return input * fastPow10<width>;
}

template<u64 width>
void populateWidth() noexcept {
    static_assert(width >= 2 && width < 9, "Illegal width!");
    static bool populated = false;
    if (!populated) {
        populated = true;
        populateWidth<width - 1>();
        auto* numPtr = getNumbers<width>();
        auto* sumPtr = getSums<width>();
        auto* prodPtr = getProducts<width>();
        auto* prevNum = getNumbers<width - 1>();
        auto* prevSum = getSums<width - 1>();
        auto* prevProd = getProducts<width - 1>();
        for (int i = 0; i < numElements<width - 1>; ++i) {
            auto s = prevSum[i];
            auto p = prevProd[i];
            auto n = makeDigitAt<1>(prevNum[i]);
            for (int j = 2; j < 10; ++j) {
                if (j != 5) {
                    *numPtr = n + j;
                    *sumPtr = s + j;
                    *prodPtr = p * j;
                    ++numPtr;
                    ++sumPtr;
                    ++prodPtr;

                }
            }
        }
    }
}
//TODO: reduce memory footprint by specializing on 6
template<>
void populateWidth<2>() noexcept {
    static bool populated = false;
    if (!populated) {
        populated = true;
        auto* sumPtr = getSums<2>();
        auto* prodPtr = getProducts<2>();
        auto* numPtr = getNumbers<2>();
        for (int i = 2; i < 10; ++i) {
            if (i != 5) {
                auto numberOuter = makeDigitAt<1>(i);
                for (int j = 2; j < 10; ++j) {
                    if (j != 5) {
                        *sumPtr = i + j;
                        *prodPtr = i * j;
                        *numPtr = numberOuter + j;
                        ++sumPtr;
                        ++prodPtr;
                        ++numPtr;
                    }
                }
            }
        }
    }
}

u64 values2To4[numElements<2>] = { 0 };
u64 values12To19[numElements<8>] = { 0 };

template<u64 width, u64 factor>
inline void populateArray(u64* storage) noexcept {
	auto nums = getNumbers<width>();
    for (int i = 0; i < numElements<width>; ++i, ++storage, ++nums) {
        *storage = *nums * fastPow10<factor>;
    }
}

void setup() noexcept {
    populateWidth<2>();
    populateWidth<8>();
    populateArray<2, 1>(values2To4);
    populateArray<8, 11>(values12To19);
}

template<u64 width> inline u64* getTransitionValues() noexcept { return nullptr; }

template<> inline u64* getTransitionValues<2>() noexcept { return values2To4; }
template<> inline u64* getTransitionValues<8>() noexcept { return values12To19; }


std::string fourthBody(u64 s, u64 p, u64 n) noexcept {
    std::ostringstream str;
    constexpr auto outerElementWidth = 8;
    constexpr auto innerElementWidth = 2;
    static_assert((outerElementWidth + innerElementWidth) == 10, "The outer element width and inner element widths are not correct!");
    static_assert(outerElementWidth == 8 || outerElementWidth == 2, "The outer element width is not 8 or 2!");
    static_assert(innerElementWidth == 8 || innerElementWidth == 2, "The inner element width is not 8 or 2!");
    auto sO = getSums<outerElementWidth>();
    auto pO = getProducts<outerElementWidth>();
    auto nO = getTransitionValues<outerElementWidth>();
    auto sI = getSums<innerElementWidth>();
    auto pI = getProducts<innerElementWidth>();
    auto nI = getTransitionValues<innerElementWidth>();
    for (int i = 0; i < numElements<outerElementWidth>; ++i, ++sO, ++pO, ++nO) {
		auto* sumInner = sI;
		auto* productInner = pI;
		auto* numberInner = nI;
		auto sum = *sO + s;
		auto product = *pO * p;
		auto index = *nO + n;
		for (int j = 0; j < numElements<innerElementWidth>; ++j, ++sumInner, ++productInner, ++numberInner) {
            auto result = index + *numberInner;
			if (innerMostBody(sum + *sumInner, product * *productInner, result)) {
				str << result << std::endl;
			}
		}
    }
    return str.str();
}

int main() {
	std::stringstream collection;
	setup();
    auto sums = getSums<8>();
    auto products = getProducts<8>();
    auto nums = getNumbers<8>();
    auto mkBody = [sums, products, nums](auto index, auto offset) {
        return std::async(std::launch::async, fourthBody, sums[index] + offset, products[index] * offset, (nums[index] * fastPow10<3>) + offset);
    };
    while(std::cin.good()) {
        int innerThreadId = 0;
        std::cin >> innerThreadId;
        if (innerThreadId < 0 || innerThreadId >= numElements<8>) {
            std::cerr << "Illegal inner thread id, must be in the range [0," << numElements<8> - 1 << "]" << std::endl;
            return 1;
        }
        auto p0 = mkBody(innerThreadId, 2);
        auto p1 = mkBody(innerThreadId, 4);
        auto p2 = mkBody(innerThreadId, 6);
        auto p3 = mkBody(innerThreadId, 8);
        collection << p0.get() << p1.get() << p2.get() << p3.get();
    }
	std::cout << collection.str() << std::endl;
    return 0;
}
