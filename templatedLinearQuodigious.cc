//  Copyright (c) 2017-2019 Joshua Scoggins
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

#include "qlib.h"
#include <iostream>


template<uint8_t depth>
void performQuodigious(u64 number = 0, u64 sum = 0, u64 product = 1) noexcept {
    static_assert(depth < 20, "Too large of a number");
    if constexpr (depth == 0) {
        if (isQuodigious(number, sum, product)) {
            std::cout << number << std::endl;
        }
    } else {
        static constexpr auto innerDepth = depth - 1;
        static constexpr auto baseFactor = factors10[innerDepth];
        // this will eliminate multiplies
        number += (baseFactor << 1); // always will have a minimum of baseFactor * 2
        sum += 2; // always will be two more than we started with
        // hand unroll to expose more optimization surface area
        performQuodigious<innerDepth>(number, sum, product * 2);
        number += baseFactor;
        ++sum;
        performQuodigious<innerDepth>(number, sum, product * 3);
        number += baseFactor;
        ++sum;
        performQuodigious<innerDepth>(number, sum, product * 4);
        number += baseFactor;
        ++sum;
        performQuodigious<innerDepth>(number, sum, product * 5);
        number += baseFactor;
        ++sum;
        performQuodigious<innerDepth>(number, sum, product * 6);
        number += baseFactor;
        ++sum;
        performQuodigious<innerDepth>(number, sum, product * 7);
        number += baseFactor;
        ++sum;
        performQuodigious<innerDepth>(number, sum, product * 8);
        number += baseFactor;
        ++sum;
        performQuodigious<innerDepth>(number, sum, product * 9);
    }
}
void performQuodigious(uint8_t depth) noexcept {
    switch (depth) {
        case 1: 
            []() noexcept {
                for (u64 i = 2; i < 10; ++i) {
                    if (isQuodigious(i, i, i)) {
                        std::cout << i << std::endl;
                    }
                }
            }();
            break;
#define X(length) case length : performQuodigious<length> (); break
        X(2); 
        X(3);  X(4);
        X(5);  X(6); 
        X(7);  X(8); 
        X(9);  X(10); 
        X(11); X(12);
        X(13); X(14);
        X(15); X(16);
        X(17); X(18);
        X(19); 
        default: break;
#undef X
    }
}
int main() {
    while(std::cin.good()) {
        u64 currentIndex = 0;
        std::cin >> currentIndex;
        if (std::cin.good()) {
            if ((currentIndex > 0) && (currentIndex < 20)) {
                performQuodigious(currentIndex);
            } else {
                std::cout << "Illegal index " << currentIndex << std::endl;
                return 1;
            }
            std::cout << std::endl;
        }
    }
    return 0;
}
