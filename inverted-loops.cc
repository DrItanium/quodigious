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
container dataCache[2097152];
template<u64 length>
inline void body(std::ostream& storage) noexcept {
    static_assert(length <= 19, "Can't have numbers over 19 digits at this time!");
	for (int i = 0; i < 2097152; ++i) {
		auto result = dataCache[i];
		loopBody<8, length>(storage, std::get<1>(result), std::get<2>(result), std::get<0>(result));
	}
}

int main() {
	std::ifstream cachedCopy("nums7", std::ifstream::in | std::ifstream::binary);
	if (!cachedCopy.good()) {
		std::cerr << "ERROR Couldn't open nums7 data cache! Make sure it exists and is named nums7" << std::endl;
		return 1;
	}
	for (int i = 0; i < 2097152 || cachedCopy.good(); ++i) {
		// layout is value, sum, product
		u64 value = cachedCopy.get();
		value |= (static_cast<u64>(cachedCopy.get()) << 8);
		value |= (static_cast<u64>(cachedCopy.get()) << 16);
		value |= (static_cast<u64>(cachedCopy.get()) << 24);
		u64 sum = cachedCopy.get();
		sum |= (static_cast<u64>(cachedCopy.get()) << 8);
		sum |= (static_cast<u64>(cachedCopy.get()) << 16);
		sum |= (static_cast<u64>(cachedCopy.get()) << 24);
		u64 product = cachedCopy.get();
		product |= (static_cast<u64>(cachedCopy.get()) << 8);
		product |= (static_cast<u64>(cachedCopy.get()) << 16);
		product |= (static_cast<u64>(cachedCopy.get()) << 24);
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
