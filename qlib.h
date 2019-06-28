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

template<u64 base, u64 exponent>
constexpr u64 compileTimePow() noexcept {
    if constexpr (exponent == 0) {
        return 1;
    } else {
        return base * compileTimePow<base, exponent - 1>();
    }
}

template<u64 base, u64 exponent>
constexpr auto fastPow = compileTimePow<base, exponent>();

template<u64 length>
constexpr auto fastPow10 = fastPow<10, length>;

template<typename T>
constexpr bool componentQuodigious(T value, T compare) noexcept {
	return value >= compare ? ((value % compare) == 0) : false;
}
/**
 * This is used all over the place, it is the actual code to check to see if a
 * number is actuall quodigious. All of the work before hand is to get the sum
 * and products (if we get this far).
 */
constexpr bool isQuodigious(u64 value, u64 sum, u64 product) noexcept {
	return componentQuodigious<u64>(value, product) && componentQuodigious<u64>(value, sum);

}


constexpr bool isQuodigious(u32 value, u32 sum, u32 product) noexcept {
	return componentQuodigious<u32>(value, product) && componentQuodigious<u32>(value, sum);
}

/*
 * Order hashes are a unique design to describe the position of a given value
 * quickly, although extracting the values out requires some unpacking. The
 * value itself is actually a modification of octal. Each digit consumes three bits
 * and has the following meanings:
 *
 * 000 -> 2
 * 001 -> 3
 * 010 -> 4
 * 011 -> 5 (never used but there for completeness)
 * 100 -> 6
 * 101 -> 7
 * 110 -> 8
 * 111 -> 9
 *
 * A 64-bit number allows for 21 complete digits to be packed. Since we only
 * need a max of 19, we actually save space. It does mean that unpacking is a tad
 * more complex though.
 */

template<u64 index>
constexpr u64 encodeDigitIntoOrderHash(u64 value, u64 digit) noexcept {
    static_assert(index < 19, "Provided index is too large, range is 0->18");
    constexpr auto shiftcount = index * 3;
    constexpr auto bitmask = 0x7ul << shiftcount;
    auto valueToInject = digit - 2;
    auto maskedValue = value & ~bitmask;
    valueToInject <<= shiftcount;
    valueToInject &= bitmask;
    return maskedValue | valueToInject;
}

constexpr auto debugEnabled() noexcept {
#ifdef DEBUG
    return true;
#else
    return false;
#endif
}


#endif // end QLIB_H__
