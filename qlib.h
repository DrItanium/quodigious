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
using byte = uint8_t;
using u64 = uint64_t;
using u32 = uint32_t;
using LaunchPolicy = decltype(std::launch::async);

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

constexpr u32 makeU32(char a, char b, char c, char d) noexcept {
	u32 value = static_cast<uint8_t>(a);
	value |= static_cast<u32>(static_cast<uint8_t>(b) << 8);
	value |= static_cast<u32>(static_cast<uint8_t>(c) << 16);
	value |= static_cast<u32>(static_cast<uint8_t>(d) << 24);
	return value;
}


struct container {
	// these containers are meant to be fixed based on the data at hand from
	// cache import, at most we are going to do 10 digit precomputed (that is a
	// lot of storage!), only the value has to be 64-bits wide
	u64 value;
	u32 product;
	uint8_t sum;
};

constexpr bool checkValue(u64 sum) noexcept {
	return (isEven(sum)) || (sum % 3 == 0);
}
constexpr u64 inspectValue(u64 value, u64 sum, u64 product) noexcept {
	if (checkValue(sum) && isQuodigious(value, sum, product)) {
		return value;
	}
	return 0;
}

inline bool loadDataCache(const std::string& fileName, container* collection, size_t size, u64 offset) noexcept {
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
	constexpr auto readSize = (sizeof(u32) * 2) + sizeof(uint8_t);
	char tmpCache[readSize] = { 0 };
	int i = 0;
	for (i = 0; i < size && cachedCopy.good(); ++i) {
		// layout is value, sum, product
		cachedCopy.read(tmpCache, readSize);
		container temp;
		temp.value = makeU64(tmpCache[0], tmpCache[1], tmpCache[2], tmpCache[3]) * offset;
		temp.product = makeU32(tmpCache[4], tmpCache[5], tmpCache[6], tmpCache[7]);
		temp.sum = uint8_t(tmpCache[8]);
		if (temp.sum >= 255) {
			std::cerr << "Found a sum greater than 255!" << std::endl;
			return false;
		}
		collection[i] = temp;
	}
	if (i < size && !cachedCopy.eof()) {
		std::cerr << "data cache is too small!" << std::endl;
		return false;
	}

	cachedCopy.close();
	return true;
}

template<u64 factor>
bool loadDataCache(const std::string& fileName, container* collection, size_t size) noexcept {
	return loadDataCache(fileName, collection, size, fastPow10<factor>);
}

template<u64 count>
constexpr auto dataCacheSize = numElements<count>;
template<u64 index>
inline void performFinalCompute(u64 value, u64 sum, u64 product, std::ostream& stream) noexcept {
	merge(inspectValue(value + index, sum + index, product * index), stream);
}
template<u64 index>
inline void performFinalCompute(u64 sum, u64 product, u64 value, container& inner, std::ostream& stream) noexcept {
	performFinalCompute<index>(inner.value + value, inner.sum + sum, inner.product * product, stream);
}
template<byte mask = 0b1111>
inline void finalCompute4(u64 sum, u64 product, u64 value, container& inner, std::ostream& stream) noexcept {
	if ((mask & 0x1) != 0) {
		performFinalCompute<2>(sum, product, value, inner, stream);
	}
	if ((mask & 0x2) != 0) {
		performFinalCompute<4>(sum, product, value, inner, stream);
	}
	if ((mask & 0x4) != 0) {
		performFinalCompute<6>(sum, product, value, inner, stream);
	}
	if ((mask & 0x8) != 0) {
		performFinalCompute<8>(sum, product, value, inner, stream);
	}
}

