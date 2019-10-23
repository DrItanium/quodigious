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


void performQuodigious(uint8_t depth, u64 number = 0, u64 sum = 0, u64 product = 1) noexcept {
    if (depth == 0) {
        if (isQuodigious(number, sum, product)) {
            std::cout << number << std::endl;
        }
    } else {
        auto innerDepth = depth - 1;
        auto baseFactor = factors10[innerDepth];
        // this will eliminate multiplies
        number += (baseFactor << 1); // always will have a minimum of baseFactor * 2
        sum += 2; // always will be two more than we started with
        // hand unroll to expose more optimization surface area
        performQuodigious(innerDepth, number, sum, product * 2);
        number += baseFactor;
        ++sum;
        performQuodigious(innerDepth, number, sum, product * 3);
        number += baseFactor;
        ++sum;
        performQuodigious(innerDepth, number, sum, product * 4);
        number += baseFactor;
        ++sum;
        performQuodigious(innerDepth, number, sum, product * 5);
        number += baseFactor;
        ++sum;
        performQuodigious(innerDepth, number, sum, product * 6);
        number += baseFactor;
        ++sum;
        performQuodigious(innerDepth, number, sum, product * 7);
        number += baseFactor;
        ++sum;
        performQuodigious(innerDepth, number, sum, product * 8);
        number += baseFactor;
        ++sum;
        performQuodigious(innerDepth, number, sum, product * 9);
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
