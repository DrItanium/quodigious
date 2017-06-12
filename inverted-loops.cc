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

u64 sums[49] = { 0 };
u64 sums3[343] = { 0 };
u64 sums4[2401] = { 0 };
u64 sums8[2401 * 2401] = { 0 };
u64 products[49] = { 0 };
u64 products3[343] = { 0 };
u64 products4[2401] = { 0 };
u64 products8[2401 * 2401] = { 0 };
u64 values16[49] = { 0 };
u64 values14[49] = { 0 };
u64 values12[49] = { 0 };
u64 values2[49] = { 0 };
u64 values12to15[343] = { 0 };
u64 values4To12[2401 * 2401] = { 0 };
u64 values12To16[2401] = { 0 };
void setup() noexcept {
	u64 values10[49] = { 0 };
	u64 values6[49] = { 0 };
	u64 values4[49] = { 0 };
	u64 values8[49] = { 0 };
	u64 values4To8[2401] = { 0 };
	u64 values8To12[2401] = { 0 };
	auto* ptrSum = sums;
	auto* ptrProd = products;
	auto* ptrVal16 = values16;
	auto* ptrVal14 = values14;
	auto* ptrVal12 = values12;
	auto* ptrVal10 = values10;
	auto* ptrVal8 = values8;
	auto* ptrVal6 = values6;
	auto* ptrVal4 = values4;
	auto* ptrVal2 = values2;
	int count = 0;
	for (int i = 2; i < 10; ++i) {
		if (i != 5) {
			auto iIndex10 = (i * fastPow10<9>);
			auto iIndex8 = ( i * fastPow10<7>);
			auto iIndex6 = (i * fastPow10<5>);
			auto iIndex4 = (i * fastPow10<3>);
			auto iIndex12 = (i * fastPow10<11>);
			auto iIndex14 = (i * fastPow10<13>);
			auto iIndex16 = (i * fastPow10<15>);
			auto iIndex2 = (i * fastPow10<1>);
			auto iMul = i;
			auto iSum = i;
			for (int j = 2; j < 10; ++j) {
				if (j != 5) {
					*ptrSum = iSum + j;
					*ptrProd = iMul * j;
					*ptrVal10 = iIndex10 + (j * fastPow10<10>);
					*ptrVal8 = iIndex8 + (j * fastPow10<8>);
					*ptrVal6 = iIndex6 + (j * fastPow10<6>);
					*ptrVal4 = iIndex4 + (j * fastPow10<4>);
					*ptrVal12 = iIndex12 + (j * fastPow10<12>);
					*ptrVal14 = iIndex14 + (j * fastPow10<14>);
					*ptrVal16 = iIndex16 + (j * fastPow10<16>);
					*ptrVal2 = iIndex2 + (j * fastPow10<2>);
					++count;
					++ptrSum;
					++ptrProd;
					++ptrVal10;
					++ptrVal8;
					++ptrVal6;
					++ptrVal4;
					++ptrVal2;
					++ptrVal12;
					++ptrVal14;
					++ptrVal16;
				}
			}
		}
	}
	if (count != 49) {
		throw "Expected exactly 49 entries!";
	}
	count = 0;
	auto* s4ptr = sums4;
	auto* p4ptr = products4;
	auto* v4to8 = values4To8;
	auto* v8to12 = values8To12;
	auto* v12to16 = values12To16;
	auto* v4 = values4;
	auto* v8 = values8;
	auto* v12 = values12;
	auto* oProd = products;
	auto* oSum = sums;
	for (int i = 0; i < 49; ++i) {
		auto s0 = *oSum;
		auto p0 = *oProd;
		auto v0_4 = *v4;
		auto v0_8 = *v8;
		auto v0_12 = *v12;
		auto* iProd = products;
		auto* iSum = sums;
		auto* v6 = values6;
		auto* v10 = values10;
		auto* v14 = values14;
		for (int j = 0; j < 49; ++j) {
			*s4ptr = s0 + (*iSum);
			*p4ptr = p0 * (*iProd);
			*v4to8 = (*v6) + (v0_4);
			*v8to12 = (*v10) + (v0_8);
			*v12to16 = (*v14) + (v0_12);
			++count;
			++s4ptr;
			++p4ptr;
			++iSum;
			++iProd;
			++v6;
			++v10;
			++v14;
			++v4to8;
			++v8to12;
			++v12to16;
		}
		++oSum;
		++oProd;
		++v4;
		++v8;
		++v12;
	}
	if (count != 2401) {
		throw "Expected exactly 2401 entries!";
	}

	count = 0;
	auto* v48 = values4To8;
	auto* s8ptr = sums8;
	auto* p8ptr = products8;
	auto* v412 = values4To12;
	s4ptr = sums4;
	p4ptr = products4;
	for (int i = 0; i < 2401; ++i) {
		auto s0 = *s4ptr;
		auto p0 = *p4ptr;
		auto v0 = *v48;
		auto* is4ptr = sums4;
		auto* ip4ptr = products4;
		auto *v812 = values8To12;
		for (int j = 0; j < 2401; ++j) {
			*s8ptr = s0 + (*is4ptr);
			*p8ptr = p0 * (*ip4ptr);
			*v412 = v0 + (*v812);
			++count;
			++s8ptr;
			++p8ptr;
			++is4ptr;
			++ip4ptr;
			++v412;
			++v812;
		}
		++s4ptr;
		++p4ptr;
		++v48;
	}
	if (count != (2401 * 2401)) {
		throw "Expected exactly 7^8 entries!";
	}

	count = 0;
	auto* s2 = sums;
	auto* p2 = products;
	auto* s3 = sums3;
	auto* p3 = products3;
	auto* v14 = values14;
	auto* v12to15 = values12to15;
	for (int i = 0; i < 49; ++i) {
		auto s0 = *s2;
		auto p0 = *p2;
		auto v0 = *v14;
		for (int j = 2; j < 10; ++j) {
			if (j != 5) {
				*s3 = s0 + j;
				*p3 = s0 * j;
				*v12to15 = *v14 + (j * fastPow10<14>);
				++count;
				++s3;
				++p3;
				++v12to15;
			}
		}
		++s2;
		++p2;
		++v14;
	}
	if (count != 343) {
		throw "Expected exactly 343 entries";
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
void iterativePrecomputedLoopBody(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept {
	iterativePrecomputedLoopBodyGeneric<nextPosition, max, 49>(storage, sum, product, index, precomputedValues, sums, products);
}
template<u64 nextPosition, u64 max>
void iterativePrecomputedLoopBody4(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept {
	iterativePrecomputedLoopBodyGeneric<nextPosition, max, 2401>(storage, sum, product, index, precomputedValues, sums4, products4);
}

template<u64 nextPosition, u64 max>
void iterativePrecomputedLoopBody8(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept {
	iterativePrecomputedLoopBodyGeneric<nextPosition, max, (2401 * 2401)>(storage, sum, product, index, precomputedValues, sums8, products8);
}
// disable some of these runs in the cases where it exceeds the max
template<> void iterativePrecomputedLoopBody<14,12>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> void iterativePrecomputedLoopBody<14,13>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> void iterativePrecomputedLoopBody<16,12>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> void iterativePrecomputedLoopBody<16,13>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> void iterativePrecomputedLoopBody<16,14>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> void iterativePrecomputedLoopBody<16,15>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> void iterativePrecomputedLoopBody<18,12>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> void iterativePrecomputedLoopBody<18,13>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> void iterativePrecomputedLoopBody<18,14>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> void iterativePrecomputedLoopBody<18,15>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> void iterativePrecomputedLoopBody<18,16>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> void iterativePrecomputedLoopBody<18,17>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }

template<> void iterativePrecomputedLoopBody4<16,15>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> void iterativePrecomputedLoopBody4<16,14>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> void iterativePrecomputedLoopBody4<16,13>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
template<> void iterativePrecomputedLoopBody4<16,12>(std::ostream& storage, u64 sum, u64 product, u64 index, u64* precomputedValues) noexcept { }
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
			iterativePrecomputedLoopBody<16, max>(storage, sum, product, index, values12to15);
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
		constexpr auto next = fastPow10<pos - 1>;
		auto originalProduct = product;
		product <<= 1;
		sum += 2;
		index += multiply<2>(next);
		auto advance = [&originalProduct, &product, &sum, &index]() noexcept { product += originalProduct; ++sum; index += next; };
		auto merge = [&storage](auto value) noexcept { if (value != 0) { storage << value << std::endl; } };
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
