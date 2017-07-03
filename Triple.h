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
#ifndef TRIPLE_H__
#define  TRIPLE_H__
#include "qlib.h"
class Triple {
	private:
		static inline constexpr bool innerMostBody(u64 sum, u64 product, u64 value) noexcept {
			return (value % product == 0) && (value % sum == 0);
		}
	public:
		Triple(u64 s, u64 p, u64 n) : _sum(s), _product(p), _number(n) { }
		Triple() : Triple(0, 0, 0) { }
		inline bool assume(u64 sum, u64 product, u64 number) noexcept {
			_sum = sum;
			_product = product;
			_number = number;
		}
		inline bool isQuodigious(u64 sCombine, u64 pCombine, u64 nCombine) const noexcept {
			return innerMostBody(sCombine + _sum, pCombine * _product, nCombine + _number);
		}
		inline u64 buildNumber(u64 offset) const noexcept {
			return _number + offset;
		}
		inline u64 getSum() const noexcept { return _sum; }
		inline u64 getProduct() const noexcept { return _product; }
		inline u64 getNumber() const noexcept { return _number; }
	private:
		u64 _sum;
		u64 _product;
		u64 _number;
};

template<u64 width>
inline Triple* getTriples() noexcept {
	static_assert(width >= 2 && width < 9, "Illegal width!");
	static Triple elements[numElements<width>];
	return elements;
}

template<u64 width>
inline void populateWidth() noexcept {
	static_assert(width >= 2 && width < 9, "Illegal width!");
	populateWidth<width - 1>();
	auto* triple = getTriples<width>();
	auto* prev = getTriples<width - 1>();
	for (int i = 0; i < numElements<width - 1>; ++i) {
		auto tmp = prev[i];
		auto s = tmp.getSum();
		auto p = tmp.getProduct();
		auto n = makeDigitAt<1>(tmp.getNumber());
		for (int j = 2; j < 10; ++j) {
			if (j != 5) {
				*triple = Triple(s + j, p * j, n + j);
				++triple;

			}
		}
	}
}
//TODO: reduce memory footprint by specializing on 6
template<>
inline void populateWidth<2>() noexcept {
	auto* triple = getTriples<2>();
	for (int i = 2; i < 10; ++i) {
		if (i != 5) {
			auto numberOuter = makeDigitAt<1>(i);
			for (int j = 2; j < 10; ++j) {
				if (j != 5) {
					*triple = Triple(i + j, i * j, numberOuter + j);
					++triple;
				}
			}
		}
	}
}

template<u64 count, u64 addonWidth>
using ArrayView = std::array<Triple, count * addonWidth>;

#endif
