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
using u64 = uint64_t;
using u32 = uint32_t;

template<u64 length>
constexpr u64 quickPow10() noexcept {
	return quickPow10<length - 1>() * 10;
}
template<>
constexpr u64 quickPow10<0>() noexcept {
	return 1;
}

template<u64 length>
constexpr auto fastPow10 = quickPow10<length>();


template<typename T>
constexpr bool componentQuodigious(T value, T compare) noexcept {
	return (value % compare) == 0;
}
/**
 * This is used all over the place, it is the actual code to check to see if a
 * number is actuall quodigious. All of the work before hand is to get the sum
 * and products (if we get this far).
 */
template<typename T>
constexpr bool isQuodigious(T value, T sum, T product) noexcept {
	// more often than not, the sum is divisible by the original value, so
	// really that sort of check is useless. If we find that the product is
	// divisible first then we should eliminate numbers faster :D
	return componentQuodigious<T>(value, product) && componentQuodigious<T>(value, sum);
}


template<u64 times>
constexpr u64 multiply(u64 product) noexcept {
	return times * product;
}

template<u32 times>
constexpr u32 multiply(u32 product) noexcept {
    return times * product;
}

template<> constexpr u64 multiply<0>(u64 product) noexcept { return 0; }
template<> constexpr u64 multiply<1>(u64 product) noexcept { return product; }
template<> constexpr u64 multiply<2>(u64 product) noexcept { return product << 1; }
template<> constexpr u64 multiply<3>(u64 product) noexcept { return (product << 1) + product; }
template<> constexpr u64 multiply<4>(u64 product) noexcept { return (product << 2); }
template<> constexpr u64 multiply<5>(u64 product) noexcept { return (product << 2) + product; }
template<> constexpr u64 multiply<6>(u64 product) noexcept { return (product << 2) + (product << 1); }
template<> constexpr u64 multiply<7>(u64 product) noexcept { return (product << 2) + (product << 1) + product; }
template<> constexpr u64 multiply<8>(u64 product) noexcept { return (product << 3); }
template<> constexpr u64 multiply<9>(u64 product) noexcept { return (product << 3) + product; }
template<> constexpr u64 multiply<10>(u64 product) noexcept { return (product << 3) + (product << 1); }

template<> constexpr u32 multiply<0>(u32 product) noexcept { return 0; }
template<> constexpr u32 multiply<1>(u32 product) noexcept { return product; }
template<> constexpr u32 multiply<2>(u32 product) noexcept { return product << 1; }
template<> constexpr u32 multiply<3>(u32 product) noexcept { return (product << 1) + product; }
template<> constexpr u32 multiply<4>(u32 product) noexcept { return (product << 2); }
template<> constexpr u32 multiply<5>(u32 product) noexcept { return (product << 2) + product; }
template<> constexpr u32 multiply<6>(u32 product) noexcept { return (product << 2) + (product << 1); }
template<> constexpr u32 multiply<7>(u32 product) noexcept { return (product << 2) + (product << 1) + product; }
template<> constexpr u32 multiply<8>(u32 product) noexcept { return (product << 3); }
template<> constexpr u32 multiply<9>(u32 product) noexcept { return (product << 3) + product; }
template<> constexpr u32 multiply<10>(u32 product) noexcept { return (product << 3) + (product << 1); }

constexpr bool isEven(u64 value) noexcept {
    return (value & 1) == 0;
}

template<u64 k>
struct EvenCheck : std::integral_constant<bool, k == ((k >> 1) << 1)> { };

template<typename T>
inline void merge(T value , std::ostream& input) noexcept {
    if (value != 0) {
        input << value << std::endl;
    }
}

template<u64 width>
constexpr int numberOfDigitsForGivenWidth() noexcept {
	static_assert(width >= 0, "Negative width doesn't make sense");
	return 7 * numberOfDigitsForGivenWidth<width - 1>();
}
template<> constexpr int numberOfDigitsForGivenWidth<0>() noexcept { return 1; }
template<u64 width>
constexpr auto numElements = numberOfDigitsForGivenWidth<width>();

template<u64 width>
constexpr u64 makeDigitAt(u64 input) noexcept {
	return input * fastPow10<width>;
}

template<u64 width, u64 divide>
constexpr int getPartialSize() noexcept {
    static_assert(divide > 0, "Can't have a divisor of 0");
    return numElements<width> / divide;
}

template<u64 width, u64 divisible>
constexpr bool isDivisibleBy(u64 factor) noexcept {
    return (factor * divisible) == numElements<width>;
}
#endif // end QLIB_H__
