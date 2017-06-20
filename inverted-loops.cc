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
#include <map>
#include "qlib.h"

inline constexpr bool checkValue(u64 sum) noexcept {
	return (sum % 2 == 0) || (sum % 3 == 0);
}
inline constexpr u64 innerMostBody(u64 sum, u64 product, u64 value) noexcept {
	if (checkValue(sum) && isQuodigious(value, sum, product)) {
		return value;
	}
	return 0;
}
template<u64 width>
constexpr int numberOfDigitsForGivenWidth() noexcept {
    static_assert(width >= 0, "Negative width doesn't make sense");
    return 7 * numberOfDigitsForGivenWidth<width - 1>();
}
template<> constexpr int numberOfDigitsForGivenWidth<0>() noexcept { return 1; }
template<u64 width>
constexpr auto numElements = numberOfDigitsForGivenWidth<width>();
u64 sums2[numElements<2>] = { 0 };
u64 sums3[numElements<3>] = { 0 };
u64 sums4[numElements<4>] = { 0 };
u64 sums5[numElements<5>] = { 0 };
u64 sums6[numElements<6>] = { 0 };
u64 sums7[numElements<7>] = { 0 };
u64 sums8[numElements<8>] = { 0 };
u64 products2[numElements<2>] = { 0 };
u64 products3[numElements<3>] = { 0 };
u64 products4[numElements<4>] = { 0 };
u64 products5[numElements<5>] = { 0 };
u64 products6[numElements<6>] = { 0 };
u64 products7[numElements<7>] = { 0 };
u64 products8[numElements<8>] = { 0 };
u64 numbers2[numElements<2>] = { 0 };
u64 numbers3[numElements<3>] = { 0 };
u64 numbers4[numElements<4>] = { 0 };
u64 numbers5[numElements<5>] = { 0 };
u64 numbers6[numElements<6>] = { 0 };
u64 numbers7[numElements<7>] = { 0 };
u64 numbers8[numElements<8>] = { 0 };

