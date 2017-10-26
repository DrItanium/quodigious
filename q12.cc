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

constexpr bool checkValue(u64 sum) noexcept {
	return (isEven(sum)) || (sum % 3 == 0);
}
constexpr u64 inspectValue(u64 value, u64 sum, u64 product) noexcept {
    if (checkValue(sum) && isQuodigious(value, sum, product)) {
        return value;
    }
    return 0;
}
void innerMostBody(std::ostream& stream, u64 sum, u64 product, u64 value) noexcept;

using container = std::tuple<u64, u64, u64>;

template<u64 pos, u64 max>
inline void loopBody(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept {
	static_assert (pos == max, "Position can't be larger than maximum!");
	// walk through two separate set of actions
}
template<u64 count>
constexpr auto dataCacheSize = numElements<count>;

constexpr auto dimensionCount = 8;
constexpr auto expectedDimensionCount = dimensionCount + 1;
constexpr auto primaryDataCacheSize = dataCacheSize<dimensionCount>;
container primaryDataCache[primaryDataCacheSize];
constexpr auto secondaryDimensionCount = 2;
constexpr auto secondaryDataCacheSize = dataCacheSize<secondaryDimensionCount>;

container secondaryDataCache[secondaryDataCacheSize];

void innerMostBody(std::ostream& stream, u64 sum, u64 product, u64 value) noexcept {
    // the last digit of all numbers is 2, 4, 6, or 8 so ignore the others and compute this right now
    for (const auto& outer : primaryDataCache) {
        u64 ov, os, op;
        std::tie(ov, os, op) = outer;
        ov += value;
        os += sum;
        op *= product;
        for (const auto& inner : secondaryDataCache) {
            u64 iv, is, ip;
            std::tie(iv, is, ip) = inner;
            iv += ov;
            is += os;
            ip *= op;
            merge(inspectValue(iv + 2, is + 2, ip << 1), stream);
            merge(inspectValue(iv + 4, is + 4, ip << 2), stream);
            merge(inspectValue(iv + 6, is + 6, ip * 6), stream);
            merge(inspectValue(iv + 8, is + 8, ip << 3), stream);
        }
    }
}

bool loadPrimaryDataCache() noexcept {
	std::ifstream cachedCopy("cache.bin", std::ifstream::in | std::ifstream::binary);
	if (!cachedCopy.good()) {
		std::cerr << "ERROR Couldn't open cache.bin data cache! Make sure it exists and is named cache.bin" << std::endl;
        return false;
	}
	// TODO: update the binary generator to eliminate the last digit from the
	// computation, then we can do four numbers at a time in the inner most
	// loop of this code which should make up the computation difference we've
	// seen so far!
	//
	// We can also reintroduce nested threading, the difference is that we can
	// just eliminate the values needed
	char tmpCache[sizeof(u32) * 3] = { 0 };
	for (int i = 0; i < primaryDataCacheSize || cachedCopy.good(); ++i) {
		// layout is value, sum, product
		cachedCopy.read(tmpCache, sizeof(u32) * 3);
		u64 value = (uint8_t)tmpCache[0];
		value |= (static_cast<u64>((uint8_t)tmpCache[1]) << 8);
		value |= (static_cast<u64>((uint8_t)tmpCache[2]) << 16);
		value |= (static_cast<u64>((uint8_t)tmpCache[3]) << 24);
		u64 sum = (uint8_t)tmpCache[4];
		sum |= (static_cast<u64>((uint8_t)tmpCache[5]) << 8);
		sum |= (static_cast<u64>((uint8_t)tmpCache[6]) << 16);
		sum |= (static_cast<u64>((uint8_t)tmpCache[7]) << 24);
		u64 product = (uint8_t)tmpCache[8];
		product |= (static_cast<u64>((uint8_t)tmpCache[9]) << 8);
		product |= (static_cast<u64>((uint8_t)tmpCache[10]) << 16);
		product |= (static_cast<u64>((uint8_t)tmpCache[11]) << 24);
        // multiply the value by 10 so we get an extra digit out of it, our dimensions become 9 in the process though!
		primaryDataCache[i] = std::make_tuple(value * 10, sum, product);
	}
	if (!cachedCopy.eof()) {
		std::cerr << "data cache is too small!" << std::endl;
        return false;
	}

	cachedCopy.close();
    return true;
}

bool loadSecondaryDataCache() noexcept {
	std::ifstream cachedCopy("cache2.bin", std::ifstream::in | std::ifstream::binary);
	if (!cachedCopy.good()) {
		std::cerr << "ERROR Couldn't open cache.bin data cache! Make sure it exists and is named cache2.bin" << std::endl;
        return false;
	}
	// TODO: update the binary generator to eliminate the last digit from the
	// computation, then we can do four numbers at a time in the inner most
	// loop of this code which should make up the computation difference we've
	// seen so far!
	//
	// We can also reintroduce nested threading, the difference is that we can
	// just eliminate the values needed
	char tmpCache[sizeof(u32) * 3] = { 0 };
	for (int i = 0; i < secondaryDataCacheSize || cachedCopy.good(); ++i) {
		// layout is value, sum, product
		cachedCopy.read(tmpCache, sizeof(u32) * 3);
		u64 value = (uint8_t)tmpCache[0];
		value |= (static_cast<u64>((uint8_t)tmpCache[1]) << 8);
		value |= (static_cast<u64>((uint8_t)tmpCache[2]) << 16);
		value |= (static_cast<u64>((uint8_t)tmpCache[3]) << 24);
		u64 sum = (uint8_t)tmpCache[4];
		sum |= (static_cast<u64>((uint8_t)tmpCache[5]) << 8);
		sum |= (static_cast<u64>((uint8_t)tmpCache[6]) << 16);
		sum |= (static_cast<u64>((uint8_t)tmpCache[7]) << 24);
		u64 product = (uint8_t)tmpCache[8];
		product |= (static_cast<u64>((uint8_t)tmpCache[9]) << 8);
		product |= (static_cast<u64>((uint8_t)tmpCache[10]) << 16);
		product |= (static_cast<u64>((uint8_t)tmpCache[11]) << 24);
        // multiply the value by 10 so we get an extra digit out of it, our dimensions become 9 in the process though!
		secondaryDataCache[i] = std::make_tuple(value * fastPow10<9>, sum, product);
	}
	if (!cachedCopy.eof()) {
		std::cerr << "data cache is too small!" << std::endl;
        return false;
	}

	cachedCopy.close();
    return true;
}

int main() {
    if (!loadPrimaryDataCache()) {
        return 1;
    }
    if (!loadSecondaryDataCache()) {
        return 1;
    }
    std::ostringstream storage;
    static constexpr auto next = fastPow10<12 - 1>;
    static constexpr auto doubleNext = next << 1;
    auto originalProduct = 1;
    auto product = 2;
    auto index = doubleNext;
    auto fn = [](auto sum, auto product, auto value) noexcept {
        std::ostringstream str;
        innerMostBody(str, sum, product, value);
        return str.str();
    };
    auto op = [fn](auto sum, auto product, auto value) noexcept {
        return std::async(std::launch::async, fn, sum, product, value);
    };
    auto p0 = op(2, product, index);
    product += originalProduct;
    index += next;
    auto p1 = op(3, product, index);
    product += originalProduct;
    index += next;
    auto p2 = op(4, product, index);
    product += (originalProduct << 1);
    index += doubleNext;
    auto p3 = op(6, product, index);
    product += originalProduct;
    index += next;
    auto p4 = op(7, product, index);
    product += originalProduct;
    index += next;
    auto p5 = op(8, product, index);
    product += originalProduct;
    index += next;
    auto p6 = op(9, product, index);
    storage << p0.get() << p1.get() << p2.get() << p3.get() << p4.get() << p5.get() << p6.get();
    std::cout << storage.str() << std::endl;
    return 0;
}
