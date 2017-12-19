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

template<u64 position>
constexpr u64 encodeDigit(u64 value, u64 digit) noexcept {
	static_assert(position <= 19, "Cannot encode digit at position 20 or more!");
	constexpr auto shift = position * 3;
	constexpr auto mask = 0b111ul << shift;
	return (value & ~mask) | (digit << shift);
}
template<u64 position>
constexpr u64 extractDigit(u64 value) noexcept {
	static_assert(position <= 19, "Cannot extract digit of position 20 or more!");
	constexpr auto shift = position * 3;
	return (value >> shift) & 0b111;
}

template<u64 position, u64 length>
struct SpecialWalker {
	SpecialWalker() = delete;
	~SpecialWalker() = delete;
	SpecialWalker(SpecialWalker&&) = delete;
	SpecialWalker(const SpecialWalker&) = delete;
	static void body(u64 sum = 0, u64 product = 1, u64 index = 0) noexcept {
		static_assert(length <= 19, "Can't have numbers over 19 digits on 64-bit numbers!");
		static_assert(length != 0, "Can't have length of zero!");
		SpecialWalker<position + 1, length>::body(sum + 2, product * 2, index);
		for (auto i = 3; i < 10; ++i) {
			if (length > 4 && i == 5) { 
				continue;
			}
			SpecialWalker<position + 1, length>::body(sum + i, product * i, encodeDigit<position>(index, (i - 2)));
		}
	}
};

template<u64 position>
constexpr u64 convertNumber(u64 value) noexcept {
	return (fastPow10<position - 1> * (extractDigit<position - 1>(value) + 2)) + convertNumber<position - 1>(value);
}

template<>
constexpr u64 convertNumber<1>(u64 value) noexcept { 
	return ((value & 0b111) + 2);
}

template<u64 length>
struct SpecialWalker<length, length> {
	SpecialWalker() = delete;
	~SpecialWalker() = delete;
	SpecialWalker(SpecialWalker&&) = delete;
	SpecialWalker(const SpecialWalker&) = delete;
	static void body(u64 sum = 0, u64 product = 1, u64 index = 0) noexcept {
		auto conv = convertNumber<length>(index);
		if ((conv % product == 0) && (conv % sum == 0)) {
			std::cout << conv << '\n';
		}
	}
};

template<u64 width>
void initialBody() noexcept {
	if (width == 1) {
		std::cout << "2\n3\n4\n5\n6\n7\n8\n9" << std::endl;
	} else if (width > 4) {
		for (auto i = 2; i < 10; i += 2) {
			SpecialWalker<1, width>::body(i, i, i - 2);
		}
	} else {
		SpecialWalker<0, width>::body();
	}
}

int main() {
	while(std::cin.good()) {
		u64 currentIndex = 0;
		std::cin >> currentIndex;
		if (std::cin.good()) {
			switch(currentIndex) {
				case 1: initialBody<1>(); break;
				case 2: initialBody<2>(); break;
				case 3: initialBody<3>(); break;
				case 4: initialBody<4>(); break;
				case 5: initialBody<5>(); break;
				case 6: initialBody<6>(); break;
				case 7: initialBody<7>(); break;
				case 8: initialBody<8>(); break;
				case 9: initialBody<9>(); break;
				case 10: initialBody<10>(); break;
				case 11: initialBody<11>(); break;
				case 12: initialBody<12>(); break;
				case 13: initialBody<13>(); break;
				case 14: initialBody<14>(); break;
				case 15: initialBody<15>(); break;
				case 16: initialBody<16>(); break;
				case 17: initialBody<17>(); break;
				case 18: initialBody<18>(); break;
				case 19: initialBody<19>(); break;
				default:
						 std::cerr << "Illegal index " << currentIndex << std::endl;
						 return 1;
			}
			std::cout << std::endl;
		}
	}
	return 0;
}
