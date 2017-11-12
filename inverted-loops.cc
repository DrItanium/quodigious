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

void innerMostBody(std::ostream& stream, u64 sum, u64 product, u64 value) noexcept;

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
        auto fn = [](auto sum, auto product, auto value) noexcept {
            std::ostringstream str;
            innerMostBody(str, sum, product, value);
            return str.str();
        };
        auto op = [fn](auto sum, auto product, auto value) noexcept {
            return std::async(std::launch::async, fn, sum, product, value);
        };
        auto p0 = op(sum, product, index);
		product += originalProduct;
		++sum;
		index += next;
        auto p1 = op(sum, product, index);
		product += originalProduct;
		++sum;
		index += next;
        auto p2 = op(sum, product, index);
		product += (originalProduct << 1);
		sum += 2;
		index += doubleNext;
        auto p3 = op(sum, product, index);
		product += originalProduct;
		++sum;
		index += next;
        auto p4 = op(sum, product, index);
		product += originalProduct;
		++sum;
		index += next;
        auto p5 = op(sum, product, index);
		product += originalProduct;
		++sum;
		index += next;
        auto p6 = op(sum, product, index);
        storage << p0.get() << p1.get() << p2.get() << p3.get() << p4.get() << p5.get() << p6.get();
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
constexpr auto expectedDimensionCount = dimensionCount + 1;
constexpr auto dataCacheSize = numElements<dimensionCount>;
container dataCache[dataCacheSize];
template<u64 length>
inline void body(std::ostream& storage) noexcept {
    static_assert(length <= 19, "Can't have numbers over 19 digits at this time!");
    loopBody<expectedDimensionCount + 1, length>(storage, 0, 1, 0);
}

void innerMostBody(std::ostream& stream, u64 sum, u64 product, u64 value) noexcept {
    // the last digit of all numbers is 2, 4, 6, or 8 so ignore the others and compute this right now
#ifndef SINGLE_THREAD_INNER
	auto fn = [sum, product, value](auto start, auto end) {
		std::ostringstream str;
		for (auto i = start; i < end; ++i) {
			auto result = dataCache[i];
            auto v = std::get<0>(result) + value;
            auto s = std::get<1>(result) + sum;
            auto p = std::get<2>(result) * product;
            merge(inspectValue(v + 2, s + 2, p * 2), str);
            merge(inspectValue(v + 4, s + 4, p * 4), str);
            merge(inspectValue(v + 6, s + 6, p * 6), str);
            merge(inspectValue(v + 8, s + 8, p * 8), str);
		}
		return str.str();
	};
	using AsyncWorker = decltype(std::async(std::launch::async, fn, 0, 1));
    constexpr auto numParts = 7;
    constexpr auto onePart = dataCacheSize / numParts;
	AsyncWorker pool[numParts];
	for (int i = 0; i < numParts; ++i) {
		pool[i] = std::async(std::launch::async, fn, onePart * i, onePart * (i + 1));
	}
	for (int i = 0; i < numParts; ++i) {
		stream << pool[i].get();
	}
#else
    for (const auto& result : dataCache) {
        auto v = std::get<0>(result);
        auto s = std::get<1>(result);
        auto p = std::get<2>(result);
        merge(inspectValue(v + value + 2, s + sum + 2, p * product * 2), stream);
        merge(inspectValue(v + value + 4, s + sum + 4, p * product * 4), stream);
        merge(inspectValue(v + value + 6, s + sum + 6, p * product * 6), stream);
        merge(inspectValue(v + value + 8, s + sum + 8, p * product * 8), stream);
    }
#endif
}

int main() {
	if (!loadDataCache("cache.bin", dataCache, dataCacheSize)) {
		return 1;
	}
    std::ostringstream storage;
    while(std::cin.good()) {
        u64 currentIndex = 0;
        std::cin >> currentIndex;
        if (std::cin.good()) {
            switch(currentIndex) {
                case 10: body<10>(storage); break;
                case 11: body<11>(storage); break;
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
