//  Copyright (c) 2018 Joshua Scoggins
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

// Perform numeric quodigious checks
#include "qlib.h"
#include <future>
#include <list>
template<u64 length>
void body(std::list<u64>& values, u64 sum = 0, u64 product = 1, u64 index = 0) noexcept {
	static_assert(length >= 10, "Cannot go lower than 10 digits!");
    if constexpr (length == 10) {
        if (sum % 3 != 0) {
            return;
        }
        if ((index % product == 0) && (index % sum == 0)) {
            values.emplace_back(index);
        }
    } else if constexpr(length >= 18) {
	    constexpr auto inner = length - 1;
	    constexpr auto next = fastPow10<inner>;
        auto t0 = std::async(std::launch::async, [](auto s, auto p, auto idx) {
                    std::list<u64> _values;
                    body<inner>(_values, s + 2, p * 2 , idx + (2 * next));
                    return _values;
                }, sum, product, index);
        auto t1 = std::async(std::launch::async, [](auto s, auto p, auto idx) {
                    std::list<u64> _values;
                    body<inner>(_values, s + 3, p * 3 , idx + (3 * next));
                    return _values;
                }, sum, product, index);
        auto t2 = std::async(std::launch::async, [](auto s, auto p, auto idx) {
                    std::list<u64> _values;
                    body<inner>(_values, s + 4, p * 4 , idx + (4 * next));
                    return _values;
                }, sum, product, index);
        auto t3 = std::async(std::launch::async, [](auto s, auto p, auto idx) {
                    std::list<u64> _values;
                    body<inner>(_values, s + 6, p * 6 , idx + (6 * next));
                    return _values;
                }, sum, product, index);
        auto t4 = std::async(std::launch::async, [](auto s, auto p, auto idx) {
                    std::list<u64> _values;
                    body<inner>(_values, s + 7, p * 7 , idx + (7 * next));
                    return _values;
                }, sum, product, index);
        auto t5 = std::async(std::launch::async, [](auto s, auto p, auto idx) {
                    std::list<u64> _values;
                    for (auto i = 8; i < 10; ++i) {
                        body<inner>(_values, s + i, p * i , idx + (i * next));
                    }
                    return _values;
                }, sum, product, index);
        auto v0 = t0.get();
        values.splice(values.cbegin(), v0);
        auto v1 = t1.get();
        values.splice(values.cbegin(), v1);
        auto v2 = t2.get();
        values.splice(values.cbegin(), v2);
        auto v3 = t3.get();
        values.splice(values.cbegin(), v3);
        auto v4 = t4.get();
        values.splice(values.cbegin(), v4);
        auto v5 = t5.get();
        values.splice(values.cbegin(), v5);
    } else if constexpr(length >= 15) {
	    constexpr auto inner = length - 1;
	    constexpr auto next = fastPow10<inner>;
        auto t0 = std::async(std::launch::async, [](auto s, auto p, auto idx) {
                    std::list<u64> _values;
                    for (auto i = 2; i < 5; ++i) {
                        body<inner>(_values, s + i, p * i , idx + (i * next));
                    }
                    return _values;
                }, sum, product, index);
        auto t1 = std::async(std::launch::async, [](auto s, auto p, auto idx) {
                    std::list<u64> _values;
                    for (auto i = 6; i < 10; ++i) {
                        body<inner>(_values, s + i, p * i , idx + (i * next));
                    }
                    return _values;
                }, sum, product, index);
        auto v0 = t0.get();
        values.splice(values.cbegin(), v0);
        auto v1 = t1.get();
        values.splice(values.cbegin(), v1);
    } else {
	    constexpr auto inner = length - 1;
	    constexpr auto next = fastPow10<inner>;
	    // unlike the 64-bit version of this code, doing the 32-bit ints for 9 digit
	    // numbers (this code is not used when you request 64-bit numbers!)
	    // does not require as much optimization. We can walk through digit level
	    // by digit level (even if the digit does not contribute too much to the
	    // overall process!).
	    for (auto i = 2; i < 10; ++i) {
            if (i != 5) {
                body<inner>(values, sum + i, product * i, index + (i * next));
            }
	    }
    }
}




int main(int argc, char** argv) {
    if (argc != 2) {
        return 1;
    }
    u64 currentWidth = 0;
    std::string tmp(argv[1]);
    std::istringstream w(tmp);
    w >> currentWidth;
	while(std::cin.good()) {
        std::list<u64> values;
		u64 currentIndex = 0;
		u64 currentSum = 0;
		u64 currentProduct = 0;
		std::cin >> currentSum;
        if (!std::cin.good()) { break; }
		std::cin >> currentProduct;
        if (!std::cin.good()) { break; }
		std::cin >> currentIndex;
		if (std::cin.good()) {
            if (auto div = currentIndex % 10; div == 3 || div == 5 || div == 7 || div == 9) {
                continue;
            }
			switch(currentWidth) {
				case 11: body<11>(values, currentSum, currentProduct, currentIndex); break;
				case 12: body<12>(values, currentSum, currentProduct, currentIndex); break;
				case 13: body<13>(values, currentSum, currentProduct, currentIndex); break;
				case 14: body<14>(values, currentSum, currentProduct, currentIndex); break;
				case 15: body<15>(values, currentSum, currentProduct, currentIndex); break;
				case 16: body<16>(values, currentSum, currentProduct, currentIndex); break;
				case 17: body<17>(values, currentSum, currentProduct, currentIndex); break;
				case 18: body<18>(values, currentSum, currentProduct, currentIndex); break;
				case 19: body<19>(values, currentSum, currentProduct, currentIndex); break;
				default:
						 std::cerr << "Illegal index " << currentIndex << std::endl;
						 return 1;
			}
            for (auto v : values) {
                std::cout << v << std::endl;
            }
		}
	}
	return 0;
}
