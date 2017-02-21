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
using u64 = uint64_t;

template<u64 length>
inline constexpr u64 fastPow10() noexcept {
	return fastPow10<length - 1>() * 10;
}
template<>
inline constexpr u64 fastPow10<0>() noexcept {
	return 1;
}

/**
 * This is used all over the place, it is the actual code to check to see if a
 * number is actuall quodigious. All of the work before hand is to get the sum
 * and products (if we get this far).
 */
template<typename T>
inline constexpr bool isQuodigious(T value, T sum, T product) noexcept {
    return ((value % sum) == 0) && ((value % product) == 0);
}

/**
 * Represents the starting offset of any base number for any width!
 */
constexpr auto shaveFactor = 2.0 + (2.0 / 9.0);


using vec64 = std::vector<u64>;
template<u64 width>
struct NotationDescription {
	static constexpr u64 numberOfLevels = 3;
	static constexpr u64 level3Digits = (width - 1) / 2;
	static constexpr u64 level2Digits = (width - 1) - level3Digits; // whats left over?
	static constexpr u64 level1Digits = 1;
	static_assert(width == (level3Digits + level2Digits + level1Digits), "Not enough digits defined!");
};
template<u64 width>
inline constexpr u64 numberOfLevels() noexcept {
	return NotationDescription<width>::numberOfLevels;
}
template<u64 width>
inline constexpr u64 level4Digits() noexcept {
	return NotationDescription<width>::level4Digits;
}
template<> inline constexpr u64 level4Digits<1>() { return 0; }
template<> inline constexpr u64 level4Digits<2>() { return 0; }
template<> inline constexpr u64 level4Digits<3>() { return 0; }
template<u64 width>
inline constexpr u64 level3Digits() noexcept {
	return NotationDescription<width>::level3Digits;
}
template<u64 width>
inline constexpr u64 level2Digits() noexcept {
	return NotationDescription<width>::level2Digits;
}
template<u64 width>
inline constexpr u64 level1Digits() noexcept {
	return NotationDescription<width>::level1Digits;
}
#define Y(width, l4, l3, l2, l1) \
	template<> \
struct NotationDescription < width > { \
	static constexpr u64 numberOfLevels = 4; \
	static constexpr u64 level4Digits = l4; \
	static constexpr u64 level3Digits = l3 ; \
	static constexpr u64 level2Digits = l2 ; \
	static constexpr u64 level1Digits = l1 ; \
};
#define X(width, l3, l2, l1) \
	template<> \
struct NotationDescription< width > { \
	static constexpr u64 numberOfLevels = 3; \
	static constexpr u64 level3Digits = l3 ; \
	static constexpr u64 level2Digits = l2 ; \
	static constexpr u64 level1Digits = l1 ; \
};
#include "notations.def"
#undef X
#undef Y


#endif // end QLIB_H__
