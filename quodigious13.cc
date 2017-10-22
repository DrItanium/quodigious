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
#include <cmath>
#include <cstdint>
#include <sstream>
#include <functional>
#include <future>
#include "qlib.h"

constexpr bool checkValue(u64 sum) noexcept {
	return (isEven(sum) || (sum % 3 == 0));
}
constexpr u64 innerMostBody(u64 sum, u64 product, u64 value) noexcept {
	if (checkValue(sum) && isQuodigious(value, sum, product)) {
		return value;
	}
	return 0;
}

template<u64 width>
inline u64* getSums() noexcept {
    static_assert(width >= 2 && width <= 9, "Illegal width!");
    return nullptr;
}
template<u64 width>
inline u64* getProducts() noexcept {
    static_assert(width >= 2 && width <= 9, "Illegal width!");
    return nullptr;
}
template<u64 width>
inline u64* getNumbers() noexcept {
    static_assert(width >= 2 && width <= 9, "Illegal width!");
    return nullptr;
}
#define defTripleStorage(width) \
    u64 sums ## width [ numElements< width > ] = { 0 }; \
    u64 products ## width [ numElements < width > ] = { 0 }; \
    u64 numbers ## width [ numElements < width > ] = { 0 }; \
    template<> inline u64* getSums< width > () noexcept { return sums ## width ; } \
    template<> inline u64* getProducts< width > () noexcept { return products ## width ; } \
    template<> inline u64* getNumbers< width > () noexcept { return numbers ## width ; }
defTripleStorage(2);
defTripleStorage(3);
defTripleStorage(4);
defTripleStorage(5);
defTripleStorage(6);
defTripleStorage(7);
defTripleStorage(8);
defTripleStorage(9);
#undef defTripleStorage

template<u64 width>
void populateWidth() noexcept {
    static_assert(width >= 2 && width <= 9, "Illegal width!");
    static bool populated = false;
    if (!populated) {
        populated = true;
        populateWidth<width - 1>();
        auto* numPtr = getNumbers<width>();
        auto* sumPtr = getSums<width>();
        auto* prodPtr = getProducts<width>();
        auto* prevNum = getNumbers<width - 1>();
        auto* prevSum = getSums<width - 1>();
        auto* prevProd = getProducts<width - 1>();
        for (int i = 0; i < numElements<width - 1>; ++i) {
            auto s = prevSum[i];
            auto p = prevProd[i];
            auto n = makeDigitAt<1>(prevNum[i]);
			for (int j = 2; j < 10; ++j) {
				if (j == 5) {
					continue;
				}
				*numPtr = n + j;
				*sumPtr = s + j;
				*prodPtr = p * j;
				++numPtr;
				++sumPtr;
				++prodPtr;
			}
        }
    }
}

template<>
void populateWidth<2>() noexcept {
    static bool populated = false;
    if (!populated) {
        populated = true;
        auto* sumPtr = getSums<2>();
        auto* prodPtr = getProducts<2>();
        auto* numPtr = getNumbers<2>();
		for (int i = 2; i < 10; ++i) {
			if (i == 5) {
				continue;
			}
			auto numberOuter = makeDigitAt<1>(i);
			for (int j = 2; j < 10; ++j) {
				if (j == 5) {
					continue;
				}
				*sumPtr = i + j;
				*prodPtr = i * j;
				*numPtr = numberOuter + j;
				++sumPtr;
				++prodPtr;
				++numPtr;
			}
		}
    }
}
constexpr auto length1To4 = numElements<2> * 4;
u64 values1To4[length1To4] = { 0 };
u64 sums1To4[length1To4] = { 0 };
u64 products1To4[length1To4] = { 0 };
u64 values2To4[numElements<2>] = { 0 };
u64 values4To13[numElements<9>] = { 0 };

template<u64 width, u64 factor>
inline void populateArray(u64* nums, u64* storage) noexcept {
    for (int i = 0; i < numElements<width>; ++i) {
        *storage = nums[i] * fastPow10<factor>;
        ++storage;
    }
}

template<u64 width, u64 factor>
inline void populateArray(u64* storage) noexcept {
    populateArray<width, factor>(getNumbers<width>(), storage);
}

template<u64 pos, u64 max>
void loopBody(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept;


template<bool topLevel>
struct ActualLoopBody {
	ActualLoopBody() = delete;
	~ActualLoopBody() = delete;
	ActualLoopBody(ActualLoopBody&&) = delete;
	ActualLoopBody(const ActualLoopBody&) = delete;

	template<u64 pos, u64 max>
	static void body(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept {
		if (pos == 4) {
			auto* s9 = sums9;
			auto* p9 = products9;
			auto* v9 = values4To13;
			for (auto i = 0u; i < numElements<9>; ++i, ++s9, ++p9, ++v9) {
				loopBody<13, max>(storage, sum + *s9, product * (*p9), index + *v9);
			}
		} 
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
        auto advance = [originalProduct, &product, &sum, &index]() noexcept { product += originalProduct; ++sum; index += next; };
        merge(innerMostBody(sum, product, index), storage); advance(); // 2
        merge(innerMostBody(sum, product, index), storage); advance(); // 3
        merge(innerMostBody(sum, product, index), storage); // 4
		product += (originalProduct << 1);
		sum += 2;
		index += doubleNext;
        merge(innerMostBody(sum, product, index), storage); advance(); // 6
        merge(innerMostBody(sum, product, index), storage); advance(); // 7
        merge(innerMostBody(sum, product, index), storage); advance(); // 8
        merge(innerMostBody(sum, product, index), storage); // 9


	}
};

template<u64 pos, u64 max>
void loopBody(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept {
	static_assert (pos <= max, "Position can't be larger than maximum!");
	// walk through two separate set of actions
	ActualLoopBody<pos == max>::template body< pos, max > (storage, sum, product, index);
}

template<u64 pos, u64 max>
std::string loopBodyString(u64 sum, u64 product, u64 index) noexcept {
	std::ostringstream storage;
	loopBody<pos, max> (storage, sum, product, index);
	return storage.str();
}


int main() {
	populateWidth<2>();
	populateArray<2, 1>(values2To4);
	auto* v1To4 = values1To4;
	auto* s1To4 = sums1To4;
	auto* p1To4 = products1To4;
	for (int i = 2; i < 10; ++i) {
		if (isEven(i)) {
			for (auto j = 0; j < numElements<2>; ++j, ++v1To4, ++s1To4, ++p1To4) {
				*v1To4 = values2To4[j] + i;
				*s1To4 = sums2[j] + i;
				*p1To4 = products2[j] * i;
			}
		}
	}
    populateWidth<9>();
	populateArray<9, 3>(values4To13);
	static constexpr auto threadCount = length1To4;
	auto mkComputation = [](auto i) noexcept {
		return std::async(std::launch::async, loopBodyString<4, 13>, sums1To4[i], products1To4[i], values1To4[i]);
	};
	using AsyncWorker = decltype(mkComputation(0));
	AsyncWorker watcher[threadCount];
	for (int i = 0; i < threadCount; ++i) {
		watcher[i] = mkComputation(i);
	}
	for (int i = 0; i < threadCount; ++i) {
		std::cout << watcher[i].get();
	}
    return 0;
}
