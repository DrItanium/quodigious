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
#include <vector>
#include <sstream>
#include <functional>
#include <future>
#include "qlib.h"

template<u64 length, bool skipFives = false>
inline void loopBody(u64 sum, u64 product, u64 index, std::ostream& storage) noexcept {
    constexpr auto inner = length - 1;
    constexpr auto next = fastPow10<inner>;
    loopBody<inner, skipFives>(2 + sum, multiply<2>(product), index + (multiply<2>(next)), storage);
    loopBody<inner, skipFives>(3 + sum, multiply<3>(product), index + (multiply<3>(next)), storage);
    loopBody<inner, skipFives>(4 + sum, multiply<4>(product), index + (multiply<4>(next)), storage);
    if (!skipFives) {
        loopBody<inner, skipFives>(5 + sum, multiply<5>(product), index + multiply<5>(next), storage);
    }
    loopBody<inner, skipFives>(6 + sum, multiply<6>(product), index + multiply<6>(next), storage);
    loopBody<inner, skipFives>(7 + sum, multiply<7>(product), index + multiply<7>(next), storage);
    loopBody<inner, skipFives>(8 + sum, multiply<8>(product), index + multiply<8>(next), storage);
    loopBody<inner, skipFives>(9 + sum, multiply<9>(product), index + multiply<9>(next), storage);
}

template<u64 k>
inline u64 innerMostBody(u64 sum, u64 product, u64 index) noexcept {
    auto l1Sum = sum + k;
    auto l1Value = k + index;
    if (componentQuodigious(l1Value, l1Sum)) {
        auto l1Product = multiply<k>(product);
        if (l1Sum == l1Product) {
            return l1Value;
        } else {
            if (componentQuodigious(l1Value, l1Product)) {
                return l1Value;
            }
        }
    }
    return 0;
}
void merge(u64 value, std::ostream& storage) noexcept {
    if (value != 0) {
        storage << value << std::endl;
    }
}
template<>
inline void loopBody<1, false>(u64 sum, u64 product, u64 index, std::ostream& storage) noexcept {
    merge(innerMostBody<2>(sum, product, index), storage);
    merge(innerMostBody<3>(sum, product, index), storage);
    merge(innerMostBody<4>(sum, product, index), storage);
    merge(innerMostBody<5>(sum, product, index), storage);
    merge(innerMostBody<6>(sum, product, index), storage);
    merge(innerMostBody<7>(sum, product, index), storage);
    merge(innerMostBody<8>(sum, product, index), storage);
    merge(innerMostBody<9>(sum, product, index), storage);
}

template<>
inline void loopBody<1, true>(u64 sum, u64 product, u64 index, std::ostream& storage) noexcept {
    merge(innerMostBody<2>(sum, product, index), storage);
    merge(innerMostBody<3>(sum, product, index), storage);
    merge(innerMostBody<4>(sum, product, index), storage);
    merge(innerMostBody<6>(sum, product, index), storage);
    merge(innerMostBody<7>(sum, product, index), storage);
    merge(innerMostBody<8>(sum, product, index), storage);
    merge(innerMostBody<9>(sum, product, index), storage);
}

template<u64 length>
inline void body(std::ostream& storage) noexcept {
	// this is not going to change ever!
    static constexpr auto skipFive = length > 4;
    loopBody<length, skipFive>(0, 1, 0, storage);
}

int main() {
    std::ostringstream storage;
	while(std::cin.good()) {
		u64 currentIndex = 0;
		std::cin >> currentIndex;
		if (std::cin.good()) {
			switch(currentIndex) {
				case 1: body<1>(storage); break;
				case 2: body<2>(storage); break;
				case 3: body<3>(storage); break;
				case 4: body<4>(storage); break;
				case 5: body<5>(storage); break;
				case 6: body<6>(storage); break;
				case 7: body<7>(storage); break;
				case 8: body<8>(storage); break;
				case 9: body<9>(storage); break;
				case 10: body<10>(storage); break;
				case 11: body<11>(storage); break;
				case 12: body<12>(storage); break;
				case 13: body<13>(storage); break;
				case 14: body<14>(storage); break;
				case 15: body<15>(storage); break;
				case 16: body<16>(storage); break;
				case 17: body<17>(storage); break;
				case 18: body<18>(storage); break;
				case 19: body<19>(storage); break;
				default:
						 std::cerr << "Illegal index " << currentIndex << std::endl;
						 return 1;
			}
            std::cout << storage.str() << std::endl;
            storage.str("");
		}
	}
	return 0;
}
