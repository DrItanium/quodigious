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
#include <vector>
#include <tuple>
using u64 = uint64_t;

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
	return componentQuodigious<T>(value, sum) && componentQuodigious(value, product);
}



/**
 * Represents the starting offset of any base number for any width!
 */
constexpr auto shaveFactor = 2.0 + (2.0 / 9.0);


using vec64 = std::vector<u64>;

template<u64 width> struct Level3Digits : std::integral_constant<u64, (width - 1) / 2> { };
template<u64 width> struct Level2Digits : std::integral_constant<u64, (width - 1) - Level3Digits<width> { }> { };
template<u64 width> struct Level1Digits : std::integral_constant<u64, (width - 1) - Level2Digits<width> { }> { };

#define X(width, l3, l2, l1) \
template<> struct Level3Digits< width > : std::integral_constant<u64, l3 > { }; \
template<> struct Level2Digits< width > : std::integral_constant<u64, l2 > { }; \
template<> struct Level1Digits< width > : std::integral_constant<u64, l1 > { }; \
static_assert(width == (l3 + l2 + l1) , "Number of digits allocated != width of number!");
#include "notations.def"
#undef X

/**
 * Useful for getting information about a given number, not used during
 * quodigious computation because it gets really slow with larger numbers!
 */
template<u64 length>
std::tuple<u64, u64> digitSumAndProduct(u64 value) noexcept {
	static_assert(length != 0, "Can't have a length of zero!");
	static_assert(length < 20, "Maximum of 19 digits!");
	u64 current = value;
	u64 sum = 0u;
	u64 product = 1u;
	for(auto count = 0; count < length; ++count) {
		auto back = current % 10u;
		sum += back;
		product *= back;
		current /= 10u;
	}
	return std::tuple<u64, u64>(sum, product);
}

template<u64 times>
constexpr u64 multiply(u64 product) noexcept {
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

constexpr bool isEven(u64 value) noexcept {
    return (value & 1) == 0;
}

template<u64 k>
struct EvenCheck : std::integral_constant<bool, k == ((k >> 1) << 1)> { };

#endif // end QLIB_H__