template<u64 secondarySize, byte mask = 0b1111>
std::string innerMostThreadBody(u64 sum, u64 product, u64 value, container* primary, container* secondary, u64 start, u64 end) noexcept {
	std::ostringstream stream;
	for (auto i = start; i < end; i+= 3) {
		// NOTE: this comment assumes that we are reading three outer numbers
		// at a time. If the number above has changed to a large number (say
		// 4,5,6,7,etc) then understand that the logic is the same just on a
		// larger scale
		//
		//
		// not every range is going to be evenly divisible by three so instead
		// do some logic walking through to figure out how many outer numbers
		// can be safely interleaved into an inner loop. So if we had an outer
		// number count of 8 digits with inner loop of 2 digits or so then we
		// encounter roughly 7^8 numbers in the outer and 49 inner numbers so
		// this is the equivalent of doing 49 operations per outer number in
		// the old model. This primarly relates to accessing the inner object
		// more than anything else. Thus for every three outer numbers we do
		// only  49 memory accesses instead of 147! However, that is assuming
		// that the entire contents of the structure will fit inside a register
		// (which it will not a 64-bit machine). Instead we have to use either
		// two registers or do memory access offsets. Assuming the later, we
		// actually save even more. For each value if we assume that each
		// memory access is separate we actually have:
		//   1 (initial memory access) from the array
		//   3 (to access each element in the structure separately)
		// this yields 4 memory accesses, if we assume that the compiler
		// optmizes loading the sum and product into a single load then we
		// have:
		//
		//   1 (initial memory access) from the array
		//   2 (one to access the value, the other to load sum and product)
		// this yields 3 memory accesses per inner loop cycle.
		//
		// With 4 accesses per loop cycle we get 196 memory accesses total
		// With 3 accesses per loop cycle we get 147 memory accesses total
		//
		// With the interleaved version below we will save in the general case
		// 588 to 441 memory accesses per complete loop cycle (depending on
		// optimization) with the special cases being a reduced number.
		// However, in all cases we do reduce the amount of data being loaded
		// from main memory and (hopefully) keep more data in the cache :D

		auto outer0 = primary[i];
		u64 ov0 = outer0.value + value;
		u64 os0 = outer0.sum + sum;
		u64 op0 = outer0.product * product;
		if ((i + 1) >= end) {
			for (auto j = 0; j < secondarySize ; ++j)  {
				auto inner = secondary[j];
				finalCompute4<mask>(os0, op0, ov0, inner, stream);
			}
		} else {
			auto outer1 = primary[i + 1];
			u64 ov1 = outer1.value + value;
			u64 os1 = outer1.sum + sum;
			u64 op1 = outer1.product * product;
			if ((i + 2) >= end) {
				for (auto j = 0; j < secondarySize ; ++j)  {
					auto inner = secondary[j];
					finalCompute4<mask>(os0, op0, ov0, inner, stream);
					finalCompute4<mask>(os1, op1, ov1, inner, stream);
				}
			} else {
				auto outer2 = primary[i + 2];
				u64 ov2 = outer2.value + value;
				u64 os2 = outer2.sum + sum;
				u64 op2 = outer2.product * product;
				for (auto j = 0; j < secondarySize ; ++j)  {
					auto inner = secondary[j];
					finalCompute4<mask>(os0, op0, ov0, inner, stream);
					finalCompute4<mask>(os1, op1, ov1, inner, stream);
					finalCompute4<mask>(os2, op2, ov2, inner, stream);
				}
			}
		}
	}
	return stream.str();
}

template<u64 width, u64 primaryDataCacheSize, u64 secondaryDataCacheSize, u64 threadCount = 7, byte mask = 0b1111>
std::string typicalInnerMostBody(u64 sum, u64 product, u64 value, container* primaryDataCache, container* secondaryDataCache) noexcept {
	std::ostringstream stream;
    if (mask != 0) {
	    // the last digit of all numbers is 2, 4, 6, or 8 so ignore the others and compute this right now
	    static constexpr auto difference = primaryDataCacheSize % threadCount;
	    static constexpr auto primaryOnePart = (primaryDataCacheSize - difference) / threadCount;
	    using Worker = decltype(std::async(LaunchPolicy::async, innerMostThreadBody<secondaryDataCacheSize, mask>, 0, 1, 0, nullptr, nullptr, 0, 1));
	    Worker workers[threadCount];

	    for (auto a = 0; a < threadCount; ++a) {
	    	workers[a] = std::async(LaunchPolicy::async, innerMostThreadBody<secondaryDataCacheSize, mask>, sum, product, value, primaryDataCache, secondaryDataCache, (a * primaryOnePart), ((a + 1) * primaryOnePart));
	    }
	    // compute the rest on teh primary thread
	    if (difference > 0) {
	    	auto lastWorker = std::async(LaunchPolicy::async, innerMostThreadBody<secondaryDataCacheSize, mask>, sum, product, value, primaryDataCache, secondaryDataCache, primaryDataCacheSize - difference, primaryDataCacheSize);
	    	stream << lastWorker.get();
	    }
	    for (auto a = 0; a < threadCount; ++a) {
	    	stream << workers[a].get();
	    }
    }
	return stream.str();
}

