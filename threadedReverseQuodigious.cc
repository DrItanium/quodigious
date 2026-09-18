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
#include <iomanip>
#include <future>
#include <vector>
#include <type_traits>

using StorageCell = std::vector<u64>;

template<uint8_t depth, uint8_t currentDepth = 0, u64 factor = 1>
void 
performQuodigious(StorageCell& cell, u64 number, u64 sum, u64 product) noexcept {
    constexpr auto nextDepth = currentDepth + 1;
    for (u64 i = 2; i < 10; ++i) {
        auto tNum = number + (i * factor);
        auto tSum = sum + i;
        auto tProd = product * i;
        if (isQuodigious(tNum, tSum, tProd)) {
            //std::cout << std::right << std::setw(32) << std::dec << tNum << std::endl;
            cell.emplace_back(tNum);
        }
        if constexpr (nextDepth < depth) {
            performQuodigious<depth, nextDepth, factor * 10>(cell, tNum, tSum, tProd);
        }
    }
}

template<uint8_t depth>
void
performQuodigious() noexcept {
    StorageCell cells[8];
    auto fn = [&cells](u64 base) {
        auto& cell = cells[base - 2];
        performQuodigious<depth, 1, 10>(cell, base, base, base);
    };
    decltype(std::async(std::launch::async, fn, 2)) storageElements[8];

    // don't waste time checking quodigiousness of single digits
    for (int i = 0, j = 2; i < 8; ++i, ++j) {
        cells[i].emplace_back(j);
    }
    auto display = [](auto& cell) {
        for (auto value : cell) {
            std::cout << std::right << std::setw(32) << std::dec << value << std::endl;
        }
    };
    for (int i = 0, j = 2; i < 8; ++i, ++j) {
        storageElements[i] = std::async(std::launch::async, fn, j);
    }
    for (int i = 0; i < 8; ++i) {
        storageElements[i].wait(); 
        display(cells[i]);
    }

}


void doQuodigious(uint8_t depth) noexcept {
    switch (depth) {
#define X(ind) case ind : performQuodigious< ind > (); break
        X(1);
        X(2);
        X(3);
        X(4);
        X(5);
        X(6);
        X(7);
        X(8);
        X(9);
        X(10);
        X(11);
        X(12);
        X(13);
        X(14);
        X(15);
        X(16);
        X(17);
        X(18);
        X(19);
#undef X
        default:
            std::cout << "Illegal depth: " << static_cast<int>(depth) << std::endl;
            break;
    }
}



int main() {
    while(std::cin.good()) {
        u64 currentIndex = 0;
        std::cin >> currentIndex;
        if (std::cin.good()) {
            if ((currentIndex > 0) && (currentIndex < 20)) {
                doQuodigious(currentIndex);
            } else {
                std::cout << "Illegal index " << currentIndex << std::endl;
                return 1;
            }
            std::cout << std::endl;
        }
    }
    return 0;
}
