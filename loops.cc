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

template<u64 length>
void loopBody(u64 sum, u64 product, u64 index, vec64& storage) noexcept {
    for (int i = 2; i < 10; ++i) {
        auto mergedSum = i + sum;
        auto mergedProduct = i * product;
        auto mergedIndex = (fastPow10<length - 1> * i) + index;
        loopBody<length - 1>(mergedSum, mergedProduct, mergedIndex, storage);
    }
}

template<>
void loopBody<0>(u64 sum, u64 product, u64 index, vec64& storage) noexcept {
    for (auto k = 2; k < 10; ++k) {
        auto l1Sum = sum + k;
        auto l1Value = k + index;
        if (componentQuodigious(l1Value, l1Sum)) {
            auto l1Product = product * k;
            if (componentQuodigious(l1Value, l1Product)) {
                storage.emplace_back(l1Value);
            }
        }
    }
}

void printout(vec64& l) noexcept {
	for (auto v : l) {
		std::cout << v << std::endl;
	}
	l.clear();
}

template<u64 length>
inline void body() noexcept {
    static vec64 storage;
	// this is not going to change ever!
    loopBody<length>(0,1,0,storage);
	printout(storage);
	std::cout << std::endl;
}

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
