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
constexpr bool isQuodigious(u64 value, u64 sum, u64 product) noexcept {
	// more often than not, the sum is divisible by the original value, so
	// really that sort of check is useless. If we find that the product is
	// divisible first then we should eliminate numbers faster :D
#ifndef EXACT_COMPUTATION
    // while not exact (there are some products which are not divisble by three
    // it will give an approximation that we can use to our advantage!
	return ((sum % 3 == 0)) && (product % 3 == 0) && componentQuodigious<u64>(value, product) && componentQuodigious<u64>(value, sum);
#else /* !EXACT_COMPUTATION */
	return componentQuodigious<u64>(value, product) && componentQuodigious<u64>(value, sum);
#endif // end EXACT_COMPUTATION

}


constexpr bool isQuodigious(u32 value, u32 sum, u32 product) noexcept {
	return componentQuodigious<u32>(value, product) && componentQuodigious<u32>(value, sum);
}



#endif // end QLIB_H__
