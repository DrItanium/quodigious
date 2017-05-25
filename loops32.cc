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
#include "qlib.h"


template<u32 length>
inline void body(u32 sum = 0, u32 product = 1, u32 index = 0) noexcept {
    static_assert(length <= 9, "Can't have numbers over 9 digits on 32-bit numbers!");
    constexpr auto inner = length - 1;
    constexpr auto next = fastPow10<inner>;
    auto advance = [&sum]() noexcept { ++sum; };
    sum += 2;
    body<inner>(sum, multiply<2>(product), index + (multiply<2>(next)));
    advance();
    body<inner>(sum, multiply<3>(product), index + (multiply<3>(next)));
    advance();
    body<inner>(sum, multiply<4>(product), index + (multiply<4>(next)));
    advance();
    body<inner>(sum, multiply<5>(product), index + multiply<5>(next));
    advance();
    body<inner>(sum, multiply<6>(product), index + multiply<6>(next));
    advance();
    body<inner>(sum, multiply<7>(product), index + multiply<7>(next));
    advance();
    body<inner>(sum, multiply<8>(product), index + multiply<8>(next));
    advance();
    body<inner>(sum, multiply<9>(product), index + multiply<9>(next));
}




template<>
inline void body<1>(u32 sum, u32 product, u32 index) noexcept {
    auto advance = [&sum, &index]() noexcept { ++sum; ++index; };
    auto innerMostBody = [](auto sum, auto product, auto value) noexcept { return isQuodigious(value, sum, product) ? value : 0; };
    sum += 2;
    index += 2;
    merge(innerMostBody(sum, multiply<2>(product), index), std::cout);
    advance();
    merge(innerMostBody(sum, multiply<3>(product), index), std::cout);
    advance();
    merge(innerMostBody(sum, multiply<4>(product), index), std::cout);
    advance();
    merge(innerMostBody(sum, multiply<5>(product), index), std::cout);
    advance();
    merge(innerMostBody(sum, multiply<6>(product), index), std::cout);
    advance();
    merge(innerMostBody(sum, multiply<7>(product), index), std::cout);
    advance();
    merge(innerMostBody(sum, multiply<8>(product), index), std::cout);
    advance();
    merge(innerMostBody(sum, multiply<9>(product), index), std::cout);
}

int main() {
    while(std::cin.good()) {
        u32 currentIndex = 0;
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
                default:
                        std::cerr << "Illegal index " << currentIndex << std::endl;
                        return 1;
            }
            std::cout << std::endl;
        }
    }
    return 0;
}
