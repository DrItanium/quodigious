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

#ifndef QLIB_H__
#define QLIB_H__
#include <cstdint>
#include <iostream>
#include <tuple>
#include <fstream>
#include <cmath>
#include <sstream>
#include <functional>
#include <future>
#include <map>
using u64 = uint64_t;
using u32 = uint32_t;

template<u64 length>
constexpr u64 quickPow10() noexcept {
	return quickPow10<length - 1>() * 10;
}
template<>
constexpr u64 quickPow10<0>() noexcept {
	return 1;
}

template<u64 length>
constexpr auto fastPow10 = quickPow10<length>();


template<typename T>
constexpr bool componentQuodigious(T value, T compare) noexcept {
	return (value % compare) == 0;
}
/**
 * This is used all over the place, it is the actual code to check to see if a
 * number is actuall quodigious. All of the work before hand is to get the sum
 * and products (if we get this far).
 */
template<typename T>
constexpr bool isQuodigious(T value, T sum, T product) noexcept {
	// more often than not, the sum is divisible by the original value, so
	// really that sort of check is useless. If we find that the product is
	// divisible first then we should eliminate numbers faster :D
	return componentQuodigious<T>(value, product) && componentQuodigious<T>(value, sum);
}


template<u64 times>
constexpr u64 multiply(u64 product) noexcept {
	return times * product;
}

template<u32 times>
constexpr u32 multiply(u32 product) noexcept {
	return times * product;
}

template<> constexpr u64 multiply<0>(u64 product) noexcept { return 0; }
template<> constexpr u64 multiply<1>(u64 product) noexcept { return product; }
template<> constexpr u64 multiply<2>(u64 product) noexcept { return product << 1; }
template<> constexpr u64 multiply<3>(u64 product) noexcept { return (product << 1) + product; }
template<> constexpr u64 multiply<4>(u64 product) noexcept { return (product << 2); }
template<> constexpr u64 multiply<5>(u64 product) noexcept { return (product << 2) + product; }
template<> constexpr u64 multiply<6>(u64 product) noexcept { return (product << 2) + (product << 1); }
template<> constexpr u64 multiply<7>(u64 product) noexcept { return (product << 2) + (product << 1) + product; }
template<> constexpr u64 multiply<8>(u64 product) noexcept { return (product << 3); }
template<> constexpr u64 multiply<9>(u64 product) noexcept { return (product << 3) + product; }
template<> constexpr u64 multiply<10>(u64 product) noexcept { return (product << 3) + (product << 1); }

template<> constexpr u32 multiply<0>(u32 product) noexcept { return 0; }
template<> constexpr u32 multiply<1>(u32 product) noexcept { return product; }
template<> constexpr u32 multiply<2>(u32 product) noexcept { return product << 1; }
template<> constexpr u32 multiply<3>(u32 product) noexcept { return (product << 1) + product; }
template<> constexpr u32 multiply<4>(u32 product) noexcept { return (product << 2); }
template<> constexpr u32 multiply<5>(u32 product) noexcept { return (product << 2) + product; }
template<> constexpr u32 multiply<6>(u32 product) noexcept { return (product << 2) + (product << 1); }
template<> constexpr u32 multiply<7>(u32 product) noexcept { return (product << 2) + (product << 1) + product; }
template<> constexpr u32 multiply<8>(u32 product) noexcept { return (product << 3); }
template<> constexpr u32 multiply<9>(u32 product) noexcept { return (product << 3) + product; }
template<> constexpr u32 multiply<10>(u32 product) noexcept { return (product << 3) + (product << 1); }

constexpr bool isEven(u64 value) noexcept {
	return (value & 1) == 0;
}

template<u64 k>
struct EvenCheck : std::integral_constant<bool, k == ((k >> 1) << 1)> { };

template<typename T>
inline void merge(T value , std::ostream& input) noexcept {
	if (value != 0) {
		input << value << std::endl;
	}
}

template<u64 width>
constexpr int numberOfDigitsForGivenWidth() noexcept {
	static_assert(width >= 0, "Negative width doesn't make sense");
	return 7 * numberOfDigitsForGivenWidth<width - 1>();
}
template<> constexpr int numberOfDigitsForGivenWidth<0>() noexcept { return 1; }
template<u64 width>
constexpr auto numElements = numberOfDigitsForGivenWidth<width>();

template<u64 width>
constexpr u64 makeDigitAt(u64 input) noexcept {
	return input * fastPow10<width>;
}

template<u64 width, u64 divide>
constexpr int getPartialSize() noexcept {
	static_assert(divide > 0, "Can't have a divisor of 0");
	return numElements<width> / divide;
}

template<u64 width, u64 divisible>
constexpr bool isDivisibleBy(u64 factor) noexcept {
	return (factor * divisible) == numElements<width>;
}