template<u64 width>
inline u64* getSums() noexcept {
    static_assert(width >= 2 && width < 9, "Illegal width!");
    return nullptr;
}
#define defSum(width) \
    template<> inline u64* getSums< width > () noexcept { return sums ## width ; }
defSum(2);
defSum(3);
defSum(4);
defSum(5);
defSum(6);
defSum(7);
defSum(8);
#undef defSum

template<u64 width>
inline u64* getProducts() noexcept {
    static_assert(width >= 2 && width < 9, "Illegal width!");
    return nullptr;
}

#define defProduct(width) \
    template<> inline u64* getProducts< width > () noexcept { return products ## width ; }
defProduct(2);
defProduct(3);
defProduct(4);
defProduct(5);
defProduct(6);
defProduct(7);
defProduct(8);
#undef defProduct

template<u64 width>
inline u64* getNumbers() noexcept {
    static_assert(width >= 2 && width < 9, "Illegal width!");
    return nullptr;
}

#define defNumber(width) \
    template<> inline u64* getNumbers< width > () noexcept { return numbers ## width ; }
defNumber(2);
defNumber(3);
defNumber(4);
defNumber(5);
defNumber(6);
defNumber(7);
defNumber(8);
#undef defNumber

template<u64 width>
constexpr u64 makeDigitAt(u64 input) noexcept {
    static_assert(width >= 0, "Can't have negative width!");
    return input * fastPow10<width>;
}

template<u64 width>
void populateWidth() noexcept {
    static_assert(width >= 2 && width < 9, "Illegal width!");
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
                if (j != 5) {
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
            if (i != 5) {
                auto numberOuter = makeDigitAt<1>(i);
                for (int j = 2; j < 10; ++j) {
                    if (j != 5) {
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
    }
}


u64 values16[numElements<2>] = { 0 };
u64 values2To4[numElements<2>] = { 0 };
u64 values4To12[numElements<8>] = { 0 };
u64 values12To14[numElements<2>] = { 0 };
u64 values12To15[numElements<3>] = { 0 };
u64 values12To16[numElements<4>] = { 0 };
u64 values12To17[numElements<5>] = { 0 };
u64 values12To18[numElements<6>] = { 0 };
u64 values12To19[numElements<7>] = { 0 };

template<u64 width, u64 factor>
inline void populateArray(u64* nums, u64* storage) noexcept {
    for (int i = 0; i < numElements<width>; ++i) {
        *storage = nums[i] * fastPow10<factor>;
        ++storage;
    }
}

void setup() noexcept {
    populateWidth<2>();
    populateWidth<3>();
    populateWidth<4>();
    populateWidth<5>();
    populateWidth<6>();
    populateWidth<7>();
    populateWidth<8>();
    populateArray<2, 1>(numbers2, values2To4);
    populateArray<2, 14>(numbers2, values16);
    populateArray<2, 10>(numbers2, values12To14);
    populateArray<8, 3>(numbers8, values4To12);
    populateArray<3, 11>(numbers3, values12To15);
    populateArray<4, 11>(numbers4, values12To16);
    populateArray<5, 11>(numbers5, values12To17);
    populateArray<6, 11>(numbers6, values12To18);
    populateArray<7, 11>(numbers7, values12To19);
}


template<u64 pos, u64 max>
void loopBody(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept;


template<u64 nextPosition, u64 max, u64 count, u64 width>
void iterativePrecomputedLoopBodyGeneric(std::ostream& storage, u64 sum, u64 product, u64 index, u64* values) noexcept {
	auto* ptrSum = getSums<width>();
	auto* ptrProd = getProducts<width>();
	auto* ptrVals = values;
	for (int i = 0; i < count; ++i) {
		loopBody<nextPosition, max>(storage, sum + (*ptrSum), product * (*ptrProd), index + (*ptrVals));
		++ptrSum;
		++ptrProd;
		++ptrVals;
	}
}
template<u64 nextPosition, u64 max, u64 width>
inline void iterativePrecomputedLoopBody(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept {
    iterativePrecomputedLoopBodyGeneric<nextPosition, max, numElements<width>, width>(storage, sum, product, index, precomputedValues);
}

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
		if (pos == 2) {
			auto mkComputation = [&sum, &product, &index](auto uS, auto uP, auto uInd) noexcept { return std::async(std::launch::async, loopBodyString<4, max>, sum + uS, product * uP, index + uInd); };
			auto* ptrSum = sums2;
			auto* ptrProd = products2;
			auto* ptrVals = values2To4;
			auto first = mkComputation(*ptrSum, *ptrProd, *ptrVals);
			decltype(first) watcher[48];
			auto* w = watcher;
			++ptrSum;
			++ptrProd;
			++ptrVals;
			for (int i = 1; i < 49; ++i, ++w, ++ptrSum, ++ptrProd, ++ptrVals) {
				*w = mkComputation(*ptrSum, *ptrProd, *ptrVals);
			}
			w = watcher;
			storage << first.get();
			for (int i = 0; i < 48; ++i, ++w) {
				storage << w->get();
			}
		} else if (pos == 4) {
			iterativePrecomputedLoopBody<12, max, 8>(storage, sum, product, index, values4To12);
		} else if (pos == 12 && max == 14) {
			iterativePrecomputedLoopBody<max, max, 2>(storage, sum, product, index, values12To14);
		} else if (pos == 12 && max == 15) {
			iterativePrecomputedLoopBody<max, max, 3>(storage, sum, product, index, values12To15);
		} else if (pos == 12 && max == 16) {
			iterativePrecomputedLoopBody<max, max, 4>(storage, sum, product, index, values12To16);
        } else if (pos == 12 && max == 17) {
            iterativePrecomputedLoopBody<max, max, 5>(storage, sum, product, index, values12To17);
        } else if (pos == 12 && max == 18) {
            iterativePrecomputedLoopBody<max, max, 6>(storage, sum, product, index, values12To18);
        } else if (pos == 12 && max == 19) {
            iterativePrecomputedLoopBody<max, max, 7>(storage, sum, product, index, values12To19);
		} else {
            constexpr auto next = fastPow10<pos - 1>;
            constexpr auto follow = pos + 1;
            auto originalProduct = product;
			product <<= 1;
			sum += 2;
			index += multiply<2>(next);
			for (int i = 2; i < 10; ++i, product += originalProduct, ++sum, index += next) {
				if (i != 5) {
					loopBody<follow, max>(storage, sum, product, index);
				}
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
		auto merge = [&storage](auto value) noexcept { if (value != 0) { storage << value << std::endl; } };
		if (max == 15) {
			merge(innerMostBody(sum, product, index));
		} else {
			constexpr auto next = fastPow10<pos - 1>;
			auto originalProduct = product;
			product <<= 1;
			sum += 2;
			index += multiply<2>(next);
			auto advance = [&originalProduct, &product, &sum, &index]() noexcept { product += originalProduct; ++sum; index += next; };
			merge(innerMostBody(sum, product, index)); // 2
			advance();
			merge(innerMostBody(sum, product, index)); // 3
			advance();
			merge(innerMostBody(sum, product, index)); // 4
			advance();
			advance();
			merge(innerMostBody(sum, product, index)); // 6
			advance();
			merge(innerMostBody(sum, product, index)); // 7
			advance();
			merge(innerMostBody(sum, product, index)); // 8
			advance();
			merge(innerMostBody(sum, product, index)); // 9
		}
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

template<u64 length>
inline void body(std::ostream& storage) noexcept {
    static_assert(length <= 19, "Can't have numbers over 19 digits at this time!");
    // spawn each section at the same time, 196 threads will be spawned for
    // simultaneous processing
	auto p0 = std::async(std::launch::async, loopBodyString<2, length>, 2, 2, 2);
	auto p1 = std::async(std::launch::async, loopBodyString<2, length>, 4, 4, 4);
	auto p2 = std::async(std::launch::async, loopBodyString<2, length>, 6, 6, 6);
	auto p3 = std::async(std::launch::async, loopBodyString<2, length>, 8, 8, 8);
    storage << p0.get() << p1.get() << p2.get() << p3.get();
}

int main() {
    std::ostringstream storage;
	setup();
    while(std::cin.good()) {
        u64 currentIndex = 0;
        std::cin >> currentIndex;
        if (std::cin.good()) {
            switch(currentIndex) {
                case 12: body<12>(storage); break;
                case 13: body<13>(storage); break;
                case 14: body<14>(storage); break;
                case 15: body<15>(storage); break;
                case 16: body<16>(storage); break;
                case 17: body<17>(storage); break;
                case 18: body<18>(storage); break;
                case 19: body<19>(storage); break;
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
