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

// Perform numeric quodigious checks using a special encoding.
//
// This special encoding uses the concept of octal but subtracts two from each
// digit and uses three bits to represent each digit separately. Printing out
// the number would show garbage in base ten or any normal number. For
// instance, the number '0' in the encoding is 2. 00 is 22, 000 is 222 and so
// on. 7 is 9, 77 is 99 and so forth. There is no way to represent one or zero
// in the encoding so it is perfect for this design.
// decimal would be
#include "qlib.h"
#include <future>

template<u32 position>
constexpr u32 encodeDigit(u32 value, u32 digit) noexcept {
	constexpr auto shift = position * 3;
	constexpr auto mask = 0b111 << shift;
	return (value & ~mask) | (mask & (digit << shift));
}
template<u32 position>
constexpr u32 extractDigit(u32 value) noexcept {
	constexpr auto shift = position * 3;
	return (value >> shift) & 0b111;
}
template<u32 position, u32 length>
struct SpecialWalker {
	static void body32(u32 sum = 0, u32 product = 1, u32 index = 0) noexcept {
		static_assert(length <= 9, "Can't have numbers over 9 digits on 32-bit numbers!");
		static_assert(length != 0, "Can't have length of zero!");
		// unlike the 64-bit version of this code, doing the 32-bit ints for 9 digit
		// numbers (this code is not used when you request 64-bit numbers!)
		// does not require as much optimization. We can walk through digit level
		// by digit level (even if the digit does not contribute too much to the
		// overall process!).
		for (auto i = 0; i < 8; ++i) {
			auto mod = i + 2;
			SpecialWalker<position - 1, length>::body32(sum + mod, product * mod, encodeDigit<position - 1>(index, i));
		}
	}
};

template<u32 position>
constexpr u32 convertNumber(u32 value) noexcept {
	return convertNumber<position - 1>(value) + (fastPow10<position - 1> * (extractDigit<position - 1>(value) + 2));
}
template<>
constexpr u32 convertNumber<0>(u32 value) noexcept { 
	return 0; 
}

template<u32 length>
struct SpecialWalker<0, length> {
	static void body32(u32 sum = 0, u32 product = 1, u32 index = 0) noexcept {
		auto conv = convertNumber<length>(index);
		if ((conv % product == 0) && (conv % sum == 0)) {
			std::cout << conv << '\n';
		}
	}
};

template<u32 width>
void initialBody32() noexcept {
	SpecialWalker<width, width>::body32();
}

int main() {
	while(std::cin.good()) {
		u64 currentIndex = 0;
		std::cin >> currentIndex;
		if (std::cin.good()) {
			switch(currentIndex) {
				case 1: initialBody32<1>(); break;
				case 2: initialBody32<2>(); break;
				case 3: initialBody32<3>(); break;
				case 4: initialBody32<4>(); break;
				case 5: initialBody32<5>(); break;
				case 6: initialBody32<6>(); break;
				case 7: initialBody32<7>(); break;
				case 8: initialBody32<8>(); break;
				case 9: initialBody32<9>(); break;
				default:
						 std::cerr << "Illegal index " << currentIndex << std::endl;
						 return 1;
			}
			std::cout << std::endl;
		}
	}
	return 0;
}
