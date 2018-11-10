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
template<u64 len, u64 pos>
constexpr bool lenGreaterAndPos(u64 length, u64 position) noexcept {
	return (length > len) && (position == pos);
}
void tryInsertIntoList(u64 value, std::list<u64>& l) noexcept {
#ifdef DEBUG
	if (auto it = std::find(l.begin(), l.end(), value); it != l.end()) {
		std::cout << "Duplicate value: " << value << std::endl;
	} else {
		l.emplace_back(value);
	}
#else
		l.emplace_back(value);
#endif
}
template<u64 position, u64 length>
void body(MatchList& list, u64 sum = 0, u64 product = 1, u64 index = 0) noexcept {
    static_assert(length <= 19, "Can't have numbers over 19 digits on 64-bit numbers!");
    static_assert(length > 0, "Can't have length of zero!");
    static_assert(length >= position, "Position is out of bounds!");
	using DataTriple = std::tuple<u64, u64, u64>;
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
    } else if constexpr (lenGreaterAndPos<10, 2>(length, position) || 
			             lenGreaterAndPos<11, 3>(length, position) || 
						 lenGreaterAndPos<12, 4>(length, position) || 
						 lenGreaterAndPos<13, 5>(length, position)) {
        // setup a series of operations to execute in parallel on two separate threads
        // of execution
        std::list<DataTriple> lower, upper;
        auto dprod = product << 1;
        static constexpr auto indexIncr = getShiftedValue<position>(1ul);
        for (auto i = 0ul; i < 8ul; ++i, ++sum, index += indexIncr) {
            SKIP5s(i);
            auto tup = std::make_tuple(sum, dprod + (i * product), index);
            if (i < 3) {
                lower.emplace_back(tup);
            } else {
                upper.emplace_back(tup);
            }
        }
        auto halveIt = [](std::list<DataTriple> & collection) {
            MatchList l;
            for(auto& a : collection) {
                body<position + 1, length>(l, std::get<0>(a), std::get<1>(a), std::get<2>(a));
            }
            return l;
        };
        auto t0 = std::async(std::launch::async, halveIt, std::ref(lower));
        auto t1 = std::async(std::launch::async, halveIt, std::ref(upper));
        auto l0 = t0.get();
        auto l1 = t1.get();
        list.splice(list.cbegin(), l0);
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
        for (auto a = 0ul; a < 8ul; ++a) {
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
				auto ab12comb = a1 + b2;
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
#define X(x,y,z) if (auto n = x + y + z ; bcheck(n)) { list.emplace_back(n); }
                    // always do this one
                    X(a1,b2,c3);
                    if (abdiff) {
                       // a != b thus we can execute these two safely
                       //
                       X(b1,c2,a3);
                       X(c1,a2,b3);
                       if (b != c && a != c) {
                           X(a1,c2,b3);
                           X(b1,a2,c3);
                           X(c1,b2,a3);
                       }
                    } else if (b != c) {
                        // a == b && b != c -> a != c
                        // a == b in this case if we get here
                        X(b1,c2,a3);
                        X(c1,a2,b3);
                    }
                }
            }
        }
