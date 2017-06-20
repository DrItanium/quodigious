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
u64 sums[numElements<2>] = { 0 };
u64 sums3[numElements<3>] = { 0 };
u64 sums4[numElements<4>] = { 0 };
u64 sums8[numElements<8>] = { 0 };
u64 products[numElements<2>] = { 0 };
u64 products3[numElements<3>] = { 0 };
u64 products4[numElements<4>] = { 0 };
u64 products8[numElements<8>] = { 0 };
u64 numbers2[numElements<2>] = { 0 };
u64 numbers3[numElements<3>] = { 0 };
u64 numbers4[numElements<4>] = { 0 };
u64 numbers8[numElements<8>] = { 0 };
template<u64 width>
constexpr u64 makeDigitAt(u64 input) noexcept {
    static_assert(width >= 0, "Can't have negative width!");
    return input * fastPow10<width>;
}
void populateWidth2(u64* sums, u64* products, u64* numbers) noexcept {
    auto* sumPtr = sums;
    auto* prodPtr = products;
    auto* numPtr = numbers;
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

void populateWidth3(u64* sums, u64* products, u64* numbers) noexcept {
    auto* sumPtr = sums;
    auto* prodPtr = products;
    auto* numPtr = numbers;
    for (int i = 2; i < 10; ++i) {
        if (i != 5) {
            auto iNum = makeDigitAt<2>(i);
            for (int j = 2; j < 10; ++j) {
                if (j != 5) {
                    auto jSum = i + j;
                    auto jProd = i * j;
                    auto jNum = makeDigitAt<1>(j) + iNum;
                    for (int k = 2; k < 10; ++k) {
                        if (k != 5) {
                            *sumPtr = jSum + k;
                            *prodPtr = jProd * k;
                            *numPtr = jNum + k;
                            ++sumPtr;
                            ++prodPtr;
                            ++numPtr;
                        }
                    }
                }
            }
        }
    }
}

void populateWidth4(u64* sums, u64* products, u64* numbers) noexcept {
    auto* numPtr = numbers;
    auto* sumPtr = sums;
    auto* prodPtr = products;
    for (int i = 2; i < 10; ++i) {
        if (i != 5) {
            auto iNum = makeDigitAt<3>(i);
            for (int j = 2; j < 10; ++j) {
                if (j != 5) {
                    auto jSum = i + j;
                    auto jProd = i * j;
                    auto jNum = iNum + makeDigitAt<2>(j);
                    for (int k = 2; k < 10; ++k) {
                        if (k != 5) {
                            auto kSum = k + jSum;
                            auto kProd = k * jProd;
                            auto kNum = jNum + makeDigitAt<1>(k);
                            for (int m = 2; m < 10; ++m) {
                                if (m != 5) {
                                    *sumPtr = kSum + m;
                                    *prodPtr = kProd * m;
                                    *numPtr = kNum + m;
                                    ++sumPtr;
                                    ++prodPtr;
                                    ++numPtr;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void populateWidth8(u64* sums4, u64* products4, u64* numbers4, u64* sums8, u64* products8, u64* numbers8) noexcept {
    auto* s8 = sums8;
    auto* p8 = products8;
    auto* n8 = numbers8;
    for (auto i = 0; i < numElements<4>; ++i) {
        auto os = sums4[i];
        auto op = products4[i];
        auto on = numbers4[i] * fastPow10<4>;
        for (auto j = 0; j < numElements<4>; ++j) {
            *s8 = os + sums4[j];
            *p8 = op * products4[j];
            *n8 = on + numbers4[j];
            ++s8;
            ++p8;
            ++n8;
        }
    }
}


u64 values16[numElements<2>] = { 0 };
u64 values14[numElements<2>] = { 0 };
u64 values12[numElements<2>] = { 0 };
u64 values2[numElements<2>] = { 0 };
u64 values12to15[numElements<3>] = { 0 };
u64 values12To15[numElements<3>] = { 0 };
u64 values4To12[numElements<8>] = { 0 };
u64 values12To16[numElements<4>] = { 0 };

void setup() noexcept {
    populateWidth2(sums, products, numbers2);
    populateWidth3(sums3, products3, numbers3);
    populateWidth4(sums4, products4, numbers4);
    populateWidth8(sums4, products4, numbers4, sums8, products8, numbers8);
    auto* n2ptr = numbers2;
    auto* v2 = values2;
    auto* v16 = values16;
    auto* v14 = values14;
    auto* v12 = values12;
    for (auto num : numbers2) {
        *v2 = num * fastPow10<1>;
        *v16 = num * fastPow10<14>;
        *v14 = num * fastPow10<12>;
        *v12 = num * fastPow10<10>;
        ++v2;
        ++v16;
        ++v14;
        ++v12;
    }
    auto* v12To15 = values12To15;
    for (auto num : numbers3) {
        *v12To15 = num * fastPow10<11>;
        ++v12To15;
    }
    auto* v12To16 = values12To16;
    for (auto num : numbers4) {
        *v12To16 = num * fastPow10<11>;
        ++v12To16;
    }
    auto* v4To12 = values4To12;
    for (auto num : numbers8) {
        *v4To12 = num * fastPow10<3>;
        ++v4To12;
    }
}


template<u64 pos, u64 max>
void loopBody(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept;

template<u64 pos, u64 max>
inline std::string loopBodyString(u64 sum, u64 product, u64 index) noexcept;

template<u64 nextPosition, u64 max, u64 count>
void iterativePrecomputedLoopBodyGeneric(std::ostream& storage, u64 sum, u64 product, u64 index, u64* values, u64* sums, u64* products) noexcept {
	auto* ptrSum = sums;
	auto* ptrProd = products;
	auto* ptrVals = values;
	for (int i = 0; i < count; ++i) {
		loopBody<nextPosition, max>(storage, sum + (*ptrSum), product * (*ptrProd), index + (*ptrVals));
		++ptrSum;
		++ptrProd;
		++ptrVals;
	}
}
template<u64 nextPosition, u64 max>
inline void iterativePrecomputedLoopBody(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept {
	iterativePrecomputedLoopBodyGeneric<nextPosition, max, numElements<2>>(storage, sum, product, index, precomputedValues, sums, products);
}
template<u64 nextPosition, u64 max>
inline void iterativePrecomputedLoopBody3(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept {
	iterativePrecomputedLoopBodyGeneric<nextPosition, max, numElements<3>>(storage, sum, product, index, precomputedValues, sums3, products3);
}
template<u64 nextPosition, u64 max>
inline void iterativePrecomputedLoopBody4(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept {
	iterativePrecomputedLoopBodyGeneric<nextPosition, max, numElements<4>>(storage, sum, product, index, precomputedValues, sums4, products4);
}

template<u64 nextPosition, u64 max>
inline void iterativePrecomputedLoopBody8(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept {
	iterativePrecomputedLoopBodyGeneric<nextPosition, max, numElements<8>>(storage, sum, product, index, precomputedValues, sums8, products8);
}
// disable some of these runs in the cases where it exceeds the max
template<> inline void iterativePrecomputedLoopBody<14,12>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> inline void iterativePrecomputedLoopBody<14,13>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> inline void iterativePrecomputedLoopBody<16,12>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> inline void iterativePrecomputedLoopBody<16,13>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> inline void iterativePrecomputedLoopBody<16,14>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> inline void iterativePrecomputedLoopBody<16,15>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> inline void iterativePrecomputedLoopBody<18,12>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> inline void iterativePrecomputedLoopBody<18,13>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> inline void iterativePrecomputedLoopBody<18,14>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> inline void iterativePrecomputedLoopBody<18,15>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> inline void iterativePrecomputedLoopBody<18,16>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> inline void iterativePrecomputedLoopBody<18,17>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }

template<> inline void iterativePrecomputedLoopBody3<15,14>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> inline void iterativePrecomputedLoopBody3<15,13>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> inline void iterativePrecomputedLoopBody3<15,12>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }

template<> inline void iterativePrecomputedLoopBody4<16,15>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> inline void iterativePrecomputedLoopBody4<16,14>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> inline void iterativePrecomputedLoopBody4<16,13>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> inline void iterativePrecomputedLoopBody4<16,12>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }

template<bool topLevel>
struct ActualLoopBody {
	ActualLoopBody() = delete;
	~ActualLoopBody() = delete;
	ActualLoopBody(ActualLoopBody&&) = delete;
	ActualLoopBody(const ActualLoopBody&) = delete;

	template<u64 pos, u64 max>
	static void body(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept {
		constexpr auto next = fastPow10<pos - 1>;
		constexpr auto follow = pos + 1;
		auto originalProduct = product;
		if (pos == 2) {
			auto mkComputation = [&sum, &product, &index](auto uS, auto uP, auto uInd) noexcept { return std::async(std::launch::async, loopBodyString<4, max>, sum + uS, product * uP, index + uInd); };
			auto* ptrSum = sums;
			auto* ptrProd = products;
			auto* ptrVals = values2;
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
			iterativePrecomputedLoopBody8<12, max>(storage, sum, product, index, values4To12);
		} else if (pos == 12 && max == 14) {
			iterativePrecomputedLoopBody<14, max>(storage, sum, product, index, values12);
		} else if (pos == 12 && max == 15) {
			iterativePrecomputedLoopBody3<15, max>(storage, sum, product, index, values12to15);
		} else if (pos == 12 && max >= 16) {
			iterativePrecomputedLoopBody4<16, max>(storage, sum, product, index, values12To16);
		} else if (pos == 16 && max >= 18) {
			iterativePrecomputedLoopBody<18, max>(storage, sum, product, index, values16);
		} else {
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
