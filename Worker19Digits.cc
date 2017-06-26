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
inline constexpr u64 innerMostBody(u64 sum, u64 product, u64 value) noexcept {
	if (checkValue(sum) && isQuodigious(value, sum, product)) {
		return value;
	}
	return 0;
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
u64 values4To12[numElements<8>] = { 0 };
u64 values12To19[numElements<8>] = { 0 };

template<u64 width, u64 factor>
inline void populateArray(u64* storage) noexcept {
	auto nums = getNumbers<width>();
    for (int i = 0; i < numElements<width>; ++i) {
        *storage = nums[i] * fastPow10<factor>;
        ++storage;
    }
}

void setup() noexcept {
    populateWidth<2>();
    populateWidth<8>();
    populateArray<2, 1>(values2To4);
    populateArray<8, 3>(values4To12);
    populateArray<8, 11>(values12To19);
}


std::string fourthBody(u64 s, u64 p, u64 n) noexcept {
    std::ostringstream str;
    auto s2 = getSums<2>();
    auto p2 = getProducts<2>();
    auto n2 = values2To4;
    for (int i = 0; i < numElements<2>; ++i, ++s2, ++p2, ++n2) {
        //loopBody<12, 19>(str, *s2 + s, *p2 * p, *n2 + n);
		auto* pSum = getSums<8>();
		auto* pProd = getProducts<8>();
		auto* transition = values12To19;
		auto sum = *s2 + s;
		auto product = *p2 * p;
		auto index = *n2 + n;
		for (int i = 0; i < numElements<8>; ++i, ++pSum, ++pProd, ++transition) {
			auto result = innerMostBody(sum + *pSum, product * *pProd, index + *transition);
			if (result != 0) {
				str << result << std::endl;
			}
		}
    }
    return str.str();
}
inline bool body(std::ostream& storage, std::istream& input) noexcept {
    int innerThreadId = 0;
	input >> innerThreadId;
    if (innerThreadId < 0 || innerThreadId >= numElements<8>) {
        std::cerr << "Illegal inner thread id, must be in the range [0," << numElements<8> - 1 << "]" << std::endl;
		return false;
    }
    auto sum = getSums<8>()[innerThreadId];
    auto prod = getProducts<8>()[innerThreadId];
    auto num = values4To12[innerThreadId];
    auto p0 = std::async(std::launch::async, fourthBody, 2 + sum, 2 * prod, 2 + num);
    auto p1 = std::async(std::launch::async, fourthBody, 4 + sum, 4 * prod, 4 + num);
    auto p2 = std::async(std::launch::async, fourthBody, 6 + sum, 6 * prod, 6 + num);
    auto p3 = std::async(std::launch::async, fourthBody, 8 + sum, 8 * prod, 8 + num);
    storage << p0.get() << p1.get() << p2.get() << p3.get();
	return true;
}

int main() {
	int errorCode = 0;
	std::stringstream collection;
	setup();
    while(std::cin.good()) {
		if (!body(collection, std::cin)) {
			errorCode = 1;
			break;
		}
    }
	std::cout << collection.str() << std::endl;
    return errorCode;
}