#undef bcheck
#undef X
    } else if constexpr (length > 10 && difference == 4) {
        // this will generate a partial number but reduce the number of conversions
        // required greatly!
        // The last two digits are handled in a base 10 fashion without the +2 added
        // This will make the partial converison correct (remember that a 0 becomes a 2
        // in this model).
        auto outerConverted = convertNumber<length>(index);
        static constexpr auto p10a = fastPow10<position>;
        static constexpr auto p10b = fastPow10<position+1>;
        static constexpr auto p10c = fastPow10<position+2>;
		static constexpr auto p10d = fastPow10<position+3>;
        for (auto a = 0ul; a < 8ul; ++a) {
            SKIP5s(a);
            auto a1 = outerConverted + (a * p10a);
            auto a2 = outerConverted + (a * p10b); 
            auto a3 = outerConverted + (a * p10c); 
			auto a4 = outerConverted + (a * p10d);
            auto as = sum + a;
            auto ap = product * (a + 2);
            for (auto b = a; b < 8ul; ++b) {
                SKIP5s(b);
                auto b1 = b * p10a;
                auto b2 = b * p10b;
                auto b3 = b * p10c;
				auto b4 = b * p10d;
                auto bs = as + b;
                auto bp = ap * (b + 2);
				auto absame = a == b;
				auto ab12comb = a1 + b2;
                for (auto c = b; c < 8ul; ++c) {
                    SKIP5s(c);
                    auto cs = bs + c;
                    auto c1 = c * p10a;
                    auto c2 = c * p10b;
                    auto c3 = c * p10c;
					auto c4 = c * p10d;
					auto bcsame = b == c;
					auto acsame = a == c;
                    auto cp = bp * (c + 2);
					for (auto d = c; d < 8ul; ++d) {
						SKIP5s(d);
						auto ds = cs + d;
						auto d1 = d * p10a;
						auto d2 = d * p10b;
						auto d3 = d * p10c;
						auto d4 = d * p10d;
						auto dp = cp * (d + 2);
						if (ds % 3 != 0) {
							continue;
						}
#define bcheck(x) ((x % dp == 0) && (x % ds == 0))
#define X(x,y,z,w) if (auto n = x ## 1 + y ## 2 + z ## 3 + w ## 4; bcheck(n)) { tryInsertIntoList(n, list); }
						// always do this one
						// output all combinations first
						X(a,b,c,d);
						if (c != d) {
							X(a,b,d,c);
						}
						if (absame) {
							if (c != d) {
								X(d,a,b,c);
								X(a,d,b,c);
							}
							if (!bcsame) {
								// a == b && b != c 
								X(a,c,b,d);
								X(c,a,b,d);
								if (a != d) {
									X(c,d,a,b);
									X(a,c,d,b);
									X(c,a,d,b);
									if (c != d) {
										// a == b && b != c && c != d
										// Thus a != c but a == d ?
										X(a,d,c,b);
										X(d,a,c,b);
										X(d,c,a,b);
									}
								}
							}
						} else {
							X(b,a,c,d);
							if (c != d) {
								X(b,a,d,c);
							}
							if (bcsame) {
								X(b,c,a,d);
								if (a != d) {
									X(b,c,d,a);
								}
								// a != b && b == c
								if (c != d) {
									// a != b && b == c && c != d -> b != d
									X(a,d,b,c);
									if (a != d) {
										X(b,d,c,a);
										X(b,d,a,c);
										X(d,a,b,c);
										X(d,b,a,c);
										X(d,b,c,a);
									}
								}
							} else {
								// a != b && b != c
								X(a,c,b,d);
								if (b != d) {
									X(a,c,d,b);
								}
								if (acsame) {
									// a != b && b != c && a == c && c != d ->
									// a != d
									if (c != d) {
										// a != b && b != c && a == c
										// c == d || b == d || (c != d && b != d)
										X(a,d,c,b);
										X(b,c,a,d);
										X(b,c,d,a);
										X(b,d,a,c);
										if (b != d) {
											X(a,d,b,c);
											X(d,a,b,c);
											X(d,a,c,b);
											X(d,b,a,c);
										}
									}
								} else {
									// a != b && b !=c && a != c
									// therefore a == d || b == d || c == d ||
									// a != d && b != d && c != d
									X(b,c,a,d);
									X(c,a,b,d);
									X(c,b,a,d);
									if (b != d) {
										X(c,a,d,b);
									}
									if (a != d) {
										X(b,c,d,a);
										X(c,b,d,a);
									}
									if (a == d) {
										X(a,d,c,b);
										X(a,d,b,c);
									} else if (b == d) {
										X(b,d,c,a);
										X(b,d,a,c);
									} else if (c == d) {
										X(c,d,b,a);
										X(c,d,a,b);
									} else {
										X(a,d,c,b);
										X(a,d,b,c);
										X(b,d,c,a);
										X(b,d,a,c);
										X(c,d,b,a);
										X(c,d,a,b);
										X(d,a,b,c);
										X(d,a,c,b);
										X(d,b,a,c);
										X(d,b,c,a);
										X(d,c,b,a);
										X(d,c,a,b);
									}
								}
							}
						}
					}
                }
            }
        }
#undef bcheck
#undef X
    } else {
        auto dprod = product << 1;
        static constexpr auto indexIncr = getShiftedValue<position>(1ul);
        for (auto i = 0ul; i < 8ul; ++i, ++sum, index += indexIncr) {
            SKIP5s(i);
            body<position + 1, length>(list, sum, dprod + (i * product), index);
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
