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
template<u64 position>
constexpr auto shiftAmount = position * 3;
template<u64 base, u64 pos>
constexpr auto computeFactor = base * fastPow10<pos>;
template<u64 position>
constexpr u64 convertNumber(u64 value) noexcept {
    static_assert(position > 0, "Can't access position 0!");
    if constexpr (position == 1) {
        return ((value & 0b111) + 2);
    } else {
		constexpr auto nextPos = position - 1;
		constexpr auto mask = 0b111ul << shiftAmount<nextPos>; 
		auto significand = (value & mask) >> shiftAmount<nextPos>;
        auto intermediate = 0ul;
        switch(significand) {
            case 0b000: intermediate = computeFactor<2ul, nextPos>; break;
            case 0b001: intermediate = computeFactor<3ul, nextPos>; break;
            case 0b010: intermediate = computeFactor<4ul, nextPos>; break;
            case 0b011: intermediate = computeFactor<5ul, nextPos>; break;
            case 0b100: intermediate = computeFactor<6ul, nextPos>; break;
            case 0b101: intermediate = computeFactor<7ul, nextPos>; break;
            case 0b110: intermediate = computeFactor<8ul, nextPos>; break;
            case 0b111: intermediate = computeFactor<9ul, nextPos>; break;
        }
        return intermediate + convertNumber<nextPos>(value);
    }
}
template<u64 position>
constexpr u64 getShiftedValue(u64 value) noexcept {
    return value << shiftAmount<position>;
}

#define SKIP5s(x) \
        if constexpr (length > 4) { \
            if (x == 3ul) { \
                continue; \
            } \
        }
#define onPosOrDiff(n) (position == n || difference == n)
template<u64 position, u64 length>
void body(MatchList& list, u64 sum = 0, u64 product = 1, u64 index = 0) noexcept {
    static_assert(length <= 19, "Can't have numbers over 19 digits on 64-bit numbers!");
    static_assert(length > 0, "Can't have length of zero!");
    static_assert(length >= position, "Position is out of bounds!");
    static constexpr auto difference = length - position;
    if constexpr (position == length) {
        if constexpr (length > 10) {
            // if the number is not divisible by three then skip it
            if (sum % 3 != 0) {
                return;
            }
        }
        if (auto conv = convertNumber<length>(index); (conv % product == 0) && (conv % sum == 0)) {
            list.emplace_back(conv);
        }
    } else if constexpr (length > 10 && difference == 2) {
        auto outerConverted = convertNumber<length>(index);
        // this will generate a partial number!
        for (auto i = 0ul; i < 8ul; ++i) {
            if (i == 3) {
                continue;
            }
            auto outerShiftI = outerConverted + (i * fastPow10<position>);
            auto innerShiftI = outerConverted + (i * fastPow10<position + 1>);
            auto os = sum + i;
            auto op = product * (i + 2);
            // start at I and work forward, if i and j are not the same then swap the digits and
            // try the sum and product with these two digits swapped
            for (auto j = i; j < 8ul; ++j) {
                if (j == 3) {
                    continue;
                }
                auto s = os + j;
                auto p = op * (j + 2);
                if (s % 3 != 0) {
                    continue;
                }
                if (auto n0 = outerShiftI + (j * fastPow10<position + 1>); (n0 % p == 0) && (n0 % s == 0)) {
                    list.emplace_back(n0);
                }
                if (i != j) {
                    if (auto n1 = innerShiftI + (j * fastPow10<position>); (n1 % p == 0) && (n1 % s == 0)) {
                        list.emplace_back(n1);
                    }
                }
            }
        }
    } else if constexpr (length > 10 && (position == 2 || onPosOrDiff(4) || onPosOrDiff(6))) {
        for (auto i = 0ul; i < 8ul; ++i) {
            SKIP5s(i);
            auto outerShiftI = index + getShiftedValue<position>(i);
            auto innerShiftI = index + getShiftedValue<position + 1>(i);
            auto os = sum + i;
            auto op = product * (i + 2);
            // start at I and work forward, if i and j are not the same then swap the digits and
            // try the sum and product with these two digits swapped
            for (auto j = i; j < 8ul; ++j) {
                SKIP5s(j);
                auto s = os + j;
                auto p = op * (j + 2);
                body<position + 2, length>(list, s, p, outerShiftI + getShiftedValue<position+1>(j));
                if (i != j) {
                    body<position + 2, length>(list, s, p, innerShiftI + getShiftedValue<position>(j));
                }
            }
        }
    } else {
        auto dprod = product << 1;
        static constexpr auto indexIncr = getShiftedValue<position>(1ul);
        for (auto i = 0ul; i < 8ul; ++i, ++sum, index += indexIncr) {
            SKIP5s(i);
            body<position + 1, length>(list, sum, dprod + (i * product), index);
        }
    }
}
#undef onPosOrDiff
#undef SKIP5s

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
    return list;
}

template<u64 width>
void initialBody() noexcept {
	MatchList list;
	if constexpr (width < 10) {
        body<0, width>(list, width * 2);
	} else {
        auto getnsplice = [&list](auto& thing) {
            auto r = thing.get();
            list.splice(list.cbegin(), r);
        };
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
        getnsplice(t0);
        getnsplice(t1);
        getnsplice(t2);
        getnsplice(t3);
        getnsplice(t4);
        getnsplice(t5);
        getnsplice(t6);
    } 
	list.sort();
	for (const auto& v : list) {
        std::cout << v << std::endl;
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
