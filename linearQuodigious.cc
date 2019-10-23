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

// Perform numeric quodigious checks using a special encoding.
//
// This special encoding uses the concept of octal but subtracts two from each
// digit and uses three bits to represent each digit separately. Printing out
// the number would show garbage in base ten or any normal number. For
// instance, the number '0' in the encoding is 2. 00 is 22, 000 is 222 and so
// on. 7 is 9, 77 is 99 and so forth. There is no way to represent one or zero
// in the encoding so it is perfect for this design.
// decimal would be
#include "qlib.h"
#include <iostream>

inline constexpr u64 factors10[] = {
    fastPow10<0>,  fastPow10<1>,  fastPow10<2>,  fastPow10<3>,
    fastPow10<4>,  fastPow10<5>,  fastPow10<6>,  fastPow10<7>,
    fastPow10<8>,  fastPow10<9>,  fastPow10<10>, fastPow10<11>,
    fastPow10<12>, fastPow10<13>, fastPow10<14>, fastPow10<15>,
    fastPow10<16>, fastPow10<17>, fastPow10<18>, fastPow10<19>,
};

void performQuodigious(uint8_t depth, u64 number = 0, u64 sum = 0, u64 product = 1) noexcept {
    if (depth == 0) {
        if (isQuodigious(number, sum, product)) {
            std::cout << number << std::endl;
        }
    } else {
        auto innerDepth = depth - 1;
        auto baseFactor = factors10[innerDepth];
        // this will eliminate multiplies
        auto computableFactor = baseFactor << 1;
#if 0
        for (auto i = 2; i < 10; ++i) {
            performQuodigious(innerDepth, number + computableFactor, sum + i, product * i);
            computableFactor += baseFactor;
        }
#endif
        performQuodigious(innerDepth, number + computableFactor, sum + 2, product * 2);
        computableFactor += baseFactor;
        performQuodigious(innerDepth, number + computableFactor, sum + 3, product * 3);
        computableFactor += baseFactor;
        performQuodigious(innerDepth, number + computableFactor, sum + 4, product * 4);
        computableFactor += baseFactor;
        performQuodigious(innerDepth, number + computableFactor, sum + 5, product * 5);
        computableFactor += baseFactor;
        performQuodigious(innerDepth, number + computableFactor, sum + 6, product * 6);
        computableFactor += baseFactor;
        performQuodigious(innerDepth, number + computableFactor, sum + 7, product * 7);
        computableFactor += baseFactor;
        performQuodigious(innerDepth, number + computableFactor, sum + 8, product * 8);
        computableFactor += baseFactor;
        performQuodigious(innerDepth, number + computableFactor, sum + 9, product * 9);
        computableFactor += baseFactor;
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
