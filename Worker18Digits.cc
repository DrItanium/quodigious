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

class Triple {
    private:
        static inline constexpr bool checkValue(u64 sum) noexcept {
            return (sum % 2 == 0) || (sum % 3 == 0);
        }
        static inline constexpr bool innerMostBody(u64 sum, u64 product, u64 value) noexcept {
            return checkValue(sum) && (value % product == 0) && (value % sum == 0);
        }
    public:
        Triple(u64 s, u64 p, u64 n) : _sum(s), _product(p), _number(n) { }
        Triple() : Triple(0, 0, 0) { }
        inline bool assume(u64 sum, u64 product, u64 number) noexcept {
            _sum = sum;
            _product = product;
            _number = number;
        }
        inline bool isQuodigious(u64 sCombine, u64 pCombine, u64 nCombine) const noexcept {
            return innerMostBody(sCombine + _sum, pCombine * _product, nCombine + _number);
        }
        inline u64 buildNumber(u64 offset) const noexcept {
            return _number + offset;
        }
        inline u64 getSum() const noexcept { return _sum; }
        inline u64 getProduct() const noexcept { return _product; }
        inline u64 getNumber() const noexcept { return _number; }
    private:
        u64 _sum;
        u64 _product;
        u64 _number;
};

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
inline void populateWidth() noexcept {
    static_assert(width >= 2 && width < 9, "Illegal width!");
    populateWidth<width - 1>();
    auto* triple = getTriples<width>();
    auto* prev = getTriples<width - 1>();
    for (int i = 0; i < numElements<width - 1>; ++i) {
        auto tmp = prev[i];
        auto s = tmp.getSum();
        auto p = tmp.getProduct();
        auto n = makeDigitAt<1>(tmp.getNumber());
        for (int j = 2; j < 10; ++j) {
            if (j != 5) {
                *triple = Triple(s + j, p * j, n + j);
                ++triple;

            }
        }
    }
}
//TODO: reduce memory footprint by specializing on 6
template<>
inline void populateWidth<2>() noexcept {
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


Triple range12To18[numElements<7>];
Triple range2To4[numElements<2>];

void fourthBody(std::ostream& str, u64 s, u64 p, u64 n) noexcept {
    for (auto const & i : range12To18) {
        auto sum = s + i.getSum();
        auto prod = p * i.getProduct();
        auto index = n + i.getNumber();
        for (auto const & tmp2 : range2To4) {
            if (tmp2.isQuodigious(sum, prod, index)) {
                str << tmp2.buildNumber(index) << std::endl;
            }
        }
    }
}

template<u64 count>
std::string doIt(int start, int stop) noexcept {
    std::array<Triple, count> tmp;
    auto t8 = getTriples<8>();
    for (int i = start, j = 0; i < stop; ++i, ++j) {
        tmp[j] = t8[i];
    }
    auto mkBody = [tmp, start, stop](auto offset) noexcept {
        auto fn = [tmp, start, stop](auto offset) noexcept {
	        std::stringstream storage;
            for (auto const & t8 : tmp) {
                auto product = t8.getProduct();
                auto sum = t8.getSum();
                auto num = t8.getNumber() * fastPow10<3>;
                fourthBody(storage, sum + offset, product * offset, num + offset);
	        }
	        return storage.str();
        };
        return std::async(std::launch::async, fn, offset);
    };
    std::stringstream storage;
    auto p0 = mkBody(2);
    auto p1 = mkBody(4);
    auto p2 = mkBody(6);
    auto p3 = mkBody(8);
	storage << p0.get() << p1.get() << p2.get() << p3.get();
    return storage.str();
}

int main() {
    auto errorCode = 0;
    constexpr auto workUnitWidth = 5;
    constexpr auto fallOver = 8 - workUnitWidth;
    constexpr auto workUnitCount = numElements<workUnitWidth>;
    constexpr auto oneSeventhWorkUnit = workUnitCount / 7;
	std::stringstream collection0;
    // setup the triples
    populateWidth<2>();
    auto t2 = getTriples<2>();
    for (auto& r24 : range2To4) {
        r24.assume(t2->getSum(), t2->getProduct(), t2->getNumber() * fastPow10<1>);
        ++t2;
    }
	populateWidth<7>();
    auto t8 = getTriples<7>();
    for (auto& r1219 : range12To18) {
        r1219.assume(t8->getSum(), t8->getProduct(), t8->getNumber() * fastPow10<11>);
        ++t8;
    }
    populateWidth<8>();
    auto fn = [](auto start, auto stop) noexcept {
        return std::async(std::launch::async, doIt<oneSeventhWorkUnit>, start, stop);
    };
    while(std::cin.good()) {
        int innerThreadId = 0;
        std::cin >> innerThreadId;
        if (innerThreadId < 0 || innerThreadId >= numElements<fallOver>) {
            std::cerr << "Illegal inner thread id, must be in the range [0," << numElements<fallOver> - 1 << "]" << std::endl;
            errorCode = 1;
            break;
        }
        if (!std::cin.good()) {
            break;
        }
        // divide the code up into seven parts
		auto start = workUnitCount * innerThreadId;
        auto stop0 = oneSeventhWorkUnit + start;
        auto stop1 = oneSeventhWorkUnit + stop0;
        auto stop2 = oneSeventhWorkUnit + stop1;
        auto stop3 = oneSeventhWorkUnit + stop2;
        auto stop4 = oneSeventhWorkUnit + stop3;
        auto stop5 = oneSeventhWorkUnit + stop4;
        auto stop6 = oneSeventhWorkUnit + stop5;
		auto stop = workUnitCount * (innerThreadId + 1);
        if (stop != stop6) {
            errorCode = 1;
            std::cerr << "size mismatch!" << std::endl;
            break;
        }
		auto b0 = fn(start, stop0);
        auto b1 = fn(stop0, stop1);
        auto b2 = fn(stop1, stop2);
		auto b3 = fn(stop2, stop3);
		auto b4 = fn(stop3, stop4);
		auto b5 = fn(stop4, stop5);
		auto b6 = fn(stop5, stop6);
		collection0 << b0.get() << b1.get() << b2.get() << b3.get() << b4.get() << b5.get() << b6.get();
    }
	std::cout << collection0.str() << std::endl;
    return errorCode;
}
