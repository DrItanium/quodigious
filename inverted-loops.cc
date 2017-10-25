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
void innerMostBody(std::ostream& stream, u64 sum, u64 product, u64 value) noexcept {
	if (checkValue(sum) && isQuodigious(value, sum, product)) {
		stream << value << std::endl;
	}
}

using container = std::tuple<u64, u64, u64>;

template<u64 pos, u64 max>
inline void loopBody(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept;

template<u64 pos, u64 max>
inline std::string loopBodyString(u64 sum, u64 product, u64 index) noexcept;

template<bool topLevel>
struct ActualLoopBody {
	ActualLoopBody() = delete;
	~ActualLoopBody() = delete;
	ActualLoopBody(ActualLoopBody&&) = delete;
	ActualLoopBody(const ActualLoopBody&) = delete;

	template<u64 pos, u64 max>
	static void body(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept {
        static constexpr auto next = fastPow10<pos - 1>;
        static constexpr auto follow = pos + 1;
        static constexpr auto doubleNext = next << 1;
        auto originalProduct = product;
        product <<= 1;
        sum += 2;
		index += doubleNext;
		loopBody<follow, max>(storage, sum, product, index); // 2
		product += originalProduct;
		++sum;
		index += next;
		loopBody<follow, max>(storage, sum, product, index); // 3
		product += originalProduct;
		++sum;
		index += next;
		loopBody<follow, max>(storage, sum, product, index); // 4
		product += (originalProduct << 1);
		sum += 2;
		index += doubleNext;
		loopBody<follow, max>(storage, sum, product, index); // 6
		product += originalProduct;
		++sum;
		index += next;
		loopBody<follow, max>(storage, sum, product, index); // 7
		product += originalProduct;
		++sum;
		index += next;
		loopBody<follow, max>(storage, sum, product, index); // 8
		product += originalProduct;
		++sum;
		index += next;
		loopBody<follow, max>(storage, sum, product, index); // 9
    }
};

template<>
struct ActualLoopBody<true> {
	ActualLoopBody() = delete;
	~ActualLoopBody() = delete;
	ActualLoopBody(ActualLoopBody&&) = delete;
	ActualLoopBody(const ActualLoopBody&) = delete;
	template<u64 pos, u64 max>
	static void body(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept {
		static_assert(max == pos, "Can't have a top level if the position and max don't match!");
        static constexpr auto next = fastPow10<pos - 1>;
		static constexpr auto doubleNext = next << 1;
        auto originalProduct = product;
        product <<= 1;
        sum += 2;
        index += doubleNext;
        (innerMostBody(storage, sum, product, index)); // 2
		product += originalProduct;
		++sum;
		index += next;
        (innerMostBody(storage, sum, product, index)); // 3
		product += originalProduct;
		++sum;
		index += next;
        (innerMostBody(storage, sum, product, index)); // 4
		product += (originalProduct << 1);
		sum += 2;
		index += doubleNext;
        (innerMostBody(storage, sum, product, index)); // 6
		product += originalProduct;
		++sum;
		index += next;
        (innerMostBody(storage, sum, product, index)); // 7
		product += originalProduct;
		++sum;
		index += next;
        (innerMostBody(storage, sum, product, index)); // 8
		product += originalProduct;
		++sum;
		index += next;
        (innerMostBody(storage, sum, product, index)); // 9


	}
};

template<u64 pos, u64 max>
inline void loopBody(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept {
	static_assert (pos <= max, "Position can't be larger than maximum!");
	// walk through two separate set of actions
	ActualLoopBody<pos == max>::template body< pos, max > (storage, sum, product, index);
}

template<u64 pos, u64 max>
inline std::string loopBodyString(u64 sum, u64 product, u64 index) noexcept {
	std::ostringstream storage;
	loopBody<pos, max> (storage, sum, product, index);
	return storage.str();
}
constexpr auto dimensionCount = 8;
constexpr auto dataCacheSize = numElements<dimensionCount>;
constexpr auto onePart = dataCacheSize / 7;
container dataCache[dataCacheSize];
template<u64 length>
inline void body(std::ostream& storage) noexcept {
    static_assert(length <= 19, "Can't have numbers over 19 digits at this time!");
	auto fn = [](auto start, auto end) { 
		std::ostringstream str;
		for (auto i = start; i < end; ++i) {
			auto result = dataCache[i];
			loopBody<dimensionCount + 1, length>(str, std::get<1>(result), std::get<2>(result), std::get<0>(result));
		}
		return str.str();
	};
	auto p0 = std::async(std::launch::async, fn, onePart * 0, onePart * 1);
	auto p1 = std::async(std::launch::async, fn, onePart * 1, onePart * 2);
	auto p2 = std::async(std::launch::async, fn, onePart * 2, onePart * 3);
	auto p3 = std::async(std::launch::async, fn, onePart * 3, onePart * 4);
	auto p4 = std::async(std::launch::async, fn, onePart * 4, onePart * 5);
	auto p5 = std::async(std::launch::async, fn, onePart * 5, onePart * 6);
	auto p6 = std::async(std::launch::async, fn, onePart * 6, onePart * 7);
	storage << p0.get() << p1.get() << p2.get() << p3.get() << p4.get() << p5.get() << p6.get();
}

int main() {
	std::ifstream cachedCopy("cache.bin", std::ifstream::in | std::ifstream::binary);
	if (!cachedCopy.good()) {
		std::cerr << "ERROR Couldn't open cache.bin data cache! Make sure it exists and is named cache.bin" << std::endl;
		return 1;
	}
	char tmpCache[sizeof(u32) * 3] = { 0 };
	for (int i = 0; i < dataCacheSize || cachedCopy.good(); ++i) {
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
		dataCache[i] = std::make_tuple(value, sum, product);
	}
	if (!cachedCopy.eof()) {
		std::cerr << "data cache is too small!" << std::endl;
		return 1;
	}

	cachedCopy.close();
	
    std::ostringstream storage;
    while(std::cin.good()) {
        u64 currentIndex = 0;
        std::cin >> currentIndex;
        if (std::cin.good()) {
            switch(currentIndex) {
                case 10: body<10>(storage); break;
                case 11: body<11>(storage); break;
                case 12: body<12>(storage); break;
                case 13: body<13>(storage); break;
                case 14: body<14>(storage); break;
                case 15: body<15>(storage); break;
                case 16: body<16>(storage); break;
                case 17: body<17>(storage); break;
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