template<u64 sum, u64 product, u64 value, u64 width, u64 primaryCacheWidth, u64 secondaryCacheWidth, u64 threadCount = 7, byte mask = 0b1111>
decltype(auto) makeWorker(container* primary, container* secondary, LaunchPolicy policy = LaunchPolicy::async) noexcept {
	return std::async(policy, typicalInnerMostBody<width, dataCacheSize<primaryCacheWidth>, dataCacheSize<secondaryCacheWidth>, threadCount, mask>, sum, product, value, primary, secondary);
}

template<u64 index>
constexpr byte getBitmask() noexcept {
    switch(index) {
        case 2: return 0x1;
        case 3: return 0x2;
        case 4: return 0x4;
        case 6: return 0x8;
        case 7: return 0x10;
        case 8: return 0x20;
        case 9: return 0x40;
        default: return 0;
    }
}
template<u64 outer, u64 index, u64 digitWidth, u64 primaryCacheWidth, u64 secondaryCacheWidth, u64 threadCount = 7, byte mask = 0b1111>
inline decltype(auto) buildWorker(int invokeMask, container* primary, container* secondary, LaunchPolicy policy = LaunchPolicy::async) noexcept {
    constexpr auto next = fastPow10<digitWidth - 1>;
    constexpr auto nextNext = fastPow10<digitWidth - 2>;
    constexpr auto outerMost = outer * next;
    if ((invokeMask & getBitmask<index>()) != 0) {
        return makeWorker<outer + index, multiply<index>(outer), outerMost + (multiply<index>(nextNext)), digitWidth, primaryCacheWidth, secondaryCacheWidth, threadCount, mask>(primary, secondary, policy);
    } else {
        return makeWorker<outer + index, multiply<index>(outer), outerMost + (multiply<index>(nextNext)), digitWidth, primaryCacheWidth, secondaryCacheWidth, threadCount, 0>(primary, secondary, policy);
    }
}

template<u64 outer, u64 digitWidth, u64 primaryDataCacheSize, u64 secondaryDataCacheSize, u64 threadCount = 7, byte mask = 0b1111>
inline decltype(auto) makeSuperWorker(int subMask, container* primary, container* secondary, LaunchPolicy policy = LaunchPolicy::deferred) noexcept {
	return std::async(policy, [subMask, primary, secondary]() {
			std::ostringstream output;
            if (mask != 0) {
                auto p0 = buildWorker<outer, 2, digitWidth, primaryDataCacheSize, secondaryDataCacheSize, threadCount, mask>(subMask, primary, secondary);
                auto p1 = buildWorker<outer, 3, digitWidth, primaryDataCacheSize, secondaryDataCacheSize, threadCount, mask>(subMask, primary, secondary);
                auto p2 = buildWorker<outer, 4, digitWidth, primaryDataCacheSize, secondaryDataCacheSize, threadCount, mask>(subMask, primary, secondary);
                auto p3 = buildWorker<outer, 6, digitWidth, primaryDataCacheSize, secondaryDataCacheSize, threadCount, mask>(subMask, primary, secondary);
                auto p4 = buildWorker<outer, 7, digitWidth, primaryDataCacheSize, secondaryDataCacheSize, threadCount, mask>(subMask, primary, secondary);
                auto p5 = buildWorker<outer, 8, digitWidth, primaryDataCacheSize, secondaryDataCacheSize, threadCount, mask>(subMask, primary, secondary);
                auto p6 = buildWorker<outer, 9, digitWidth, primaryDataCacheSize, secondaryDataCacheSize, threadCount, mask>(subMask, primary, secondary);
			    output << p0.get() << p1.get() << p2.get() << p3.get() << p4.get() << p5.get() << p6.get();
            }
			return output.str();
			});
}
template<u64 index, u64 digitWidth, u64 primarySize, u64 secondarySize, u64 threadCount = 7, byte mask = 0b1111>
inline decltype(auto) buildSuperWorker(int checkMask, int secondaryMask, container* primary, container* secondary, LaunchPolicy policy = LaunchPolicy::deferred) noexcept {
    if ((checkMask & getBitmask<index>()) != 0) {
        return makeSuperWorker<index, digitWidth, primarySize, secondarySize, threadCount, mask>(secondaryMask, primary, secondary, policy);
    } else {
        return makeSuperWorker<index, digitWidth, primarySize, secondarySize, threadCount, 0>(secondaryMask, primary, secondary, policy);
    }
}

