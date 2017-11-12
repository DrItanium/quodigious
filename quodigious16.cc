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

constexpr auto digitCount = 16;
template<u64 count>
constexpr auto dataCacheSize = numElements<count>;

constexpr auto dimensionCount = 8;
constexpr auto expectedDimensionCount = dimensionCount + 1;
constexpr auto primaryDataCacheSize = dataCacheSize<dimensionCount>;
container primaryDataCache[primaryDataCacheSize];
constexpr auto secondaryDimensionCount = 5;
constexpr auto secondaryDataCacheSize = dataCacheSize<secondaryDimensionCount>;

container secondaryDataCache[secondaryDataCacheSize];

std::string innerMostBody(u64 sum, u64 product, u64 value) noexcept {
    std::ostringstream stream;
    // the last digit of all numbers is 2, 4, 6, or 8 so ignore the others and compute this right now
	static constexpr auto primaryThreadCount = 14;
	static constexpr auto difference = primaryDataCacheSize % primaryThreadCount;
	static constexpr auto primaryOnePart = (primaryDataCacheSize - difference) / primaryThreadCount;
	auto fn = [sum, product, value](auto start, auto end) noexcept {
    	std::ostringstream stream;
		for (auto i = start; i < end; ++i) {
			auto outer = primaryDataCache[i];
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
		return stream.str();
	};
	using Worker = decltype(std::async(std::launch::async, fn, 0, 1));
	Worker workers[primaryThreadCount];
	
	for (auto a = 0; a < primaryThreadCount; ++a) {
		workers[a] = std::async(std::launch::async, fn, (a * primaryOnePart), ((a + 1) * primaryOnePart));
	}
	// compute the rest on teh primary thread
	auto lastWorker = std::async(std::launch::async, fn, primaryDataCacheSize - difference, primaryDataCacheSize);
	for (auto a = 0; a < primaryThreadCount; ++a) {
		stream << workers[a].get();
	}
	stream << lastWorker.get();
    return stream.str();
}


template<u64 sum, u64 product, u64 value>
decltype(auto) makeWorker() noexcept {
    return std::async(std::launch::async, innerMostBody, sum, product, value);
}

template<u64 outer>
decltype(auto) makeSuperWorker() noexcept {
	return std::async(std::launch::deferred, []() {
			static constexpr auto next = fastPow10<digitCount - 1>;
			static constexpr auto nextNext = fastPow10<digitCount - 2>;
			static constexpr auto outerMost = outer * next;
			std::ostringstream output;
			auto p0 = makeWorker<outer + 2, outer * 2, outerMost + (nextNext * 2)>();
			auto p1 = makeWorker<outer + 3, outer * 3, outerMost + (nextNext * 3)>();
			auto p2 = makeWorker<outer + 4, outer * 4, outerMost + (nextNext * 4)>();
			auto p3 = makeWorker<outer + 6, outer * 6, outerMost + (nextNext * 6)>();
			auto p4 = makeWorker<outer + 7, outer * 7, outerMost + (nextNext * 7)>();
			auto p5 = makeWorker<outer + 8, outer * 8, outerMost + (nextNext * 8)>();
			auto p6 = makeWorker<outer + 9, outer * 9, outerMost + (nextNext * 9)>();
			output << p0.get() << p1.get() << p2.get() << p3.get() << p4.get() << p5.get() << p6.get();
			return output.str();
	});
}

int main(int argc, char* argv[]) {
	if (!loadDataCache<1>("cache.bin", primaryDataCache, primaryDataCacheSize) || !loadDataCache<9>("cache5.bin", secondaryDataCache, secondaryDataCacheSize)) {
        return 1;
    }
	if (argc > 1) {
		switch (argv[1][0]) {
			case '2': std::cout << makeSuperWorker<2>().get(); break;
			case '3': std::cout << makeSuperWorker<3>().get(); break;
			case '4': std::cout << makeSuperWorker<4>().get(); break;
			case '6': std::cout << makeSuperWorker<6>().get(); break;
			case '7': std::cout << makeSuperWorker<7>().get(); break;
			case '8': std::cout << makeSuperWorker<8>().get(); break;
			case '9': std::cout << makeSuperWorker<9>().get(); break;
			default: break;
		}
	} else {
		auto p0 = makeSuperWorker<2>();
		auto p1 = makeSuperWorker<3>();
		auto p2 = makeSuperWorker<4>();
		auto p3 = makeSuperWorker<6>();
		auto p4 = makeSuperWorker<7>();
		auto p5 = makeSuperWorker<8>();
		auto p6 = makeSuperWorker<9>();
		std::cout << p0.get() << p1.get() << p2.get() << p3.get() << p4.get() << p5.get() << p6.get();
	}
    return 0;
}
