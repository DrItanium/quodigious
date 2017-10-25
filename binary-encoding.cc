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

// Perform 32-bit numeric quodigious checks
#include "qlib.h"

template<u32 length>
inline void innerBody(u32 sum, u32 product, u32 index) noexcept;

template<u32 length>
inline void body(u32 sum = 0, u32 product = 1, u32 index = 0) noexcept {
    static_assert(length <= 9, "Can't have numbers over 9 digits on 32-bit numbers!");
    static_assert(length != 0, "Can't have length of zero!");
    constexpr auto inner = length - 1;
    constexpr auto next = fastPow10<inner>;
    auto baseProduct = product;
    sum += 2;
    product <<= 1;
    index += multiply<2>(next);
	for (int i = 2; i < 10; ++i) {
		if (i != 5) {
			// skip the fives digit
			innerBody<inner>(sum, product, index);
		}
		++sum;
		product += baseProduct;
		index += next;
	}
}

template<u32 length>
inline void innerBody(u32 sum, u32 product, u32 index) noexcept {
    body<length>(sum, product, index);
}

template<>
inline void innerBody<0>(u32 sum, u32 product, u32 index) noexcept {
	union {
		u32 value;
		uint8_t bytes[sizeof(u32)];
	} tmp;
	tmp.value = index;
	for (int i = 0; i < sizeof(u32); ++i) {
		std::cout.put(tmp.bytes[i]);
	}
	tmp.value = sum;
	for (int i = 0; i < sizeof(u32); ++i) {
		std::cout.put(tmp.bytes[i]);
	}
	tmp.value = product;
	for (int i = 0; i < sizeof(u32); ++i) {
		std::cout.put(tmp.bytes[i]);
	}
}

int main() {
	// generate a binary encoding that we can save to a file and load as needed!
	body<8>();
    return 0;
}
