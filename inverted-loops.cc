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

constexpr bool checkValue(u64 sum) noexcept {
	return (isEven(sum)) || (sum % 3 == 0);
}
constexpr u64 performCheck(u64 sum, u64 product, u64 value) noexcept {
	if (checkValue(sum) && isQuodigious(value, sum, product)) {
		return value;
	}
	return 0;
}
void innerMostBody(std::ostream& stream, u64 sum, u64 product, u64 value) noexcept {
	// inject the least significant digits 2,4,6,8
	merge(performCheck(sum + 2, product << 1, value + 2), stream); // 2
	merge(performCheck(sum + 4, product << 2, value + 4), stream); // 4
	merge(performCheck(sum + 6, (product << 1) + (product << 2) , value + 6), stream); // 6
	merge(performCheck(sum + 8, product << 3, value + 8), stream); // 8
}

template<u64 width>
inline u64* getSums() noexcept {
    static_assert(width >= 2 && width < 9, "Illegal width!");
    return nullptr;
}
template<u64 width>
inline u64* getProducts() noexcept {
    static_assert(width >= 2 && width < 9, "Illegal width!");
    return nullptr;
}
template<u64 width>
inline u64* getNumbers() noexcept {
    static_assert(width >= 2 && width < 9, "Illegal width!");
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
#undef defTripleStorage

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

u64 values2To4[numElements<2>] = { 0 };

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

void setup() noexcept {
	// not sure why, but this storage here seems to make the program go much
	// faster, the odd part is, I'm not using the majority of this memory. I
	// think it has something to do with cache coherency.
    populateWidth<2>();
    populateWidth<3>();
    populateWidth<4>();
    populateWidth<5>();
    populateWidth<8>();
    populateArray<2, 1>(values2To4);
}


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
		if (pos == 2) {
			static constexpr auto threadCount = 49;
			auto mkComputation = [sum, product, index](auto uS, auto uP, auto uInd) noexcept { return std::async(std::launch::async, loopBodyString<4, max>, sum + uS, product * uP, index + uInd); };
			auto* ptrSum = sums2;
			auto* ptrProd = products2;
			auto* ptrVals = values2To4;
			decltype(mkComputation(*ptrSum, *ptrProd, *ptrVals)) watcher[threadCount];
			for (int i = 0; i < threadCount; ++i, ++ptrSum, ++ptrProd, ++ptrVals) {
				watcher[i] = mkComputation(*ptrSum, *ptrProd, *ptrVals);
			}
			for (int i = 0; i < threadCount; ++i) {
				storage << watcher[i].get();
			}
		} else {
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

template<u64 length>
inline void body(std::ostream& storage) noexcept {
    static_assert(length <= 19, "Can't have numbers over 19 digits at this time!");
	loopBody<2, length>(storage, 0, 1, 0);
}


int main() {
    std::ostringstream storage;
	setup();
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
