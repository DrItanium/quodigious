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
template<u64 len, u64 pos, u64 length, u64 position>
constexpr auto lenGreaterAndPos = (length > len) && (position == pos);
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

template<typename T, typename ... Args>
void tryInsertIntoList(MatchList& list, u64 sum, u64 product, T value, Args ... args) noexcept {
    if ((value % product == 0) && (value % sum == 0)) {
        list.emplace_back(value);
    }
    if constexpr (sizeof...(args) > 0) {
        tryInsertIntoList<Args...>(list, sum, product, args...);
    }
}

template<u64 position, u64 length>
void body(MatchList& list, u64 sum = 0, u64 product = 1, u64 index = 0) noexcept {
	static_assert(length <= 19, "Can't have numbers over 19 digits on 64-bit numbers!");
	static_assert(length > 0, "Can't have length of zero!");
	static_assert(length >= position, "Position is out of bounds!");
	using DataTriple = std::tuple<u64, u64, u64>;
	static constexpr auto difference = length - position;
	static constexpr auto p10a = fastPow10<position>;
	static constexpr auto p10b = fastPow10<position+1>;
	static constexpr auto p10c = fastPow10<position+2>;
	static constexpr auto p10d = fastPow10<position+3>;
	static constexpr auto p10e = fastPow10<position+4>;
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
	} else if constexpr (lenGreaterAndPos<10, 2,length, position> || 
			lenGreaterAndPos<11, 3, length, position> || 
			lenGreaterAndPos<12, 4, length, position> || 
			lenGreaterAndPos<13, 5, length, position>) {
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
	} else if constexpr (length > 10 && difference == 5) {
        
		// this will generate a partial number but reduce the number of conversions
		// required greatly!
		// The last two digits are handled in a base 10 fashion without the +2 added
		// This will make the partial converison correct (remember that a 0 becomes a 2
		// in this model).
        //
        // Thus we implicitly add the offsets for each position to this base10 2 value :D
		auto outerConverted = convertNumber<length>(index);
#define X(x,y,z,w,h) if (auto n = x ## 1 + y ## 2 + z ## 3 + w ## 4 + h ## 5; ((n % ep == 0) && (n % es == 0))) { tryInsertIntoList(n, list); }
#define Y(x,y,z,w,h) x ## 1 + y ## 2 + z ## 3 + w ## 4 + h ## 5
		for (auto a = 0ul; a < 8ul; ++a) {
			SKIP5s(a);
			auto a1 = outerConverted + (a * p10a);
			auto a2 = a * p10b;
			auto a3 = a * p10c;
			auto a4 = a * p10d;
			auto a5 = a * p10e;
			auto as = sum + a;
			auto ap = product * (a + 2);
			for (auto b = a; b < 8ul; ++b) {
				SKIP5s(b);
				auto b1 = outerConverted + (b * p10a);
				auto b2 = b * p10b;
				auto b3 = b * p10c;
				auto b4 = b * p10d;
				auto b5 = b * p10e;
				auto bs = as + b;
				auto bp = ap * (b + 2);
				if (a == b) {
					for (auto c = b; c < 8ul; ++c) {
						SKIP5s(c);
						auto cs = bs + c;
						auto c1 = outerConverted + (c * p10a);
						auto c2 = c * p10b;
						auto c3 = c * p10c;
						auto c4 = c * p10d;
						auto c5 = c * p10e;
						auto cp = bp * (c + 2);
						if (b == c) {
							for (auto d = c; d < 8ul; ++d) {
								SKIP5s(d);
								auto ds = cs + d;
								auto dp = cp * (d + 2);
								auto d1 = outerConverted + (d * p10a);
								auto d2 = d * p10b;
								auto d3 = d * p10c;
								auto d4 = d * p10d;
								auto d5 = d * p10e;
								if (c == d) {
									for (auto e = d; e < 8ul; ++e) {
										SKIP5s(e);
										auto es = ds + e;
										if (es % 3 != 0) {
											continue;
										}
										auto ep = dp * (e + 2);
										auto e5 = e * p10e;
										X(a,b,c,d,e);
										if (d != e) {
											auto e1 = outerConverted + (e * p10a);
											auto e2 = e * p10b;
											auto e3 = e * p10c;
											auto e4 = e * p10d;
                                            tryInsertIntoList(list, es, ep,
                                                    Y(e,d,d,d,d),
                                                    Y(d,e,c,c,c), 
                                                    Y(d,c,e,c,c), 
                                                    Y(d,c,c,e,c));
										}
									}
								} else {
									for (auto e = d; e < 8ul; ++e) {
										SKIP5s(e);
										auto es = ds + e;
										if (es % 3 != 0) {
											continue;
										}
										auto ep = dp * (e + 2);
										auto e1 = outerConverted + (e * p10a);
										auto e2 = e * p10b;
										auto e3 = e * p10c;
										auto e5 = e * p10e;
                                        tryInsertIntoList(list, es, ep,
                                                Y(e,d,c,c,c), 
                                                Y(e,c,d,c,c), 
                                                Y(e,c,c,d,c),
                                                Y(e,c,c,c,d), 
                                                Y(a,b,c,d,e),
                                                Y(c,e,d,c,c), 
                                                Y(c,e,c,d,c), 
                                                Y(c,e,c,c,d), 
                                                Y(c,c,e,d,c), 
                                                Y(c,c,e,c,d));
										if (d != e) {
											auto e4 = e * p10d;
                                            tryInsertIntoList(list, es, ep,
											Y(d,e,c,c,c), 
											Y(d,c,e,c,c), 
											Y(d,c,c,e,c), 
											Y(d,c,c,c,e), 
											Y(c,d,e,c,c), 
											Y(c,d,c,e,c), 
											Y(c,d,c,c,e), 
											Y(c,c,d,e,c), 
											Y(c,c,d,c,e), 
											Y(c,c,c,e,d)) ;
										}
									}
								}
							}
						} else {
							for (auto d = c; d < 8ul; ++d) {
								SKIP5s(d);
								auto ds = cs + d;
								auto dp = cp * (d + 2);
								auto d1 = outerConverted + (d * p10a);
								auto d2 = d * p10b;
								auto d3 = d * p10c;
								auto d4 = d * p10d;
								auto d5 = d * p10e;
								if (c == d) {
									for (auto e = d; e < 8ul; ++e) {
										SKIP5s(e);
										auto es = ds + e;
										if (es % 3 != 0) {
											continue;
										}
										auto ep = dp * (e + 2);
										auto e1 = outerConverted + (e * p10a);
										X(e,d,c,b,b); X(e,d,b,c,b); X(e,d,b,b,c); 
										X(e,b,b,d,c); X(e,b,d,c,b); X(e,b,d,b,c); 
										auto e2 = e * p10b;
										X(b,e,b,d,c); X(b,e,d,c,b); X(b,e,c,b,d); 
										auto e5 = e * p10e;
										X(a,b,c,d,e); 
										if (d != e) {
											auto e3 = e * p10c;
											auto e4 = e * p10d;
											X(d,e,c,b,b); X(d,e,b,c,b); X(d,e,b,b,c);
											X(d,c,e,b,b); X(d,c,b,e,b); X(d,c,b,b,e);
											X(d,b,e,c,b); X(d,b,e,b,c); X(d,b,c,e,b);
											X(d,b,c,b,e); X(d,b,b,e,c); X(d,b,b,c,e);
											X(b,d,e,c,b); X(b,d,e,b,c); X(b,d,c,e,b);
											X(b,d,c,b,e); X(b,d,b,e,c); X(b,d,b,c,e);
											X(b,b,e,d,d); X(b,b,d,e,d); 
										}
									}
								} else {
									for (auto e = d; e < 8ul; ++e) {
										SKIP5s(e);
										auto es = ds + e;
										if (es % 3 != 0) {
											continue;
										}
										auto ep = dp * (e + 2);
										auto e1 = outerConverted + (e * p10a);
										X(e,d,c,b,b); X(e,d,b,c,b); X(e,d,b,b,c); 
										X(e,b,b,d,c); X(e,b,d,c,b); X(e,b,d,b,c); 
										X(e,c,d,b,b); X(e,c,b,d,b); X(e,c,b,b,d); 
										X(e,b,c,d,b); X(e,b,c,b,d); X(e,b,b,c,d);
										auto e2 = e * p10b;
										auto e3 = e * p10c;
										auto e4 = e * p10d;
										auto e5 = e * p10e;
										X(a,b,c,d,e); 
										X(b,e,d,c,b); X(b,e,c,b,d); X(b,e,b,d,c); 
										X(c,e,d,b,b); X(c,e,b,d,b); X(c,e,b,b,d);
										X(c,b,e,d,b); X(c,b,e,b,d); X(c,b,b,d,e);

										X(b,e,d,b,c); X(b,e,c,d,b); X(b,e,b,c,d);
										X(b,c,e,d,b); X(b,c,e,b,d); X(b,c,b,e,d); 
										X(b,b,e,d,c); X(b,b,e,c,d); 
										if (d != e) {
											X(d,e,c,b,b); X(d,e,b,c,b); X(d,e,b,b,c);
											X(d,c,e,b,b); X(d,c,b,e,b); X(d,c,b,b,e);
											X(d,b,e,c,b); X(d,b,e,b,c); X(d,b,c,e,b);
											X(d,b,c,b,e); X(d,b,b,e,c); X(d,b,b,c,e);
											X(b,d,e,c,b); X(b,d,e,b,c); X(b,d,c,e,b);
											X(b,d,c,b,e); X(b,d,b,e,c); X(b,d,b,c,e);
											X(c,d,e,b,b); X(c,d,b,e,b); X(c,d,b,b,e);
											X(c,b,d,e,b); X(c,b,d,b,e); X(c,b,b,e,d); 
											X(b,c,d,e,b); X(b,c,d,b,e); X(b,c,b,d,e);
											X(b,b,d,c,e); X(b,b,d,e,c); X(b,b,c,e,d); 
										}
									}
								}
							}
						}
					}
				} else {
					for (auto c = b; c < 8ul; ++c) {
						SKIP5s(c);
						auto cs = bs + c;
						auto c1 = outerConverted + (c * p10a);
						auto c2 = c * p10b;
						auto c3 = c * p10c;
						auto c4 = c * p10d;
						auto c5 = c * p10e;
						auto cp = bp * (c + 2);
						if (b == c) {
							for (auto d = c; d < 8ul; ++d) {
								SKIP5s(d);
								auto ds = cs + d;
								auto dp = cp * (d + 2);
								auto d1 = outerConverted + (d * p10a);
								auto d2 = d * p10b;
								auto d3 = d * p10c;
								auto d4 = d * p10d;
								auto d5 = d * p10e;
								if (c == d) {
									for (auto e = d; e < 8ul; ++e) {
										SKIP5s(e);
										auto es = ds + e;
										if (es % 3 != 0) {
											continue;
										}
										auto ep = dp * (e + 2);
										auto e1 = outerConverted + (e * p10a);
										X(e,d,c,c,a); X(e,d,c,a,c); X(e,d,a,c,c); 
										X(e,a,d,c,c); 
										auto e5 = e * p10e;
										X(a,b,c,d,e); 
										if (d != e) {
											auto e2 = e * p10b;
											auto e3 = e * p10c;
											auto e4 = e * p10d;
											X(d,e,c,c,a); X(d,e,c,a,c); X(d,e,a,c,c);
											X(d,c,e,c,a); X(d,c,e,a,c); X(d,c,c,e,a);
											X(d,c,c,a,e); X(d,c,a,e,c); X(d,c,a,c,e);
											X(a,c,c,e,d); X(c,a,d,e,c); X(c,a,d,c,e); 
											X(d,a,e,d,d); X(a,e,d,d,d); X(a,d,e,d,d); 
										}
									}
								} else {
									for (auto e = d; e < 8ul; ++e) {
										SKIP5s(e);
										auto es = ds + e;
										if (es % 3 != 0) {
											continue;
										}
										auto ep = dp * (e + 2);
										auto e1 = outerConverted + (e * p10a);
										X(e,a,d,c,c); X(e,d,c,c,a); 
										X(e,d,c,a,c); X(e,d,a,c,c); X(e,c,d,c,a); 
										X(e,c,d,a,c); X(e,c,c,d,a); X(e,c,c,a,d); 
										X(e,c,a,d,c); X(e,c,a,c,d); X(e,a,c,d,c); 
										X(e,a,c,c,d); 
										auto e2 = e * p10b;
										auto e3 = e * p10c;
										auto e4 = e * p10d;
										auto e5 = e * p10e;

										X(a,b,c,d,e); 
										X(c,e,d,c,a); X(c,e,d,a,c); 
										X(c,e,c,d,a); X(c,e,c,a,d); X(c,e,a,d,c); 
										X(c,e,a,c,d); X(c,c,e,d,a); X(c,c,e,a,d); 
										X(c,c,a,e,d); X(c,a,e,d,c); X(c,a,e,c,d); 
										X(c,a,c,e,d); X(a,e,d,c,c); X(a,e,c,d,c); 
										X(a,e,c,c,d); X(a,c,e,d,c); X(a,c,e,c,d); 
										if (d != e) {
											X(d,e,c,c,a); X(d,e,c,a,c); X(d,e,a,c,c);
											X(d,c,e,c,a); X(d,c,e,a,c); X(d,c,c,e,a);
											X(d,c,c,a,e); X(d,c,a,e,c); X(d,c,a,c,e);
											X(a,c,c,e,d); X(c,a,d,e,c); X(c,a,d,c,e); 
											X(d,a,e,c,c); X(d,a,c,e,c); X(d,a,c,c,e);
											X(c,d,e,c,a); X(c,d,e,a,c); X(c,d,c,e,a);
											X(c,d,c,a,e); X(c,d,a,e,c); X(c,d,a,c,e);
											X(c,c,d,e,a); X(c,c,d,a,e); X(c,c,a,d,e);
											X(a,d,e,c,c); X(a,d,c,e,c); X(a,d,c,c,e);
											X(a,c,d,e,c); X(a,c,d,c,e); X(c,a,c,d,e);
										}
									}
								}
							}
						} else {
							for (auto d = c; d < 8ul; ++d) {
								SKIP5s(d);
								auto ds = cs + d;
								auto dp = cp * (d + 2);
								auto d1 = outerConverted + (d * p10a);
								auto d2 = d * p10b;
								auto d3 = d * p10c;
								auto d4 = d * p10d;
								auto d5 = d * p10e;
								if (c == d) {
									for (auto e = d; e < 8ul; ++e) {
										SKIP5s(e);
										auto es = ds + e;
										if (es % 3 != 0) {
											continue;
										}
										auto ep = dp * (e + 2);
										auto e1 = outerConverted + (e * p10a);
										X(e,d,d,b,a); X(e,d,d,a,b); X(e,d,b,d,a);
										X(e,d,b,a,d); X(e,d,a,d,b); X(e,d,a,b,d);
										X(e,b,d,d,a); X(e,b,d,a,d); X(e,b,a,d,d);
										X(e,a,d,d,b); X(e,a,d,b,d); X(e,a,b,d,d);
										auto e2 = e * p10b;
										auto e5 = e * p10e;
										X(a,b,c,d,e); X(a,e,d,d,b); X(a,e,d,b,d); 
										X(a,e,b,d,d);
										X(b,e,d,d,a); X(b,e,d,a,d); X(b,e,a,d,d);
										X(b,a,d,d,e);

										if (d != e) {
											auto e3 = e * p10c;
											auto e4 = e * p10d;
											X(d,e,d,b,a); X(d,e,d,a,b); X(d,e,b,d,a);
											X(d,e,b,a,d); X(d,e,a,d,b); X(d,e,a,b,d);
											X(d,d,e,b,a); X(d,d,e,a,b); X(d,d,b,e,a);
											X(d,d,b,a,e); X(d,d,a,e,b); X(d,d,a,b,e);
											X(d,b,e,d,a); X(d,b,e,a,d); X(d,b,d,e,a);
											X(d,b,d,a,e); X(d,b,a,e,d); X(d,b,a,d,e);
											X(d,a,e,d,b); X(d,a,e,b,d); X(d,a,d,e,b);
											X(d,a,d,b,e); X(d,a,b,e,d); X(d,a,b,d,e);

											X(b,d,e,d,a); X(b,d,e,a,d); X(b,d,d,e,a);
											X(b,d,d,a,e); X(b,d,a,e,d); X(b,d,a,d,e);
											X(b,a,e,d,d); X(b,a,d,e,d); 

											X(a,d,e,d,b); X(a,d,e,b,d); X(a,d,d,e,b);
											X(a,d,d,b,e); X(a,d,b,e,d); X(a,d,b,d,e);
											X(a,b,e,d,d); X(a,b,d,e,d); 
										}
									}
								} else {
									for (auto e = d; e < 8ul; ++e) {
										SKIP5s(e);
										auto es = ds + e;
										if (es % 3 != 0) {
											continue;
										}
										auto ep = dp * (e + 2);
										auto e1 = outerConverted + (e * p10a);
										X(e,a,b,d,c); X(e,a,c,b,d); X(e,a,c,d,b);
										X(e,d,c,b,a); X(e,d,c,a,b); X(e,d,b,c,a);
										X(e,d,a,b,c); X(e,d,a,c,b); X(e,d,b,a,c); 
										X(e,c,b,d,a); X(e,c,d,a,b); X(e,c,d,b,a); 
										X(e,c,a,b,d); X(e,c,a,d,b); X(e,c,b,a,d);
										X(e,a,b,c,d); X(e,a,d,b,c); X(e,a,d,c,b); 
										X(e,b,a,c,d); X(e,b,a,d,c); X(e,b,c,a,d); 
										X(e,b,c,d,a); X(e,b,d,a,c); X(e,b,d,c,a); 
										auto e2 = e * p10b;
										auto e4 = e * p10d;
										auto e5 = e * p10e;
										X(a,b,c,d,e);
										X(a,e,c,d,b); X(a,e,d,b,c); X(a,e,d,c,b); 
										X(a,e,b,c,d); X(a,e,b,d,c); X(a,e,c,b,d); 
										X(a,c,b,e,d); X(a,c,d,b,e); X(a,c,d,e,b);
										X(a,b,d,c,e); X(a,b,d,e,c);

										X(b,e,c,d,a); X(b,e,d,a,c); X(b,e,d,c,a); 
										X(b,e,a,c,d); X(b,e,a,d,c); X(b,e,c,a,d); 
										X(b,c,d,e,a); X(b,c,d,a,e); X(b,c,a,d,e);
										X(b,a,c,e,d); X(b,a,d,c,e); X(b,a,d,e,c);

										X(c,e,b,d,a); X(c,e,d,a,b); X(c,e,d,b,a); 
										X(c,e,a,b,d); X(c,e,a,d,b); X(c,e,b,a,d); 
										X(c,b,d,a,e); X(c,b,d,e,a); X(c,b,a,d,e);
										X(c,a,d,e,b); X(c,a,d,b,e); X(c,a,b,d,e);


										if (d != e) {
											auto e3 = e * p10c;
											X(a,b,e,c,d); X(a,b,e,d,c); X(a,c,b,d,e);
											X(a,c,e,b,d); X(a,c,e,d,b); X(a,d,b,c,e);
											X(a,d,b,e,c); X(a,d,c,b,e); X(a,d,c,e,b);
											X(a,d,e,b,c); X(a,d,e,c,b); X(a,b,c,e,d); 

											X(b,a,c,d,e); X(b,a,e,c,d); X(b,a,e,d,c); 
											X(b,c,a,e,d); X(b,c,e,a,d); X(b,c,e,d,a); 
											X(b,d,a,c,e); X(b,d,a,e,c); X(b,d,c,a,e); 
											X(b,d,c,e,a); X(b,d,e,a,c); X(b,d,e,c,a); 

											X(c,a,b,e,d); X(c,a,e,b,d); X(c,a,e,d,b); 
											X(c,b,a,e,d); X(c,d,e,a,b); X(c,d,e,b,a); 
											X(c,b,e,a,d); X(c,b,e,d,a); X(c,d,a,b,e);
											X(c,d,a,e,b); X(c,d,b,a,e); X(c,d,b,e,a);

											X(d,a,b,e,c); X(d,a,c,b,e); X(d,a,c,e,b);
											X(d,a,e,b,c); X(d,a,e,c,b); X(d,b,a,c,e);
											X(d,b,a,e,c); X(d,b,c,a,e); X(d,b,c,e,a);
											X(d,b,e,a,c); X(d,b,e,c,a); X(d,c,a,b,e);
											X(d,c,a,e,b); X(d,c,b,a,e); X(d,c,b,e,a);
											X(d,c,e,a,b); X(d,c,e,b,a); X(d,e,a,b,c);
											X(d,e,a,c,b); X(d,e,b,a,c); X(d,e,b,c,a);
											X(d,e,c,a,b); X(d,e,c,b,a); X(d,a,b,c,e);
										}
									}
								}
							}
						}
					}
				}
			}
		}
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
