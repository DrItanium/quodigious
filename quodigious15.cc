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
#include <fstream>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <functional>
#include <future>
#include <map>
#include "qlib.h"

constexpr auto dimensionCount = 8;
constexpr auto expectedDimensionCount = dimensionCount + 1;
constexpr auto primaryDataCacheSize = dataCacheSize<dimensionCount>;
container primaryDataCache[primaryDataCacheSize];
constexpr auto secondaryDimensionCount = 5;
constexpr auto secondaryDataCacheSize = dataCacheSize<secondaryDimensionCount>;

container secondaryDataCache[secondaryDataCacheSize];

template<u64 pos, u64 index>
decltype(auto) makeWorker() noexcept {
    return std::async(std::launch::async, typicalInnerMostBody<15, primaryDataCacheSize, secondaryDataCacheSize, 21>, pos, pos, index, primaryDataCache, secondaryDataCache);
}

int main() {
	if (!loadDataCache<1>("cache.bin", primaryDataCache, primaryDataCacheSize) || !loadDataCache<9>("cache5.bin", secondaryDataCache, secondaryDataCacheSize)) {
        return 1;
    }
    static constexpr auto next = fastPow10<15 - 1>;
    static constexpr auto doubleNext = next << 1;
    auto p0 = makeWorker<2, doubleNext>(); // 2
    auto p1 = makeWorker<3, doubleNext + next>(); // 3
    auto p2 = makeWorker<4, (2 * doubleNext)>(); // 4
    auto p3 = makeWorker<6, (3 * doubleNext)>(); // 6
    auto p4 = makeWorker<7, (3 * doubleNext) + next>(); // 7
    auto p5 = makeWorker<8, (4 * doubleNext)>(); // 8
    auto p6 = makeWorker<9, (4 * doubleNext) + next>(); // 9
    std::cout << p0.get() << p1.get() << p2.get() << p3.get() << p4.get() << p5.get() << p6.get();
    return 0;
}