constexpr u64 makeU64(char a, char b, char c, char d) noexcept {
	u64 value = static_cast<uint8_t>(a);
	value |= static_cast<u64>(static_cast<uint8_t>(b) << 8);
	value |= static_cast<u64>(static_cast<uint8_t>(c) << 16);
	value |= static_cast<u64>(static_cast<uint8_t>(d) << 24);
	return value;
}


using container = std::tuple<u64, u64, u64>;
bool loadDataCache(const std::string& fileName, container* collection, size_t size);

constexpr bool checkValue(u64 sum) noexcept {
	return (isEven(sum)) || (sum % 3 == 0);
}
constexpr u64 inspectValue(u64 value, u64 sum, u64 product) noexcept {
	if (checkValue(sum) && isQuodigious(value, sum, product)) {
		return value;
	}
	return 0;
}

template<u64 factor>
bool loadDataCache(const std::string& fileName, container* collection, size_t size) {
	std::ifstream cachedCopy(fileName, std::ifstream::in | std::ifstream::binary);
	if (!cachedCopy.good()) {
		std::cerr << "ERROR: Couldn't open " << fileName << " data cache file! Make sure it exists and is named " << fileName << std::endl;
		return false;
	}
	// TODO: update the binary generator to eliminate the last digit from the
	// computation, then we can do four numbers at a time in the inner most
	// loop of this code which should make up the computation difference we've
	// seen so far!
	//
	// We can also reintroduce nested threading, the difference is that we can
	// just eliminate the values needed
	constexpr auto readSize = sizeof(u32) * 3;
	char tmpCache[readSize] = { 0 };
	for (int i = 0; i < size || cachedCopy.good(); ++i) {
		// layout is value, sum, product
		cachedCopy.read(tmpCache, readSize);
		u64 value = makeU64(tmpCache[0], tmpCache[1], tmpCache[2], tmpCache[3]);
		u64 sum = makeU64(tmpCache[4], tmpCache[5], tmpCache[6], tmpCache[7]);
		u64 product = makeU64(tmpCache[8], tmpCache[9], tmpCache[10], tmpCache[11]);
		// multiply the value by 10 so we get an extra digit out of it, our dimensions become 9 in the process though!
		collection[i] = std::make_tuple(value * fastPow10<factor>, sum, product);
	}
	if (!cachedCopy.eof()) {
		std::cerr << "data cache is too small!" << std::endl;
		return false;
	}

	cachedCopy.close();
	return true;
}

template<u64 count>
constexpr auto dataCacheSize = numElements<count>;