template<u64 digitWidth, u64 primarySize, u64 secondarySize, u64 threadCount = 7, byte mask = 0b1111>
inline void useSuperWorkers(std::ostream& stream, int primaryDigitsMask, int secondaryMask, container* primary, container* secondary, LaunchPolicy policy = LaunchPolicy::deferred) noexcept {
    if (mask != 0) {
        auto p0 = buildSuperWorker<2, digitWidth, primarySize, secondarySize, threadCount, mask>(primaryDigitsMask, secondaryMask, primary, secondary, policy);
        auto p1 = buildSuperWorker<3, digitWidth, primarySize, secondarySize, threadCount, mask>(primaryDigitsMask, secondaryMask, primary, secondary, policy);
        auto p2 = buildSuperWorker<4, digitWidth, primarySize, secondarySize, threadCount, mask>(primaryDigitsMask, secondaryMask, primary, secondary, policy);
        auto p3 = buildSuperWorker<6, digitWidth, primarySize, secondarySize, threadCount, mask>(primaryDigitsMask, secondaryMask, primary, secondary, policy);
        auto p4 = buildSuperWorker<7, digitWidth, primarySize, secondarySize, threadCount, mask>(primaryDigitsMask, secondaryMask, primary, secondary, policy);
        auto p5 = buildSuperWorker<8, digitWidth, primarySize, secondarySize, threadCount, mask>(primaryDigitsMask, secondaryMask, primary, secondary, policy);
        auto p6 = buildSuperWorker<9, digitWidth, primarySize, secondarySize, threadCount, mask>(primaryDigitsMask, secondaryMask, primary, secondary, policy);
	    stream << p0.get() << p1.get() << p2.get() << p3.get() << p4.get() << p5.get() << p6.get();
    }
}
template<u64 digitWidth, u64 primarySize, u64 secondarySize, u64 threadCount = 7>
inline void doMaskedSuperWorker(int input, int digitsMask, int secondaryMask, std::ostream& stream, container* primary, container* secondary, LaunchPolicy policy = LaunchPolicy::deferred) noexcept {
	switch (input) {
		case 0x0: break;
		case 0x1: useSuperWorkers<digitWidth, primarySize, secondarySize, threadCount, 0b0001>(stream, digitsMask, secondaryMask, primary, secondary, policy); break;
		case 0x2: useSuperWorkers<digitWidth, primarySize, secondarySize, threadCount, 0b0010>(stream, digitsMask, secondaryMask, primary, secondary, policy); break;
		case 0x3: useSuperWorkers<digitWidth, primarySize, secondarySize, threadCount, 0b0011>(stream, digitsMask, secondaryMask, primary, secondary, policy); break;
		case 0x4: useSuperWorkers<digitWidth, primarySize, secondarySize, threadCount, 0b0100>(stream, digitsMask, secondaryMask, primary, secondary, policy); break;
		case 0x5: useSuperWorkers<digitWidth, primarySize, secondarySize, threadCount, 0b0101>(stream, digitsMask, secondaryMask, primary, secondary, policy); break;
		case 0x6: useSuperWorkers<digitWidth, primarySize, secondarySize, threadCount, 0b0110>(stream, digitsMask, secondaryMask, primary, secondary, policy); break;
		case 0x7: useSuperWorkers<digitWidth, primarySize, secondarySize, threadCount, 0b0111>(stream, digitsMask, secondaryMask, primary, secondary, policy); break;
		case 0x8: useSuperWorkers<digitWidth, primarySize, secondarySize, threadCount, 0b1000>(stream, digitsMask, secondaryMask, primary, secondary, policy); break;
		case 0x9: useSuperWorkers<digitWidth, primarySize, secondarySize, threadCount, 0b1001>(stream, digitsMask, secondaryMask, primary, secondary, policy); break;
		case 0xA: useSuperWorkers<digitWidth, primarySize, secondarySize, threadCount, 0b1010>(stream, digitsMask, secondaryMask, primary, secondary, policy); break;
		case 0xB: useSuperWorkers<digitWidth, primarySize, secondarySize, threadCount, 0b1011>(stream, digitsMask, secondaryMask, primary, secondary, policy); break;
		case 0xC: useSuperWorkers<digitWidth, primarySize, secondarySize, threadCount, 0b1100>(stream, digitsMask, secondaryMask, primary, secondary, policy); break;
		case 0xD: useSuperWorkers<digitWidth, primarySize, secondarySize, threadCount, 0b1101>(stream, digitsMask, secondaryMask, primary, secondary, policy); break;
		case 0xE: useSuperWorkers<digitWidth, primarySize, secondarySize, threadCount, 0b1110>(stream, digitsMask, secondaryMask, primary, secondary, policy); break;
		case 0xF:
		default:  useSuperWorkers<digitWidth, primarySize, secondarySize, threadCount, 0b1111>(stream, digitsMask, secondaryMask, primary, secondary, policy); break;
	}
}
template<u64 digitWidth, u64 primarySize, u64 secondarySize, u64 threadCount = 7>
inline void doMaskedSuperWorker(int input, int digitsMask, std::ostream& stream, container* primary, container* secondary, LaunchPolicy policy = LaunchPolicy::deferred) noexcept {
	doMaskedSuperWorker<digitWidth, primarySize, secondarySize, threadCount>(input, digitsMask, 0x7f, stream, primary, secondary, policy);
}

