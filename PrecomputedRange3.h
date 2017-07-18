//  Copyright (c) 2017 Joshua Scoggins
//
//  This software is provided 'as-is', without any express or implied
//  warranty. In no event will the authors be held liable for any damages
//  arising from the use of this software.  //
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

#ifndef PRECOMPUTED_RANGE3_H__
#define PRECOMPUTED_RANGE3_H__
#include "qlib.h"
#include "Triple.h"

constexpr auto digits3Width = numElements<2>;
Triple range3[digits3Width];
// these were the three least significant digits for all numbers 13 digits and
// above! So we can do 49 numbers instead of 196!
u64 collection3[digits3Width] = {
	224, 232, 248, 264, 272, 288, 296,
	328, 336, 344, 368, 376, 384, 392,
	424, 432, 448, 464, 472, 488, 496,
	624, 632, 648, 664, 672, 688, 696,
	728, 736, 744, 768, 776, 784, 792,
	824, 832, 848, 864, 872, 888, 896,
	928, 936, 944, 968, 976, 984, 992,
};

inline void setupPrecomputedWidth3() noexcept {
    for (int i = 0; i < digits3Width; ++i) {
        auto number = collection3[i];
        auto digits0 = number % 10;
        auto digits1 = (number / 10) % 10;
        auto digits2 = (number / 100) % 10;
        range3[i].assume(digits0 + digits1 + digits2, digits0 * digits1 * digits2, number);
    }
}

template<u64 count, u64 topRangeWidth>
inline std::string performQuodigiousWalk3(int start, int stop) noexcept {
    ArrayView<count, digits3Width> tmp;
	auto t8 = getTriples<8>();
	for (int i = start, j = 0; i < stop; ++i) {
		auto curr = t8[i];
		auto sum = curr.getSum();
		auto product = curr.getProduct();
		auto number = curr.getNumber() * fastPow10<4>;
		for (auto const& tmp2 : range3) {
			tmp[j].assume(sum + tmp2.getSum(), product * tmp2.getProduct(), number + tmp2.getNumber());
			++j;
		}
	}
	std::stringstream storage;
	static constexpr auto factor = getPartialSize<topRangeWidth, 49>();
    static_assert(isDivisibleBy<topRangeWidth, 49>(factor), "Not divisible");
	auto mkBody = [&tmp](auto mult) noexcept {
		auto start = mult * factor;
		auto stop = (mult + 1) * factor;
		auto fn = [&tmp, start, stop]() noexcept {
			std::stringstream storage;
            auto topRange = getTopRangeTriples<topRangeWidth>();
			for (int i = start; i < stop; ++i) {
				auto s = topRange[i].getSum();
				auto p = topRange[i].getProduct();
				auto n = topRange[i].getNumber();
				for (auto const& curr : tmp) {
					if (curr.isQuodigious(s, p, n)) {
						storage << curr.buildNumber(n) << std::endl;
					}
				}
			}
			return storage.str();
		};
		return std::async(std::launch::async, fn);
	};
	auto b0 = mkBody(0);
	decltype(b0) rest[48];
	for (int i = 0, j = 1; i < 48; ++i, ++j) {
		rest[i] = mkBody(j);
	}
	storage << b0.get();
	for (int i = 0; i < 48; ++i) {
		storage << rest[i].get();
	}
	return storage.str();
}
#endif
