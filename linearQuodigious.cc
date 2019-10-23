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
#include <list>

void performQuodigious(uint8_t depth, u64 number = 0, u64 sum = 0, u64 product = 1) noexcept {
    if (depth == 0) {
        if (isQuodigious(number, sum, product)) {
            std::cout << number << std::endl;
        }
    } else {
        auto innerDepth = depth - 1;
        auto currentFactor = factors10[innerDepth];
        ++sum; // always add one to start with
        for (auto i = 2; i < 10; ++i) {
            ++sum;
            performQuodigious(innerDepth, number + (i * currentFactor), sum, product * i);
        }
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
                std::cerr << "Illegal index " << currentIndex << std::endl;
                return 1;
            }
            std::cout << std::endl;
        }
    }
    return 0;
}