template<u64 digitWidth, u64 primarySize, u64 secondarySize, u64 threadCount = 7>
inline void doMaskedSuperWorker(int input, std::ostream& stream, container* primary, container* secondary, LaunchPolicy policy = LaunchPolicy::deferred) noexcept {
	doMaskedSuperWorker<digitWidth, primarySize, secondarySize, threadCount>(input, 0x7f, stream, primary, secondary, policy);
}

using InputDescription = std::tuple<int, int, int>;
template<u64 digitWidth, u64 primarySize, u64 secondarySize, u64 threadCount = 7>
inline void doMaskedSuperWorker(InputDescription description, std::ostream& stream, container* primary, container* secondary, LaunchPolicy policy = LaunchPolicy::deferred) noexcept {
	int input, digits, secondaryMask;
	std::tie(input, digits, secondaryMask) = description;
    if (input == 0 || digits == 0 || secondaryMask == 0) {
        return;
    }
	doMaskedSuperWorker<digitWidth, primarySize, secondarySize, threadCount>(input, digits, secondaryMask, stream, primary, secondary, policy);
}

inline InputDescription readInputDescription(std::istream& stream = std::cin) noexcept {
	int type;
	int digs;
	int secondaryMask;
	std::cin >> std::hex >> type >> digs >> secondaryMask;
	type &= 0x0f;
	digs &= 0x7f;
	secondaryMask &= 0x7f;
	return std::make_tuple(type, digs, secondaryMask);
}

#endif // end QLIB_H__
