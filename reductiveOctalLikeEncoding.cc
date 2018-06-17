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
template<u64 position, u64 length>
void body(MatchList& list, u64 sum = 0, u64 product = 1, u64 index = 0, u64 depth = 0) noexcept {
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
#define lenGreaterAndPos(len,pos) (length > len && position == pos)
    } else if constexpr (lenGreaterAndPos(10, 2) || lenGreaterAndPos(11, 3) || lenGreaterAndPos(12, 4) || lenGreaterAndPos(13, 5)) {
#undef lenGreaterAndPos
        // setup a series of operations to execute in parallel on two separate threads
        // of execution
	using PackedData = std::tuple<u64, u64, u64, u64>; 
        std::list<PackedData> lower, upper;
        auto dprod = product << 1;
        static constexpr auto indexIncr = getShiftedValue<position>(1ul);
        for (auto i = depth; i < 8ul; ++i, ++sum, index += indexIncr) {
            SKIP5s(i);
            auto tup = std::make_tuple(sum, dprod + (i * product), index, i);
            if (i < 3) {
                lower.emplace_back(tup);
            } else {
                upper.emplace_back(tup);
            }
        }
        auto halveIt = [](std::list<PackedData> & collection) {
            MatchList l;
            for(auto& a : collection) {
                body<position + 1, length>(l, std::get<0>(a), std::get<1>(a), std::get<2>(a), std::get<3>(a));
            }
            return l;
        };
        auto t0 = std::async(std::launch::async, halveIt, std::ref(lower));
        auto t1 = std::async(std::launch::async, halveIt, std::ref(upper));
        auto l0 = t0.get();
        list.splice(list.cbegin(), l0);
        auto l1 = t1.get();
        list.splice(list.cbegin(), l1);
    } else if constexpr (length > 10 && difference == 3) {
        // this will generate a partial number but reduce the number of conversions
        // required greatly!
        // The last two digits are handled in a base 10 fashion without the +2 added
        // This will make the partial converison correct (remember that a 0 becomes a 2
        // in this model).
        auto outerConverted = convertNumber<length>(index);
        static constexpr auto p10a = fastPow10<position>;
        static constexpr auto p10b = fastPow10<position+1>;
        static constexpr auto p10c = fastPow10<position+2>;
        for (auto a = depth; a < 8ul; ++a) {
            SKIP5s(a);
            auto a1 = outerConverted + (a * p10a);
            auto a2 = outerConverted + (a * p10b); 
            auto a3 = outerConverted + (a * p10c); 
            auto as = sum + a;
            auto ap = product * (a + 2);
            for (auto b = a; b < 8ul; ++b) {
                SKIP5s(b);
                auto b1 = b * p10a;
                auto b2 = b * p10b;
                auto b3 = b * p10c;
                auto bs = as + b;
                auto bp = ap * (b + 2);
                auto abdiff = a != b;
                for (auto c = b; c < 8ul; ++c) {
                    SKIP5s(c);
                    auto cs = bs + c;
                    if (cs % 3 != 0) {
                        continue;
                    }
                    auto c1 = c * p10a;
                    auto c2 = c * p10b;
                    auto c3 = c * p10c;
                    auto cp = bp * (c + 2);
#define bcheck(x) ((x % cp == 0) && (x % cs == 0))
#define ibody(x,y,z) if (auto n = x + y + z ; bcheck(n)) { list.emplace_back(n); }
                    // always do this one
                    ibody(a1,b2,c3);
                    if (abdiff) {
                       // a != b thus we can execute these two safely
                       //
                       ibody(b1,c2,a3);
                       ibody(c1,a2,b3);
                       if (b != c && a != c) {
                           ibody(a1,c2,b3);
                           ibody(b1,a2,c3);
                           ibody(c1,b2,a3);
                       }
                    } else if (b != c) {
                        // a == b && b != c -> a != c
                        // a == b in this case if we get here
                        ibody(b1,c2,a3);
                        ibody(c1,a2,b3);
                    }
                }
            }
        }
#undef bcheck
#undef ibody
    } else {
        auto dprod = product << 1;
        static constexpr auto indexIncr = getShiftedValue<position>(1ul);
        for (auto i = depth; i < 8ul; ++i, ++sum, index += indexIncr) {
            SKIP5s(i);
            body<position + 1, length>(list, sum, dprod + (i * product), index, i);
        }
    }
}
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
