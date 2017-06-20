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

u64* getSums(u64 width) noexcept {
    static std::map<u64, u64*> ptrs = {
        { 2, sums2 },
        { 3, sums3 },
        { 4, sums4 },
        { 5, sums5 },
        { 6, sums6 },
        { 7, sums7 },
        { 8, sums8 },
    };
    auto point = ptrs.find(width);
    if (point == ptrs.end()) {
        return nullptr;
    } else {
        return point->second;
    }
}

u64* getProducts(u64 width) noexcept {
    static std::map<u64, u64*> ptrs = {
        { 2, products2 },
        { 3, products3 },
        { 4, products4 },
        { 5, products5 },
        { 6, products6 },
        { 7, products7 },
        { 8, products8 },
    };
    auto point = ptrs.find(width);
    if (point == ptrs.end()) {
        return nullptr;
    } else {
        return point->second;
    }
}

template<u64 width>
constexpr u64 makeDigitAt(u64 input) noexcept {
    static_assert(width >= 0, "Can't have negative width!");
    return input * fastPow10<width>;
}
void populateWidth2() noexcept {
    static bool populated = false;
    if (!populated) {
        populated = true;
        auto* sumPtr = sums2;
        auto* prodPtr = products2;
        auto* numPtr = numbers2;
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

void populateWidth3() noexcept {
    static bool populated = false;
    if (!populated) {
        populated = true;
        auto* sumPtr = sums3;
        auto* prodPtr = products3;
        auto* numPtr = numbers3;
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
}

void populateWidth4() noexcept {
    static bool populated = false;
    if (!populated) {
        populated = true;
        auto* numPtr = numbers4;
        auto* sumPtr = sums4;
        auto* prodPtr = products4;
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
}

void populateWidth5() noexcept {
    static bool populated = false;
    if (!populated) {
        populated = true;
        populateWidth4();
        auto* numPtr = numbers5;
        auto* sumPtr = sums5;
        auto* prodPtr = products5;
        for (int i = 0; i < numElements<4>; ++i) {
            auto s4 = sums4[i];
            auto p4 = products4[i];
            auto n4 = makeDigitAt<1>(numbers4[i]);
            for (int j = 2; j < 10; ++j) {
                if (j != 5) {
                    *numPtr = n4 + j;
                    *sumPtr = s4 + j;
                    *prodPtr = p4 + j;
                    ++numPtr;
                    ++sumPtr;
                    ++prodPtr;
                }
            }
        }
    }
}

void populateWidth6() noexcept {
    static bool populated = false;
    if (!populated) {
        populated = true;
        populateWidth5();
        auto* numPtr = numbers6;
        auto* sumPtr = sums6;
        auto* prodPtr = products6;
        for (int i = 0; i < numElements<5>; ++i) {
            auto s5 = sums5[i];
            auto p5 = products5[i];
            auto n5 = makeDigitAt<1>(numbers5[i]);
            for (int j = 2; j < 10; ++j) {
                if (j != 5) {
                    *numPtr = n5 + j;
                    *sumPtr = s5 + j;
                    *prodPtr = p5 + j;
                    ++numPtr;
                    ++sumPtr;
                    ++prodPtr;
                }
            }
        }
    }
}

void populateWidth7() noexcept {
    static bool populated = false;
    if (!populated) {
        populated = true;
        populateWidth6();
        auto* numPtr = numbers7;
        auto* sumPtr = sums7;
        auto* prodPtr = products7;
        for (int i = 0; i < numElements<6>; ++i) {
            auto s6 = sums6[i];
            auto p6 = products6[i];
            auto n6 = makeDigitAt<1>(numbers6[i]);
            for (int j = 2; j < 10; ++j) {
                if (j != 5) {
                    *numPtr = n6 + j;
                    *sumPtr = s6 + j;
                    *prodPtr = p6 + j;
                    ++numPtr;
                    ++sumPtr;
                    ++prodPtr;
                }
            }
        }
    }
}


void populateWidth8() noexcept {
    static bool populated = false;
    if (!populated) {
        populated = true;
        populateWidth4();
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

template<int width>
void populateArray(u64* nums, u64* storage) noexcept {
    for (int i = 0; i < numElements<width>; ++i) {
        *storage = nums[i] * fastPow10<11>;
        ++storage;
    }
}
void setup() noexcept {
    populateWidth2();
    populateWidth3();
    populateWidth4();
    populateWidth5();
    populateWidth6();
    populateWidth7();
    populateWidth8();
    auto* n2ptr = numbers2;
    auto* v2 = values2To4;
    auto* v16 = values16;
    auto* v12 = values12To14;
    for (auto num : numbers2) {
        *v2 = num * fastPow10<1>;
        *v16 = num * fastPow10<14>;
        *v12 = num * fastPow10<10>;
        ++v2;
        ++v16;
        ++v12;
    }
    auto* v4To12 = values4To12;
    for (auto num : numbers8) {
        *v4To12 = num * fastPow10<3>;
        ++v4To12;
    }
    populateArray<3>(numbers3, values12To15);
    populateArray<4>(numbers4, values12To16);
    populateArray<5>(numbers5, values12To17);
    populateArray<6>(numbers6, values12To18);
    populateArray<7>(numbers7, values12To19);
}


template<u64 pos, u64 max>
void loopBody(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept;

template<u64 pos, u64 max>
inline std::string loopBodyString(u64 sum, u64 product, u64 index) noexcept;

template<u64 nextPosition, u64 max, u64 count, u64 width>
void iterativePrecomputedLoopBodyGeneric(std::ostream& storage, u64 sum, u64 product, u64 index, u64* values) noexcept {
	auto* ptrSum = getSums(width);
	auto* ptrProd = getProducts(width);
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

// disable some of these runs in the cases where it exceeds the max
#define stubLoopBody(a, b, c) \
    template<> inline void iterativePrecomputedLoopBody< a, b, c >(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
stubLoopBody(14,12,2);
stubLoopBody(14,13,2);
stubLoopBody(15,14,3);
stubLoopBody(15,13,3);
stubLoopBody(15,12,3);
stubLoopBody(16,13,2);
stubLoopBody(16,14,2);
stubLoopBody(16,15,2);
stubLoopBody(18,12,2);
stubLoopBody(18,13,2);
stubLoopBody(18,14,2);
stubLoopBody(18,15,2);
stubLoopBody(18,16,2);
stubLoopBody(18,17,2);
stubLoopBody(16,15,4);
stubLoopBody(16,14,4);
stubLoopBody(16,13,4);
stubLoopBody(16,12,4);
#undef stubLoopBody

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
		} else if (pos == 16 && max >= 18) {
			iterativePrecomputedLoopBody<18, max, 2>(storage, sum, product, index, values16);
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
