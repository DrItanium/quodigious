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
struct CompileTimeExponentiation final {
    CompileTimeExponentiation() = delete;
    CompileTimeExponentiation(const CompileTimeExponentiation&) = delete;
    CompileTimeExponentiation(CompileTimeExponentiation&&) = delete;
    ~CompileTimeExponentiation() = delete;
    static constexpr u64 operation() noexcept {
        return base * CompileTimeExponentiation<base, exponent - 1>::operation();
    }
};

template<u64 base>
struct CompileTimeExponentiation<base, 0> final {
    CompileTimeExponentiation() = delete;
    CompileTimeExponentiation(const CompileTimeExponentiation&) = delete;
    CompileTimeExponentiation(CompileTimeExponentiation&&) = delete;
    ~CompileTimeExponentiation() = delete;
    static constexpr u64 operation() noexcept {
        return 1;
    }
};


template<u64 base, u64 exponent>
constexpr u64 compileTimePow() noexcept {
    return CompileTimeExponentiation<base, exponent>::operation();
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

template<typename T, typename F, T bitmask, T shiftcount>
constexpr T encodeBits(T input, F value) noexcept {
    auto valueToInject = static_cast<T>(value);
    auto maskedValue = input & ~bitmask;
    if (shiftcount != 0) {
        valueToInject <<= shiftcount;
    }
    valueToInject &= bitmask;
    return static_cast<T>(maskedValue | valueToInject);
}

constexpr u64 encodeOrderBit(u64 digit) noexcept {
    return digit - 2;
}

template<u64 position>
constexpr u64 encodeOctalBit(u64 input, u64 value) noexcept {
    return encodeBits<u64, u64, 0x7ul << (position * 3), position * 3>(input, encodeOrderBit(value));
}


template<u64 index>
constexpr u64 encodeDigitIntoOrderHash(u64 value, u64 digit) noexcept {
    static_assert(index < 19, "Provided index is too large, range is 0->18");
    switch(index) {
        case 0: return encodeOctalBit<0>(value, digit);
        case 1: return encodeOctalBit<1>(value, digit);
        case 2: return encodeOctalBit<2>(value, digit);
        case 3: return encodeOctalBit<3>(value, digit);
        case 4: return encodeOctalBit<4>(value, digit);
        case 5: return encodeOctalBit<5>(value, digit);
        case 6: return encodeOctalBit<6>(value, digit);
        case 7: return encodeOctalBit<7>(value, digit);
        case 8: return encodeOctalBit<8>(value, digit);
        case 9: return encodeOctalBit<9>(value, digit);
        case 10: return encodeOctalBit<10>(value, digit);
        case 11: return encodeOctalBit<11>(value, digit);
        case 12: return encodeOctalBit<12>(value, digit);
        case 13: return encodeOctalBit<13>(value, digit);
        case 14: return encodeOctalBit<14>(value, digit);
        case 15: return encodeOctalBit<15>(value, digit);
        case 16: return encodeOctalBit<16>(value, digit);
        case 17: return encodeOctalBit<17>(value, digit);
        case 18: return encodeOctalBit<18>(value, digit);
        default: return 0;
    }
}

#endif // end QLIB_H__
