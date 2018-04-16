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
#include <list>

using MatchList = std::list<u64>;
constexpr bool doNotStoreConvertedNumbers = false;
template<u64 position>
constexpr auto shiftAmount = position * 3;
template<u64 position>
constexpr u64 convertNumber(u64 value) noexcept {
    if constexpr (position == 1) {
        return ((value & 0b111) + 2);
    } else {
		constexpr auto nextPos = position - 1;
		constexpr auto mask = 0b111ul << shiftAmount<nextPos>; 
		constexpr auto factor = fastPow10<nextPos>;
		auto significand = (value & mask) >> shiftAmount<nextPos>;
        auto intermediate = 0ul;
        switch(significand) {
            case 0b000: intermediate = (2ul * factor); break;
            case 0b001: intermediate = (3ul * factor); break;
            case 0b010: intermediate = (4ul * factor); break;
            case 0b011: intermediate = (5ul * factor); break;
            case 0b100: intermediate = (6ul * factor); break;
            case 0b101: intermediate = (7ul * factor); break;
            case 0b110: intermediate = (8ul * factor); break;
            case 0b111: intermediate = (9ul * factor); break;
        }
        return intermediate + convertNumber<nextPos>(value);
    }
}


template<u64 position, u64 length>
void body(MatchList& list, u64 sum = 0, u64 product = 1, u64 index = 0) noexcept {
    static_assert(length <= 19, "Can't have numbers over 19 digits on 64-bit numbers!");
    static_assert(length != 0, "Can't have length of zero!");
    if constexpr (position == length) {
        if constexpr (length > 10) {
            if (sum % 3 != 0) {
                return;
            }
        }
        if (auto conv = convertNumber<length>(index); (conv % product == 0) && (conv % sum == 0)) {
			if constexpr (doNotStoreConvertedNumbers) {
				list.emplace_back(index);
			} else {
            	list.emplace_back(conv);
			}
        }
    } else {
        for (auto i = 0ul; i < 8ul; ++i) {
            if constexpr (length > 4) {
                if (i == 3ul) {
                    continue;
                }
            }
            body<position + 1, length>(list, sum + i, (product << 1) + (product * i), index + (i << shiftAmount<position>));
        }
    }
}

template<auto width>
MatchList parallelBody(u64 base) noexcept {
    MatchList list;
	auto start = (base - 2ul);
	auto index = start << 3;
	static constexpr auto addon = width << 1;
	// using the frequency analysis I did before for loops64.cc I found
	// that on even digits that 4 and 8 are used while odd digits use 2
	// and 6. This is a frequency analysis job only :D
	for (auto i = ((base % 2ul == 0) ? 4ul : 2ul); i < 10ul; i += 4ul) {
		auto j = i - 2ul;
		body<2, width>(list, start + j + addon, base * i, index + j);
	}
    list.sort();
    return list;
}

template<u64 width>
void initialBody() noexcept {
	MatchList list;
	if constexpr (width < 10) {
        body<0, width>(list, width * 2);
	} else {
        auto mkfuture = [](auto base) {
            return std::async(std::launch::async, parallelBody<width>, base);
        };
        auto t0 = mkfuture(2);
        auto t1 = mkfuture(3);
        auto t2 = mkfuture(4);
        auto t3 = mkfuture(6);
        auto t4 = mkfuture(7);
        auto t5 = mkfuture(8);
        auto t6 = mkfuture(9);
		auto r0 = t0.get();
		list.splice(list.cbegin(), r0);
		auto r1 = t1.get();
		list.splice(list.cbegin(), r1);
		auto r2 = t2.get();
		list.splice(list.cbegin(), r2);
		auto r3 = t3.get();
		list.splice(list.cbegin(), r3);
		auto r4 = t4.get();
		list.splice(list.cbegin(), r4);
		auto r5 = t5.get();
		list.splice(list.cbegin(), r5);
		auto r6 = t6.get();
		list.splice(list.cbegin(), r6);
    } 
	list.sort();
	for (const auto& v : list) {
        if constexpr ( doNotStoreConvertedNumbers) {
            constexpr auto mask = 0b111ul << shiftAmount<width >;
            for (int i = width - 1; i >= 0; --i) {
                std::cout << ((v & (0b111ul << (3 * i))) >> (3 * i));
            }
            std::cout << std::endl;
        } else {
            std::cout << v << std::endl;
        }
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