template<u64 width, u64 primaryDataCacheSize, u64 secondaryDataCacheSize, u64 threadCount = 7>
std::string typicalInnerMostBody(u64 sum, u64 product, u64 value, container* primaryDataCache, container* secondaryDataCache) noexcept {
	std::ostringstream stream;
	// the last digit of all numbers is 2, 4, 6, or 8 so ignore the others and compute this right now
	static constexpr auto difference = primaryDataCacheSize % threadCount;
	static constexpr auto primaryOnePart = (primaryDataCacheSize - difference) / threadCount; 
	auto fn = [sum, product, value, primaryDataCache, secondaryDataCache](auto start, auto end) noexcept {
		std::ostringstream stream;
		for (auto i = start; i < end; ++i) {
			auto outer = primaryDataCache[i];
			u64 ov, os, op;
			std::tie(ov, os, op) = outer;
			ov += value;
			os += sum;
			op *= product;
			for (auto j = 0; j < secondaryDataCacheSize; ++j)  {
				auto inner = secondaryDataCache[j];
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
	Worker workers[threadCount];

	for (auto a = 0; a < threadCount; ++a) {
		workers[a] = std::async(std::launch::async, fn, (a * primaryOnePart), ((a + 1) * primaryOnePart));
	}
	// compute the rest on teh primary thread
	auto lastWorker = std::async(std::launch::async, fn, primaryDataCacheSize - difference, primaryDataCacheSize);
	for (auto a = 0; a < threadCount; ++a) {
		stream << workers[a].get();
	}
	stream << lastWorker.get();
	return stream.str();
}

template<u64 sum, u64 product, u64 value, u64 width, u64 primaryCacheWidth, u64 secondaryCacheWidth, u64 threadCount = 7>
decltype(auto) makeWorker(container* primary, container* secondary, decltype(std::launch::async) policy = std::launch::async) noexcept {
	return std::async(policy, typicalInnerMostBody<width, dataCacheSize<primaryCacheWidth>, dataCacheSize<secondaryCacheWidth>, threadCount>, sum, product, value, primary, secondary);
}

template<u64 outer, u64 digitWidth, u64 primaryDataCacheSize, u64 secondaryDataCacheSize, u64 threadCount = 7>
decltype(auto) makeSuperWorker(container* primary, container* secondary, decltype(std::launch::deferred) policy = std::launch::deferred) noexcept {
	return std::async(policy, [primary, secondary]() {
			static constexpr auto next = fastPow10<digitWidth - 1>;
			static constexpr auto nextNext = fastPow10<digitWidth - 2>;
			static constexpr auto outerMost = outer * next;
			std::ostringstream output;
			auto p0 = makeWorker<outer + 2, outer * 2, outerMost + (nextNext * 2), digitWidth, primaryDataCacheSize, secondaryDataCacheSize, threadCount>(primary, secondary);
			auto p1 = makeWorker<outer + 3, outer * 3, outerMost + (nextNext * 3), digitWidth, primaryDataCacheSize, secondaryDataCacheSize, threadCount>(primary, secondary);
			auto p2 = makeWorker<outer + 4, outer * 4, outerMost + (nextNext * 4), digitWidth, primaryDataCacheSize, secondaryDataCacheSize, threadCount>(primary, secondary);
			auto p3 = makeWorker<outer + 6, outer * 6, outerMost + (nextNext * 6), digitWidth, primaryDataCacheSize, secondaryDataCacheSize, threadCount>(primary, secondary);
			auto p4 = makeWorker<outer + 7, outer * 7, outerMost + (nextNext * 7), digitWidth, primaryDataCacheSize, secondaryDataCacheSize, threadCount>(primary, secondary);
			auto p5 = makeWorker<outer + 8, outer * 8, outerMost + (nextNext * 8), digitWidth, primaryDataCacheSize, secondaryDataCacheSize, threadCount>(primary, secondary);
			auto p6 = makeWorker<outer + 9, outer * 9, outerMost + (nextNext * 9), digitWidth, primaryDataCacheSize, secondaryDataCacheSize, threadCount>(primary, secondary);
			output << p0.get() << p1.get() << p2.get() << p3.get() << p4.get() << p5.get() << p6.get();
			return output.str();
			});
}

template<u64 digitWidth, u64 primaryDimensions, u64 secondaryDimensions, u64 threadCount>
std::string nonSuperComputation(container* cache0, container* cache1) {
	std::ostringstream stream;
	auto p0 = makeWorker<2, 2, (2 * fastPow10<digitWidth - 1>), digitWidth, primaryDimensions, secondaryDimensions, threadCount>(cache0, cache1); // 2
	auto p1 = makeWorker<3, 3, (3 * fastPow10<digitWidth - 1>), digitWidth, primaryDimensions, secondaryDimensions, threadCount>(cache0, cache1); // 3
	auto p2 = makeWorker<4, 4, (4 * fastPow10<digitWidth - 1>), digitWidth, primaryDimensions, secondaryDimensions, threadCount>(cache0, cache1); // 4
	auto p3 = makeWorker<6, 6, (6 * fastPow10<digitWidth - 1>), digitWidth, primaryDimensions, secondaryDimensions, threadCount>(cache0, cache1); // 6
	auto p4 = makeWorker<7, 7, (7 * fastPow10<digitWidth - 1>), digitWidth, primaryDimensions, secondaryDimensions, threadCount>(cache0, cache1); // 7
	auto p5 = makeWorker<8, 8, (8 * fastPow10<digitWidth - 1>), digitWidth, primaryDimensions, secondaryDimensions, threadCount>(cache0, cache1); // 8
	auto p6 = makeWorker<9, 9, (9 * fastPow10<digitWidth - 1>), digitWidth, primaryDimensions, secondaryDimensions, threadCount>(cache0, cache1); // 9
	stream << p0.get() << p1.get() << p2.get() << p3.get() << p4.get() << p5.get() << p6.get();
	auto s = stream.str();
	return s;
}
template<u64 digitCount, u64 dim0, u64 dim1, u64 threadCount>
std::string oneSeventhSuperComputation(char symbol, container* cache0, container* cache1) {
	std::ostringstream stream;
	switch (symbol) {
		case '2': stream << makeSuperWorker<2, digitCount, dim0, dim1, threadCount>(cache0, cache1).get(); break;
		case '3': stream << makeSuperWorker<3, digitCount, dim0, dim1, threadCount>(cache0, cache1).get(); break;
		case '4': stream << makeSuperWorker<4, digitCount, dim0, dim1, threadCount>(cache0, cache1).get(); break;
		case '6': stream << makeSuperWorker<6, digitCount, dim0, dim1, threadCount>(cache0, cache1).get(); break;
		case '7': stream << makeSuperWorker<7, digitCount, dim0, dim1, threadCount>(cache0, cache1).get(); break;
		case '8': stream << makeSuperWorker<8, digitCount, dim0, dim1, threadCount>(cache0, cache1).get(); break;
		case '9': stream << makeSuperWorker<9, digitCount, dim0, dim1, threadCount>(cache0, cache1).get(); break;
		default: break;
	}
	auto s = stream.str();
	return s;
}
#endif // end QLIB_H__
