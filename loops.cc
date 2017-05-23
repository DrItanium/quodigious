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

template<u64 length, bool skipFives>
inline void loopBody(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept {
    constexpr auto inner = length - 1;
    constexpr auto next = fastPow10<inner>;
    loopBody<inner, skipFives>(storage, 2 + sum, multiply<2>(product), index + (multiply<2>(next)));
    loopBody<inner, skipFives>(storage, 3 + sum, multiply<3>(product), index + (multiply<3>(next)));
    loopBody<inner, skipFives>(storage, 4 + sum, multiply<4>(product), index + (multiply<4>(next)));
    if (!skipFives) {
        loopBody<inner, skipFives>(storage, 5 + sum, multiply<5>(product), index + multiply<5>(next));
    }
    loopBody<inner, skipFives>(storage, 6 + sum, multiply<6>(product), index + multiply<6>(next));
    loopBody<inner, skipFives>(storage, 7 + sum, multiply<7>(product), index + multiply<7>(next));
    loopBody<inner, skipFives>(storage, 8 + sum, multiply<8>(product), index + multiply<8>(next));
    loopBody<inner, skipFives>(storage, 9 + sum, multiply<9>(product), index + multiply<9>(next));
}

template<u64 k>
inline u64 innerMostBody(u64 sum, u64 product, u64 index) noexcept {
    static_assert(k < 10, "K can't be wider than 10!");
    auto l1Sum = sum + k;
    auto l1Value = k + index;
    if (componentQuodigious(l1Value, l1Sum)) {
        auto l1Product = multiply<k>(product);
        // research code
        auto tmp = [&l1Product, &l1Sum, &l1Value]() {
            auto divisibleBy = [](auto value, auto against) { return (value % against) == 0; };
            //auto divisibleByThree = [divisibleBy](auto value) { return divisibleBy(value, 3); };
            auto divisibleByTwo = [divisibleBy](auto value) { return divisibleBy(value, 2); };
            //auto divisibleBySeven = [divisibleBy](auto value) { return divisibleBy(value, 7); };
            auto divisibleByNine = [divisibleBy](auto value) { return divisibleBy(value, 9); };
            std::cout << "value: " << l1Value << " sum: " << l1Sum << " product: " << l1Product;
            if (!divisibleByTwo(l1Sum)) {
                std::cout << "\n\tSum is odd!";
            }
            if (!divisibleByTwo(l1Product)) {
                std::cout << "\n\tProduct is odd!";
            }
            //std::cout << "\n\tSum divisible by three: " << divisibleByThree(l1Sum);
            //std::cout << "\n\tproduct divisible by three: " << divisibleByThree(l1Product);
            //std::cout << "\n\tvalue divisible by three: " << divisibleByThree(l1Value);
            //std::cout << "\n\tSum divisible by two: " << divisibleByTwo(l1Sum);
            //std::cout << "\n\tproduct divisible by two: " << divisibleByTwo(l1Product);
            //std::cout << "\n\tvalue divisible by two: " << divisibleByTwo(l1Value);
            //std::cout << "\n\tSum divisible by seven: " << divisibleBySeven(l1Sum);
            //std::cout << "\n\tproduct divisible by seven: " << divisibleBySeven(l1Product);
            //std::cout << "\n\tvalue divisible by seven: " << divisibleBySeven(l1Value);
            //std::cout << "\n\tSum divisible by nine: " << divisibleByNine(l1Sum);
            //std::cout << "\n\tProduct divisible by nine: " << divisibleByNine(l1Product);
            //std::cout << "\n\tProduct / sum: " << l1Product / l1Sum << std::endl;
            //std::cout << "\n\tsum divisor: " << (l1Value / l1Sum);
            //std::cout << "\n\tproduct divisor: " << (l1Value / l1Product);
            std::cout << std::endl;
        };
        if (l1Sum == l1Product) {
            tmp();
            return l1Value;
        } else if (componentQuodigious(l1Value, l1Product)) {
            tmp();
            return l1Value;
        }
    }
    return 0;
}
inline void merge(u64 value, std::ostream& storage) noexcept {
    if (value != 0) {
        storage << value << std::endl;
    }
}
template<>
inline void loopBody<1, false>(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept {
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
inline void loopBody<1, true>(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept {
    merge(innerMostBody<2>(sum, product, index), storage);
    merge(innerMostBody<4>(sum, product, index), storage);
    merge(innerMostBody<6>(sum, product, index), storage);
    merge(innerMostBody<8>(sum, product, index), storage);
}

template<u64 length, bool skipFives, u64 pos>
inline std::string parallelBody() noexcept {
    std::string out;
    if (pos == 5 && skipFives) {
        return out;
    }
    std::ostringstream storage;
    loopBody<length - 1, skipFives>(storage, pos, pos, multiply<pos>(fastPow10<length - 1>));
    out = storage.str();
    return out;
}

template<u64 length, bool skipFives>
inline void loopBody(std::ostream& storage) noexcept {
    if (length > 7) {
        auto b2 = std::async(std::launch::async, parallelBody<length, skipFives, 2>);
        auto b3 = std::async(std::launch::async, parallelBody<length, skipFives, 3>);
        auto b4 = std::async(std::launch::async, parallelBody<length, skipFives, 4>);
        auto b5 = std::async(std::launch::async, parallelBody<length, skipFives, 5>);
        auto b6 = std::async(std::launch::async, parallelBody<length, skipFives, 6>);
        auto b7 = std::async(std::launch::async, parallelBody<length, skipFives, 7>);
        auto b8 = std::async(std::launch::async, parallelBody<length, skipFives, 8>);
        auto b9 = std::async(std::launch::async, parallelBody<length, skipFives, 9>);
        storage << b2.get() << b3.get() << b4.get() << b5.get() << b6.get();
        storage << b7.get() << b8.get() << b9.get();
    } else {
        loopBody<length, skipFives>(storage, 0, 1, 0);
    }
}

template<> inline void loopBody<1, false>(std::ostream& storage) noexcept { loopBody<1, false>(storage, 0, 1, 0); }

template<u64 length>
inline void body(std::ostream& storage) noexcept {
	// this is not going to change ever!
    loopBody<length, (length > 4)>(storage);
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
