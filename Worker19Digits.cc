//  Copyright (c) 2017 Joshua Scoggins
//
//  This software is provided 'as-is', without any express or implied
//  warranty. In no event will the authors be held liable for any damages
//  arising from the use of this software.  //
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

struct Triple {
	Triple(u64 _s, u64 _p, u64 _n) : sum(_s), product(_p), number(_n) { }
	Triple() : Triple(0, 0, 0) { }
	u64 sum;
	u64 product;
	u64 number;
	bool isQuodigious() const noexcept {

	}
};
//using Triple = std::tuple<u64, u64, u64>;

inline u64 getSum(const Triple& in) noexcept {
	return in.sum;
    //return std::get<0>(in);
}
inline u64 getProduct(const Triple& in) noexcept {
	return in.product;
    //return std::get<1>(in);
}
inline u64 getNumber(const Triple& in) noexcept {
	return in.number;
    //return std::get<2>(in);
}

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
constexpr u64 makeDigitAt(u64 input) noexcept {
    static_assert(width >= 0, "Can't have negative width!");
    return input * fastPow10<width>;
}

template<u64 width>
inline Triple* getTriples() noexcept {
    static_assert(width >= 2 && width < 9, "Illegal width!");
    static Triple elements[numElements<width>];
    return elements;
}


template<u64 width>
void populateWidth() noexcept {
    static_assert(width >= 2 && width < 9, "Illegal width!");
    static bool populated = false;
    if (!populated) {
        populated = true;
        populateWidth<width - 1>();
        auto* triple = getTriples<width>();
        auto* prev = getTriples<width - 1>();
        for (int i = 0; i < numElements<width - 1>; ++i) {
            auto tmp = prev[i];
            auto s = getSum(tmp);
            auto p = getProduct(tmp);
            auto n = makeDigitAt<1>(getNumber(tmp));
            for (int j = 2; j < 10; ++j) {
                if (j != 5) {
                    *triple = Triple(s + j, p * j, n + j);
                    ++triple;

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
        auto* triple = getTriples<2>();
        for (int i = 2; i < 10; ++i) {
            if (i != 5) {
                auto numberOuter = makeDigitAt<1>(i);
                for (int j = 2; j < 10; ++j) {
                    if (j != 5) {
                        *triple = Triple(i + j, i * j, numberOuter + j);
                        ++triple;
                    }
                }
            }
        }
    }
}


template<u64 width>
using Range = std::array<Triple, numElements<width>>;

Triple range12To19[numElements<8>];
Range<2> range2To4;

std::string fourthBody(u64 s, u64 p, u64 n) noexcept {
    std::ostringstream str;
    auto out = range12To19;
    for (int i =0; i < numElements<8>; ++i, ++out) {
		auto tmp = *out;
        auto sum = s + getSum(tmp);
        auto prod = p * getProduct(tmp);
        auto index = n + getNumber(tmp);
        for (const auto& tmp2 : range2To4) {
            auto result = index + getNumber(tmp2);
            if (innerMostBody(sum + getSum(tmp2), prod * getProduct(tmp2), result)) {
                str << result << std::endl;
            }
        }
    }
    return str.str();
}

int main() {
	std::stringstream collection;
    // setup the triples
    populateWidth<2>();
    populateWidth<8>();
    auto t8 = getTriples<8>();
    for (int i = 0; i < numElements<8>; ++i) {
        range12To19[i] = Triple(getSum(t8[i]), getProduct(t8[i]), getNumber(t8[i]) * fastPow10<11>);
    }
    auto t2 = getTriples<2>();
    for (int i = 0; i < numElements<2>; ++i) {
        range2To4[i] = Triple(getSum(t2[i]), getProduct(t2[i]), getNumber(t2[i]) * fastPow10<1>);
    }
    auto triples = getTriples<8>();
    auto mkBody = [triples](auto index, auto offset) {
        auto t = triples[index];
        return std::async(std::launch::async, fourthBody, getSum(t) + offset, getProduct(t) * offset, (getNumber(t) * fastPow10<3>) + offset);
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
