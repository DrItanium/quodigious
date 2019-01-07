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
		return [&significand]() -> u64 {
			switch(significand) {
				case 0b000: return computeFactor<2ul, nextPos>;
				case 0b001: return computeFactor<3ul, nextPos>;
				case 0b010: return computeFactor<4ul, nextPos>; 
				case 0b011: return computeFactor<5ul, nextPos>; 
				case 0b100: return computeFactor<6ul, nextPos>; 
				case 0b101: return computeFactor<7ul, nextPos>; 
				case 0b110: return computeFactor<8ul, nextPos>; 
				case 0b111: return computeFactor<9ul, nextPos>; 
			}
			return 0;
		}() + convertNumber<nextPos>(value);
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
template<u64 position, u64 length>
void body(MatchList& list, u64 sum = 0, u64 product = 1, u64 index = 0) noexcept {
	static_assert(length <= 19, "Can't have numbers over 19 digits on 64-bit numbers!");
	static_assert(length > 0, "Can't have length of zero!");
	static_assert(length >= position, "Position is out of bounds!");
	using DataTriple = std::tuple<u64, u64, u64>;
	static constexpr auto p10a = fastPow10<position>;
	static constexpr auto p10b = fastPow10<position+1>;
	static constexpr auto p10c = fastPow10<position+2>;
	static constexpr auto p10d = fastPow10<position+3>;
	static constexpr auto p10e = fastPow10<position+4>;
	static constexpr auto p10f = fastPow10<position+5>;
	static constexpr auto enableSplit6 = true;
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
			lenGreaterAndPos<13, 5, length, position> || 
			lenGreaterAndPos<14, 6, length, position>) {
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
	} else if constexpr (length > 10 && (length - position) == 5) {

		// this will generate a partial number but reduce the number of conversions
		// required greatly!
		// The last two digits are handled in a base 10 fashion without the +2 added
		// This will make the partial converison correct (remember that a 0 becomes a 2
		// in this model).
		//
		// Thus we implicitly add the offsets for each position to this base10 2 value :D

		auto outerConverted = convertNumber<length>(index);
#define X(x,y,z,w,h) if (auto n = x ## 1 + y ## 2 + z ## 3 + w ## 4 + h ## 5; ((n % ep == 0) && (n % es == 0))) { list.emplace_back(n); }
#define SUMCHECK if (es % 3 != 0) { continue; }
#define DECLARE_POSITION_VALUES(var) \
		auto var ## 1 = outerConverted + ( var * p10a ); \
		auto var ## 2 = var * p10b; \
		auto var ## 3 = var * p10c; \
		auto var ## 4 = var * p10d; \
		auto var ## 5 = var * p10e
		for (auto a = 0ul; a < 8ul; ++a) {
			SKIP5s(a);
			DECLARE_POSITION_VALUES(a);
			auto as = sum + a;
			auto ap = product * (a + 2);
			for (auto b = a; b < 8ul; ++b) {
				SKIP5s(b);
				DECLARE_POSITION_VALUES(b);
				auto bs = as + b;
				auto bp = ap * (b + 2);
				if (a == b) {
					for (auto c = b; c < 8ul; ++c) {
						SKIP5s(c);
						auto cs = bs + c;
						auto cp = bp * (c + 2);
						DECLARE_POSITION_VALUES(c);
						if (b == c) {
							for (auto d = c; d < 8ul; ++d) {
								SKIP5s(d);
								auto ds = cs + d;
								auto dp = cp * (d + 2);
								DECLARE_POSITION_VALUES(d);
								if (c == d) {
									for (auto e = d; e < 8ul; ++e) {
										SKIP5s(e);
										auto es = ds + e;
										SUMCHECK
											auto ep = dp * (e + 2);
										auto e5 = e * p10e;
										X(d,d,d,d,e);
										if (d != e) {
											auto e1 = outerConverted + (e * p10a);
											auto e2 = e * p10b;
											auto e3 = e * p10c;
											auto e4 = e * p10d;
											X(e,d,d,d,d); X(d,e,d,d,d); X(d,d,e,d,d); 
											X(d,d,d,e,d); 
										}
									}
								} else {
									for (auto e = d; e < 8ul; ++e) {
										SKIP5s(e);
										auto es = ds + e;
										SUMCHECK
											auto ep = dp * (e + 2);
										auto e1 = outerConverted + (e * p10a);
										X(e,d,c,c,c); X(e,c,d,c,c); X(e,c,c,d,c);
										X(e,c,c,c,d); 
										auto e2 = e * p10b;
										auto e3 = e * p10c;
										auto e5 = e * p10e;
										X(a,b,c,d,e); X(c,e,d,c,c); X(c,e,c,d,c); 
										X(c,e,c,c,d); X(c,c,e,d,c); X(c,c,e,c,d); 
										if (d != e) {
											auto e4 = e * p10d;
											X(d,e,c,c,c); X(d,c,e,c,c); X(d,c,c,e,c); 
											X(d,c,c,c,e); X(c,d,e,c,c); X(c,d,c,e,c); 
											X(c,d,c,c,e); X(c,c,d,e,c); X(c,c,d,c,e); 
											X(c,c,c,e,d); 
										}
									}
								}
							}
						} else {
							for (auto d = c; d < 8ul; ++d) {
								SKIP5s(d);
								auto ds = cs + d;
								auto dp = cp * (d + 2);
								DECLARE_POSITION_VALUES(d);
								if (c == d) {
									for (auto e = d; e < 8ul; ++e) {
										SKIP5s(e);
										auto es = ds + e;
										SUMCHECK
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
										SUMCHECK
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
						auto cp = bp * (c + 2);
						DECLARE_POSITION_VALUES(c);
						if (b == c) {
							for (auto d = c; d < 8ul; ++d) {
								SKIP5s(d);
								auto ds = cs + d;
								auto dp = cp * (d + 2);
								DECLARE_POSITION_VALUES(d);
								if (c == d) {
									for (auto e = d; e < 8ul; ++e) {
										SKIP5s(e);
										auto es = ds + e;
										SUMCHECK
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
										SUMCHECK
											auto ep = dp * (e + 2);
										auto e1 = outerConverted + (e * p10a);
										X(e,a,d,c,c); X(e,d,c,c,a); X(e,d,c,a,c); 
										X(e,d,a,c,c); X(e,c,d,c,a); X(e,c,d,a,c); 
										X(e,c,c,d,a); X(e,c,c,a,d); X(e,c,a,d,c); 
										X(e,c,a,c,d); X(e,a,c,d,c); X(e,a,c,c,d); 
										auto e2 = e * p10b;
										auto e3 = e * p10c;
										auto e4 = e * p10d;
										auto e5 = e * p10e;
										X(a,b,c,d,e); X(c,e,d,c,a); X(c,e,d,a,c); 
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
								DECLARE_POSITION_VALUES(d);
								if (c == d) {
									for (auto e = d; e < 8ul; ++e) {
										SKIP5s(e);
										auto es = ds + e;
										SUMCHECK
											auto ep = dp * (e + 2);
										auto e1 = outerConverted + (e * p10a);
										X(e,d,d,b,a); X(e,d,d,a,b); X(e,d,b,d,a);
										X(e,d,b,a,d); X(e,d,a,d,b); X(e,d,a,b,d);
										X(e,b,d,d,a); X(e,b,d,a,d); X(e,b,a,d,d);
										X(e,a,d,d,b); X(e,a,d,b,d); X(e,a,b,d,d);
										auto e2 = e * p10b;
										auto e5 = e * p10e;
										X(a,b,c,d,e); X(a,e,d,d,b); X(a,e,d,b,d); 
										X(a,e,b,d,d); X(b,e,d,d,a); X(b,e,d,a,d); 
										X(b,e,a,d,d); X(b,a,d,d,e);

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
										SUMCHECK
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
#undef DECLARE_POSITION_VALUES
#undef SUMCHECK
#undef X
	} else if constexpr (enableSplit6 && length > 13 && (length - position) == 6) {

		// this will generate a partial number but reduce the number of conversions
		// required greatly!
		// The last two digits are handled in a base 10 fashion without the +2 added
		// This will make the partial converison correct (remember that a 0 becomes a 2
		// in this model).
		//
		// Thus we implicitly add the offsets for each position to this base10 2 value :D
		//

		auto outerConverted = convertNumber<length>(index);
		auto fn = [&list](auto n, auto ep, auto es) {
			if ((n % ep == 0) && (n % es == 0)) {
				list.emplace_back(n);
			}
		};
#define X(x,y,z,w,h,q) fn( x ## 1 + y ## 2 + z ## 3 + w ## 4 + h ## 5 + q ## 6, ep, es)
#define SUMCHECK if (es % 3 != 0) { continue; }
#define DECLARE_POSITION_VALUES(var) \
		auto var ## 1 = outerConverted + (var * p10a); \
		auto var ## 2 = (var * p10b); \
		auto var ## 3 = (var * p10c); \
		auto var ## 4 = (var * p10d); \
		auto var ## 5 = (var * p10e); \
		auto var ## 6 = (var * p10f)
		for (auto f = 0ul; f < 8ul; ++f) {
			SKIP5s(f);
			DECLARE_POSITION_VALUES(f);
			auto fs = sum + f;
			auto fp = product * (f + 2);
			for (auto a = f; a < 8ul; ++a) {
				SKIP5s(a);
				DECLARE_POSITION_VALUES(a);
				if (auto as = fs + a, ap = fp * (a + 2); a == f) {
					for (auto b = a; b < 8ul; ++b) {
						SKIP5s(b);
						DECLARE_POSITION_VALUES(b);
						if (auto bs = as + b, bp = ap * (b + 2); a == b) {
							for (auto c = b; c < 8ul; ++c) {
								SKIP5s(c);
								DECLARE_POSITION_VALUES(c);
								if (auto cs = bs + c, cp = bp * (c + 2); b == c) {
									for (auto d = c; d < 8ul; ++d) {
										SKIP5s(d);
										DECLARE_POSITION_VALUES(d);
										if (auto ds = cs + d, dp = cp * (d + 2); c == d) {
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												if (auto ep = dp * (e + 2); d != e) {
													DECLARE_POSITION_VALUES(e);
													X(e,d,d,d,d,d); X(d,e,d,d,d,d); X(d,d,e,d,d,d); 
													X(d,d,d,e,d,d); X(d,d,d,d,e,d); X(d,d,d,d,d,e); 
												} else {
													X(d,d,d,d,d,d);
												}
											}
										} else {
											// c != d
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												if (auto ep = dp * (e + 2); d != e) {
													DECLARE_POSITION_VALUES(e);
													X(e,d,c,c,c,c); X(e,c,d,c,c,c); X(e,c,c,d,c,c);
													X(e,c,c,c,d,c); X(e,c,c,c,c,d); X(d,e,c,c,c,c);
													X(d,c,e,c,c,c); X(d,c,c,e,c,c); X(d,c,c,c,e,c);
													X(d,c,c,c,c,e); X(c,e,d,c,c,c); X(c,e,c,d,c,c);
													X(c,e,c,c,d,c); X(c,e,c,c,c,d); X(c,d,e,c,c,c);
													X(c,d,c,e,c,c); X(c,d,c,c,e,c); X(c,d,c,c,c,e);
													X(c,c,e,d,c,c); X(c,c,e,c,d,c); X(c,c,e,c,c,d);
													X(c,c,d,e,c,c); X(c,c,d,c,e,c); X(c,c,d,c,c,e);
													X(c,c,c,e,d,c); X(c,c,c,e,c,d); X(c,c,c,d,e,c);
													X(c,c,c,d,c,e); X(c,c,c,c,e,d); X(c,c,c,c,d,e);
												} else {
													X(d,d,c,c,c,c); X(d,c,d,c,c,c); X(d,c,c,d,c,c);
													X(d,c,c,c,d,c); X(d,c,c,c,c,d); X(c,d,d,c,c,c);
													X(c,d,c,d,c,c); X(c,d,c,c,d,c); X(c,d,c,c,c,d);
													X(c,c,d,d,c,c); X(c,c,d,c,d,c); X(c,c,d,c,c,d);
													X(c,c,c,d,d,c); X(c,c,c,d,c,d); X(c,c,c,c,d,d);
												}
											}
										}
									}
								} else {
									// b != c
									for (auto d = c; d < 8ul; ++d) {
										SKIP5s(d);
										DECLARE_POSITION_VALUES(d);
										if (auto ds = cs + d, dp = cp * (d + 2); c == d) {
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												if (auto ep = dp * (e + 2); d != e) {
													DECLARE_POSITION_VALUES(e);
													X(e,c,c,b,b,b); X(e,c,b,c,b,b); X(e,c,b,b,c,b);
													X(e,c,b,b,b,c); X(e,b,c,c,b,b); X(e,b,c,b,c,b);
													X(e,b,c,b,b,c); X(e,b,b,c,c,b); X(e,b,b,c,b,c);
													X(e,b,b,b,c,c); X(c,e,c,b,b,b); X(c,e,b,c,b,b);
													X(c,e,b,b,c,b); X(c,e,b,b,b,c); X(c,c,e,b,b,b);
													X(c,c,b,e,b,b); X(c,c,b,b,e,b); X(c,c,b,b,b,e);
													X(c,b,e,c,b,b); X(c,b,e,b,c,b); X(c,b,e,b,b,c);
													X(c,b,c,e,b,b); X(c,b,c,b,e,b); X(c,b,c,b,b,e);
													X(c,b,b,e,c,b); X(c,b,b,e,b,c); X(c,b,b,c,e,b);
													X(c,b,b,c,b,e); X(c,b,b,b,e,c); X(c,b,b,b,c,e);
													X(b,e,c,c,b,b); X(b,e,c,b,c,b); X(b,e,c,b,b,c);
													X(b,e,b,c,c,b); X(b,e,b,c,b,c); X(b,e,b,b,c,c);
													X(b,c,e,c,b,b); X(b,c,e,b,c,b); X(b,c,e,b,b,c);
													X(b,c,c,e,b,b); X(b,c,c,b,e,b); X(b,c,c,b,b,e);
													X(b,c,b,e,c,b); X(b,c,b,e,b,c); X(b,c,b,c,e,b);
													X(b,c,b,c,b,e); X(b,c,b,b,e,c); X(b,c,b,b,c,e);
													X(b,b,e,c,c,b); X(b,b,e,c,b,c); X(b,b,e,b,c,c);
													X(b,b,c,e,c,b); X(b,b,c,e,b,c); X(b,b,c,c,e,b);
													X(b,b,c,c,b,e); X(b,b,c,b,e,c); X(b,b,c,b,c,e);
													X(b,b,b,e,c,c); X(b,b,b,c,e,c); X(b,b,b,c,c,e);
												} else {
													X(c,c,c,b,b,b); X(c,c,b,c,b,b); X(c,c,b,b,c,b);
													X(c,c,b,b,b,c); X(c,b,c,c,b,b); X(c,b,c,b,c,b);
													X(c,b,c,b,b,c); X(c,b,b,c,c,b); X(c,b,b,c,b,c);
													X(c,b,b,b,c,c); X(b,c,c,c,b,b); X(b,c,c,b,c,b);
													X(b,c,c,b,b,c); X(b,c,b,c,c,b); X(b,c,b,c,b,c);
													X(b,c,b,b,c,c); X(b,b,c,c,c,b); X(b,b,c,c,b,c);
													X(b,b,c,b,c,c); X(b,b,b,c,c,c);
												}
											}
										} else {
											// c != d
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												if (auto ep = dp * (e + 2); d != e) {
													DECLARE_POSITION_VALUES(e);
													X(e,d,c,b,b,b); X(e,d,b,c,b,b); X(e,d,b,b,c,b);
													X(e,d,b,b,b,c); X(e,c,d,b,b,b); X(e,c,b,d,b,b);
													X(e,c,b,b,d,b); X(e,c,b,b,b,d); X(e,b,d,c,b,b);
													X(e,b,d,b,c,b); X(e,b,d,b,b,c); X(e,b,c,d,b,b);
													X(e,b,c,b,d,b); X(e,b,c,b,b,d); X(e,b,b,d,c,b);
													X(e,b,b,d,b,c); X(e,b,b,c,d,b); X(e,b,b,c,b,d);
													X(e,b,b,b,d,c); X(e,b,b,b,c,d); X(d,e,c,b,b,b);
													X(d,e,b,c,b,b); X(d,e,b,b,c,b); X(d,e,b,b,b,c);
													X(d,c,e,b,b,b); X(d,c,b,e,b,b); X(d,c,b,b,e,b);
													X(d,c,b,b,b,e); X(d,b,e,c,b,b); X(d,b,e,b,c,b);
													X(d,b,e,b,b,c); X(d,b,c,e,b,b); X(d,b,c,b,e,b);
													X(d,b,c,b,b,e); X(d,b,b,e,c,b); X(d,b,b,e,b,c);
													X(d,b,b,c,e,b); X(d,b,b,c,b,e); X(d,b,b,b,e,c);
													X(d,b,b,b,c,e); X(c,e,d,b,b,b); X(c,e,b,d,b,b);
													X(c,e,b,b,d,b); X(c,e,b,b,b,d); X(c,d,e,b,b,b);
													X(c,d,b,e,b,b); X(c,d,b,b,e,b); X(c,d,b,b,b,e);
													X(c,b,e,d,b,b); X(c,b,e,b,d,b); X(c,b,e,b,b,d);
													X(c,b,d,e,b,b); X(c,b,d,b,e,b); X(c,b,d,b,b,e);
													X(c,b,b,e,d,b); X(c,b,b,e,b,d); X(c,b,b,d,e,b);
													X(c,b,b,d,b,e); X(c,b,b,b,e,d); X(c,b,b,b,d,e);
													X(b,e,d,c,b,b); X(b,e,d,b,c,b); X(b,e,d,b,b,c);
													X(b,e,c,d,b,b); X(b,e,c,b,d,b); X(b,e,c,b,b,d);
													X(b,e,b,d,c,b); X(b,e,b,d,b,c); X(b,e,b,c,d,b);
													X(b,e,b,c,b,d); X(b,e,b,b,d,c); X(b,e,b,b,c,d);
													X(b,d,e,c,b,b); X(b,d,e,b,c,b); X(b,d,e,b,b,c);
													X(b,d,c,e,b,b); X(b,d,c,b,e,b); X(b,d,c,b,b,e);
													X(b,d,b,e,c,b); X(b,d,b,e,b,c); X(b,d,b,c,e,b);
													X(b,d,b,c,b,e); X(b,d,b,b,e,c); X(b,d,b,b,c,e);
													X(b,c,e,d,b,b); X(b,c,e,b,d,b); X(b,c,e,b,b,d);
													X(b,c,d,e,b,b); X(b,c,d,b,e,b); X(b,c,d,b,b,e);
													X(b,c,b,e,d,b); X(b,c,b,e,b,d); X(b,c,b,d,e,b);
													X(b,c,b,d,b,e); X(b,c,b,b,e,d); X(b,c,b,b,d,e);
													X(b,b,e,d,c,b); X(b,b,e,d,b,c); X(b,b,e,c,d,b);
													X(b,b,e,c,b,d); X(b,b,e,b,d,c); X(b,b,e,b,c,d);
													X(b,b,d,e,c,b); X(b,b,d,e,b,c); X(b,b,d,c,e,b);
													X(b,b,d,c,b,e); X(b,b,d,b,e,c); X(b,b,d,b,c,e);
													X(b,b,c,e,d,b); X(b,b,c,e,b,d); X(b,b,c,d,e,b);
													X(b,b,c,d,b,e); X(b,b,c,b,e,d); X(b,b,c,b,d,e);
													X(b,b,b,e,d,c); X(b,b,b,e,c,d); X(b,b,b,d,e,c);
													X(b,b,b,d,c,e); X(b,b,b,c,e,d); X(b,b,b,c,d,e);
												} else {
													// d == e
													X(d,d,c,b,b,b); X(d,d,b,c,b,b); X(d,d,b,b,c,b);
													X(d,d,b,b,b,c); X(d,c,d,b,b,b); X(d,c,b,d,b,b);
													X(d,c,b,b,d,b); X(d,c,b,b,b,d); X(d,b,d,c,b,b);
													X(d,b,d,b,c,b); X(d,b,d,b,b,c); X(d,b,c,d,b,b);
													X(d,b,c,b,d,b); X(d,b,c,b,b,d); X(d,b,b,d,c,b);
													X(d,b,b,d,b,c); X(d,b,b,c,d,b); X(d,b,b,c,b,d);
													X(d,b,b,b,d,c); X(d,b,b,b,c,d); X(c,d,d,b,b,b);
													X(c,d,b,d,b,b); X(c,d,b,b,d,b); X(c,d,b,b,b,d);
													X(c,b,d,d,b,b); X(c,b,d,b,d,b); X(c,b,d,b,b,d);
													X(c,b,b,d,d,b); X(c,b,b,d,b,d); X(c,b,b,b,d,d);
													X(b,d,d,c,b,b); X(b,d,d,b,c,b); X(b,d,d,b,b,c);
													X(b,d,c,d,b,b); X(b,d,c,b,d,b); X(b,d,c,b,b,d);
													X(b,d,b,d,c,b); X(b,d,b,d,b,c); X(b,d,b,c,d,b);
													X(b,d,b,c,b,d); X(b,d,b,b,d,c); X(b,d,b,b,c,d);
													X(b,c,d,d,b,b); X(b,c,d,b,d,b); X(b,c,d,b,b,d);
													X(b,c,b,d,d,b); X(b,c,b,d,b,d); X(b,c,b,b,d,d);
													X(b,b,d,d,c,b); X(b,b,d,d,b,c); X(b,b,d,c,d,b);
													X(b,b,d,c,b,d); X(b,b,d,b,d,c); X(b,b,d,b,c,d);
													X(b,b,c,d,d,b); X(b,b,c,d,b,d); X(b,b,c,b,d,d);
													X(b,b,b,d,d,c); X(b,b,b,d,c,d); X(b,b,b,c,d,d);
												}
											}
										}
									}
								}
							}
						} else {
							// a != b
							for (auto c = b; c < 8ul; ++c) {
								SKIP5s(c);
								DECLARE_POSITION_VALUES(c);
								if (auto cs = bs + c, cp = bp * (c + 2); b == c) {
									for (auto d = c; d < 8ul; ++d) {
										SKIP5s(d);
										DECLARE_POSITION_VALUES(d);
										if (auto ds = cs + d, dp = cp * (d + 2); c == d) {
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												if (auto ep = dp * (e + 2); d != e) {
													DECLARE_POSITION_VALUES(e);
													X(e,d,d,d,a,a); X(e,d,d,a,d,a); X(e,d,d,a,a,d);
													X(e,d,a,d,d,a); X(e,d,a,d,a,d); X(e,d,a,a,d,d);
													X(e,a,d,d,d,a); X(e,a,d,d,a,d); X(e,a,d,a,d,d);
													X(e,a,a,d,d,d); X(d,e,d,d,a,a); X(d,e,d,a,d,a);
													X(d,e,d,a,a,d); X(d,e,a,d,d,a); X(d,e,a,d,a,d);
													X(d,e,a,a,d,d); X(d,d,e,d,a,a); X(d,d,e,a,d,a);
													X(d,d,e,a,a,d); X(d,d,d,e,a,a); X(d,d,d,a,e,a);
													X(d,d,d,a,a,e); X(d,d,a,e,d,a); X(d,d,a,e,a,d);
													X(d,d,a,d,e,a); X(d,d,a,d,a,e); X(d,d,a,a,e,d);
													X(d,d,a,a,d,e); X(d,a,e,d,d,a); X(d,a,e,d,a,d);
													X(d,a,e,a,d,d); X(d,a,d,e,d,a); X(d,a,d,e,a,d);
													X(d,a,d,d,e,a); X(d,a,d,d,a,e); X(d,a,d,a,e,d);
													X(d,a,d,a,d,e); X(d,a,a,e,d,d); X(d,a,a,d,e,d);
													X(d,a,a,d,d,e); X(a,e,d,d,d,a); X(a,e,d,d,a,d);
													X(a,e,d,a,d,d); X(a,e,a,d,d,d); X(a,d,e,d,d,a);
													X(a,d,e,d,a,d); X(a,d,e,a,d,d); X(a,d,d,e,d,a);
													X(a,d,d,e,a,d); X(a,d,d,d,e,a); X(a,d,d,d,a,e);
													X(a,d,d,a,e,d); X(a,d,d,a,d,e); X(a,d,a,e,d,d);
													X(a,d,a,d,e,d); X(a,d,a,d,d,e); X(a,a,e,d,d,d);
													X(a,a,d,e,d,d); X(a,a,d,d,e,d); X(a,a,d,d,d,e);
												} else {
													X(d,d,d,d,a,a); X(d,d,d,a,d,a); X(d,d,d,a,a,d);
													X(d,d,a,d,d,a); X(d,d,a,d,a,d); X(d,d,a,a,d,d);
													X(d,a,d,d,d,a); X(d,a,d,d,a,d); X(d,a,d,a,d,d);
													X(d,a,a,d,d,d); X(a,d,d,d,d,a); X(a,d,d,d,a,d);
													X(a,d,d,a,d,d); X(a,d,a,d,d,d); X(a,a,d,d,d,d);
												}
											}
										} else {
											// c != d
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												if (auto ep = dp * (e + 2); d != e) {
													DECLARE_POSITION_VALUES(e);
													X(e,d,c,c,a,a); X(e,d,c,a,c,a); X(e,d,c,a,a,c);
													X(e,d,a,c,c,a); X(e,d,a,c,a,c); X(e,d,a,a,c,c);
													X(e,c,d,c,a,a); X(e,c,d,a,c,a); X(e,c,d,a,a,c);
													X(e,c,c,d,a,a); X(e,c,c,a,d,a); X(e,c,c,a,a,d);
													X(e,c,a,d,c,a); X(e,c,a,d,a,c); X(e,c,a,c,d,a);
													X(e,c,a,c,a,d); X(e,c,a,a,d,c); X(e,c,a,a,c,d);
													X(e,a,d,c,c,a); X(e,a,d,c,a,c); X(e,a,d,a,c,c);
													X(e,a,c,d,c,a); X(e,a,c,d,a,c); X(e,a,c,c,d,a);
													X(e,a,c,c,a,d); X(e,a,c,a,d,c); X(e,a,c,a,c,d);
													X(e,a,a,d,c,c); X(e,a,a,c,d,c); X(e,a,a,c,c,d);
													X(d,e,c,c,a,a); X(d,e,c,a,c,a); X(d,e,c,a,a,c);
													X(d,e,a,c,c,a); X(d,e,a,c,a,c); X(d,e,a,a,c,c);
													X(d,c,e,c,a,a); X(d,c,e,a,c,a); X(d,c,e,a,a,c);
													X(d,c,c,e,a,a); X(d,c,c,a,e,a); X(d,c,c,a,a,e);
													X(d,c,a,e,c,a); X(d,c,a,e,a,c); X(d,c,a,c,e,a);
													X(d,c,a,c,a,e); X(d,c,a,a,e,c); X(d,c,a,a,c,e);
													X(d,a,e,c,c,a); X(d,a,e,c,a,c); X(d,a,e,a,c,c);
													X(d,a,c,e,c,a); X(d,a,c,e,a,c); X(d,a,c,c,e,a);
													X(d,a,c,c,a,e); X(d,a,c,a,e,c); X(d,a,c,a,c,e);
													X(d,a,a,e,c,c); X(d,a,a,c,e,c); X(d,a,a,c,c,e);
													X(c,e,d,c,a,a); X(c,e,d,a,c,a); X(c,e,d,a,a,c);
													X(c,e,c,d,a,a); X(c,e,c,a,d,a); X(c,e,c,a,a,d);
													X(c,e,a,d,c,a); X(c,e,a,d,a,c); X(c,e,a,c,d,a);
													X(c,e,a,c,a,d); X(c,e,a,a,d,c); X(c,e,a,a,c,d);
													X(c,d,e,c,a,a); X(c,d,e,a,c,a); X(c,d,e,a,a,c);
													X(c,d,c,e,a,a); X(c,d,c,a,e,a); X(c,d,c,a,a,e);
													X(c,d,a,e,c,a); X(c,d,a,e,a,c); X(c,d,a,c,e,a);
													X(c,d,a,c,a,e); X(c,d,a,a,e,c); X(c,d,a,a,c,e);
													X(c,c,e,d,a,a); X(c,c,e,a,d,a); X(c,c,e,a,a,d);
													X(c,c,d,e,a,a); X(c,c,d,a,e,a); X(c,c,d,a,a,e);
													X(c,c,a,e,d,a); X(c,c,a,e,a,d); X(c,c,a,d,e,a);
													X(c,c,a,d,a,e); X(c,c,a,a,e,d); X(c,c,a,a,d,e);
													X(c,a,e,d,c,a); X(c,a,e,d,a,c); X(c,a,e,c,d,a);
													X(c,a,e,c,a,d); X(c,a,e,a,d,c); X(c,a,e,a,c,d);
													X(c,a,d,e,c,a); X(c,a,d,e,a,c); X(c,a,d,c,e,a);
													X(c,a,d,c,a,e); X(c,a,d,a,e,c); X(c,a,d,a,c,e);
													X(c,a,c,e,d,a); X(c,a,c,e,a,d); X(c,a,c,d,e,a);
													X(c,a,c,d,a,e); X(c,a,c,a,e,d); X(c,a,c,a,d,e);
													X(c,a,a,e,d,c); X(c,a,a,e,c,d); X(c,a,a,d,e,c);
													X(c,a,a,d,c,e); X(c,a,a,c,e,d); X(c,a,a,c,d,e);
													X(a,e,d,c,c,a); X(a,e,d,c,a,c); X(a,e,d,a,c,c);
													X(a,e,c,d,c,a); X(a,e,c,d,a,c); X(a,e,c,c,d,a);
													X(a,e,c,c,a,d); X(a,e,c,a,d,c); X(a,e,c,a,c,d);
													X(a,e,a,d,c,c); X(a,e,a,c,d,c); X(a,e,a,c,c,d);
													X(a,d,e,c,c,a); X(a,d,e,c,a,c); X(a,d,e,a,c,c);
													X(a,d,c,e,c,a); X(a,d,c,e,a,c); X(a,d,c,c,e,a);
													X(a,d,c,c,a,e); X(a,d,c,a,e,c); X(a,d,c,a,c,e);
													X(a,d,a,e,c,c); X(a,d,a,c,e,c); X(a,d,a,c,c,e);
													X(a,c,e,d,c,a); X(a,c,e,d,a,c); X(a,c,e,c,d,a);
													X(a,c,e,c,a,d); X(a,c,e,a,d,c); X(a,c,e,a,c,d);
													X(a,c,d,e,c,a); X(a,c,d,e,a,c); X(a,c,d,c,e,a);
													X(a,c,d,c,a,e); X(a,c,d,a,e,c); X(a,c,d,a,c,e);
													X(a,c,c,e,d,a); X(a,c,c,e,a,d); X(a,c,c,d,e,a);
													X(a,c,c,d,a,e); X(a,c,c,a,e,d); X(a,c,c,a,d,e);
													X(a,c,a,e,d,c); X(a,c,a,e,c,d); X(a,c,a,d,e,c);
													X(a,c,a,d,c,e); X(a,c,a,c,e,d); X(a,c,a,c,d,e);
													X(a,a,e,d,c,c); X(a,a,e,c,d,c); X(a,a,e,c,c,d);
													X(a,a,d,e,c,c); X(a,a,d,c,e,c); X(a,a,d,c,c,e);
													X(a,a,c,e,d,c); X(a,a,c,e,c,d); X(a,a,c,d,e,c);
													X(a,a,c,d,c,e); X(a,a,c,c,e,d); X(a,a,c,c,d,e);
												} else {
													X(d,d,c,c,a,a); X(d,d,c,a,c,a); X(d,d,c,a,a,c);
													X(d,d,a,c,c,a); X(d,d,a,c,a,c); X(d,d,a,a,c,c);
													X(d,c,d,c,a,a); X(d,c,d,a,c,a); X(d,c,d,a,a,c);
													X(d,c,c,d,a,a); X(d,c,c,a,d,a); X(d,c,c,a,a,d);
													X(d,c,a,d,c,a); X(d,c,a,d,a,c); X(d,c,a,c,d,a);
													X(d,c,a,c,a,d); X(d,c,a,a,d,c); X(d,c,a,a,c,d);
													X(d,a,d,c,c,a); X(d,a,d,c,a,c); X(d,a,d,a,c,c);
													X(d,a,c,d,c,a); X(d,a,c,d,a,c); X(d,a,c,c,d,a);
													X(d,a,c,c,a,d); X(d,a,c,a,d,c); X(d,a,c,a,c,d);
													X(d,a,a,d,c,c); X(d,a,a,c,d,c); X(d,a,a,c,c,d);
													X(c,d,d,c,a,a); X(c,d,d,a,c,a); X(c,d,d,a,a,c);
													X(c,d,c,d,a,a); X(c,d,c,a,d,a); X(c,d,c,a,a,d);
													X(c,d,a,d,c,a); X(c,d,a,d,a,c); X(c,d,a,c,d,a);
													X(c,d,a,c,a,d); X(c,d,a,a,d,c); X(c,d,a,a,c,d);
													X(c,c,d,d,a,a); X(c,c,d,a,d,a); X(c,c,d,a,a,d);
													X(c,c,a,d,d,a); X(c,c,a,d,a,d); X(c,c,a,a,d,d);
													X(c,a,d,d,c,a); X(c,a,d,d,a,c); X(c,a,d,c,d,a);
													X(c,a,d,c,a,d); X(c,a,d,a,d,c); X(c,a,d,a,c,d);
													X(c,a,c,d,d,a); X(c,a,c,d,a,d); X(c,a,c,a,d,d);
													X(c,a,a,d,d,c); X(c,a,a,d,c,d); X(c,a,a,c,d,d);
													X(a,d,d,c,c,a); X(a,d,d,c,a,c); X(a,d,d,a,c,c);
													X(a,d,c,d,c,a); X(a,d,c,d,a,c); X(a,d,c,c,d,a);
													X(a,d,c,c,a,d); X(a,d,c,a,d,c); X(a,d,c,a,c,d);
													X(a,d,a,d,c,c); X(a,d,a,c,d,c); X(a,d,a,c,c,d);
													X(a,c,d,d,c,a); X(a,c,d,d,a,c); X(a,c,d,c,d,a);
													X(a,c,d,c,a,d); X(a,c,d,a,d,c); X(a,c,d,a,c,d);
													X(a,c,c,d,d,a); X(a,c,c,d,a,d); X(a,c,c,a,d,d);
													X(a,c,a,d,d,c); X(a,c,a,d,c,d); X(a,c,a,c,d,d);
													X(a,a,d,d,c,c); X(a,a,d,c,d,c); X(a,a,d,c,c,d);
													X(a,a,c,d,d,c); X(a,a,c,d,c,d); X(a,a,c,c,d,d);
												}
											}
										}
									}
								} else {
									// b != c
									for (auto d = c; d < 8ul; ++d) {
										SKIP5s(d);
										DECLARE_POSITION_VALUES(d);
										if (auto ds = cs + d, dp = cp * (d + 2); c == d) {
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												if (auto ep = dp * (e + 2); d != e) {
													DECLARE_POSITION_VALUES(e);
													X(e,d,d,b,a,a); X(e,d,d,a,b,a); X(e,d,d,a,a,b);
													X(e,d,b,d,a,a); X(e,d,b,a,d,a); X(e,d,b,a,a,d);
													X(e,d,a,d,b,a); X(e,d,a,d,a,b); X(e,d,a,b,d,a);
													X(e,d,a,b,a,d); X(e,d,a,a,d,b); X(e,d,a,a,b,d);
													X(e,b,d,d,a,a); X(e,b,d,a,d,a); X(e,b,d,a,a,d);
													X(e,b,a,d,d,a); X(e,b,a,d,a,d); X(e,b,a,a,d,d);
													X(e,a,d,d,b,a); X(e,a,d,d,a,b); X(e,a,d,b,d,a);
													X(e,a,d,b,a,d); X(e,a,d,a,d,b); X(e,a,d,a,b,d);
													X(e,a,b,d,d,a); X(e,a,b,d,a,d); X(e,a,b,a,d,d);
													X(e,a,a,d,d,b); X(e,a,a,d,b,d); X(e,a,a,b,d,d);
													X(d,e,d,b,a,a); X(d,e,d,a,b,a); X(d,e,d,a,a,b);
													X(d,e,b,d,a,a); X(d,e,b,a,d,a); X(d,e,b,a,a,d);
													X(d,e,a,d,b,a); X(d,e,a,d,a,b); X(d,e,a,b,d,a);
													X(d,e,a,b,a,d); X(d,e,a,a,d,b); X(d,e,a,a,b,d);
													X(d,d,e,b,a,a); X(d,d,e,a,b,a); X(d,d,e,a,a,b);
													X(d,d,b,e,a,a); X(d,d,b,a,e,a); X(d,d,b,a,a,e);
													X(d,d,a,e,b,a); X(d,d,a,e,a,b); X(d,d,a,b,e,a);
													X(d,d,a,b,a,e); X(d,d,a,a,e,b); X(d,d,a,a,b,e);
													X(d,b,e,d,a,a); X(d,b,e,a,d,a); X(d,b,e,a,a,d);
													X(d,b,d,e,a,a); X(d,b,d,a,e,a); X(d,b,d,a,a,e);
													X(d,b,a,e,d,a); X(d,b,a,e,a,d); X(d,b,a,d,e,a);
													X(d,b,a,d,a,e); X(d,b,a,a,e,d); X(d,b,a,a,d,e);
													X(d,a,e,d,b,a); X(d,a,e,d,a,b); X(d,a,e,b,d,a);
													X(d,a,e,b,a,d); X(d,a,e,a,d,b); X(d,a,e,a,b,d);
													X(d,a,d,e,b,a); X(d,a,d,e,a,b); X(d,a,d,b,e,a);
													X(d,a,d,b,a,e); X(d,a,d,a,e,b); X(d,a,d,a,b,e);
													X(d,a,b,e,d,a); X(d,a,b,e,a,d); X(d,a,b,d,e,a);
													X(d,a,b,d,a,e); X(d,a,b,a,e,d); X(d,a,b,a,d,e);
													X(d,a,a,e,d,b); X(d,a,a,e,b,d); X(d,a,a,d,e,b);
													X(d,a,a,d,b,e); X(d,a,a,b,e,d); X(d,a,a,b,d,e);
													X(b,e,d,d,a,a); X(b,e,d,a,d,a); X(b,e,d,a,a,d);
													X(b,e,a,d,d,a); X(b,e,a,d,a,d); X(b,e,a,a,d,d);
													X(b,d,e,d,a,a); X(b,d,e,a,d,a); X(b,d,e,a,a,d);
													X(b,d,d,e,a,a); X(b,d,d,a,e,a); X(b,d,d,a,a,e);
													X(b,d,a,e,d,a); X(b,d,a,e,a,d); X(b,d,a,d,e,a);
													X(b,d,a,d,a,e); X(b,d,a,a,e,d); X(b,d,a,a,d,e);
													X(b,a,e,d,d,a); X(b,a,e,d,a,d); X(b,a,e,a,d,d);
													X(b,a,d,e,d,a); X(b,a,d,e,a,d); X(b,a,d,d,e,a);
													X(b,a,d,d,a,e); X(b,a,d,a,e,d); X(b,a,d,a,d,e);
													X(b,a,a,e,d,d); X(b,a,a,d,e,d); X(b,a,a,d,d,e);
													X(a,e,d,d,b,a); X(a,e,d,d,a,b); X(a,e,d,b,d,a);
													X(a,e,d,b,a,d); X(a,e,d,a,d,b); X(a,e,d,a,b,d);
													X(a,e,b,d,d,a); X(a,e,b,d,a,d); X(a,e,b,a,d,d);
													X(a,e,a,d,d,b); X(a,e,a,d,b,d); X(a,e,a,b,d,d);
													X(a,d,e,d,b,a); X(a,d,e,d,a,b); X(a,d,e,b,d,a);
													X(a,d,e,b,a,d); X(a,d,e,a,d,b); X(a,d,e,a,b,d);
													X(a,d,d,e,b,a); X(a,d,d,e,a,b); X(a,d,d,b,e,a);
													X(a,d,d,b,a,e); X(a,d,d,a,e,b); X(a,d,d,a,b,e);
													X(a,d,b,e,d,a); X(a,d,b,e,a,d); X(a,d,b,d,e,a);
													X(a,d,b,d,a,e); X(a,d,b,a,e,d); X(a,d,b,a,d,e);
													X(a,d,a,e,d,b); X(a,d,a,e,b,d); X(a,d,a,d,e,b);
													X(a,d,a,d,b,e); X(a,d,a,b,e,d); X(a,d,a,b,d,e);
													X(a,b,e,d,d,a); X(a,b,e,d,a,d); X(a,b,e,a,d,d);
													X(a,b,d,e,d,a); X(a,b,d,e,a,d); X(a,b,d,d,e,a);
													X(a,b,d,d,a,e); X(a,b,d,a,e,d); X(a,b,d,a,d,e);
													X(a,b,a,e,d,d); X(a,b,a,d,e,d); X(a,b,a,d,d,e);
													X(a,a,e,d,d,b); X(a,a,e,d,b,d); X(a,a,e,b,d,d);
													X(a,a,d,e,d,b); X(a,a,d,e,b,d); X(a,a,d,d,e,b);
													X(a,a,d,d,b,e); X(a,a,d,b,e,d); X(a,a,d,b,d,e);
													X(a,a,b,e,d,d); X(a,a,b,d,e,d); X(a,a,b,d,d,e);
												} else {
													X(d,d,d,b,a,a); X(d,d,d,a,b,a); X(d,d,d,a,a,b);
													X(d,d,b,d,a,a); X(d,d,b,a,d,a); X(d,d,b,a,a,d);
													X(d,d,a,d,b,a); X(d,d,a,d,a,b); X(d,d,a,b,d,a);
													X(d,d,a,b,a,d); X(d,d,a,a,d,b); X(d,d,a,a,b,d);
													X(d,b,d,d,a,a); X(d,b,d,a,d,a); X(d,b,d,a,a,d);
													X(d,b,a,d,d,a); X(d,b,a,d,a,d); X(d,b,a,a,d,d);
													X(d,a,d,d,b,a); X(d,a,d,d,a,b); X(d,a,d,b,d,a);
													X(d,a,d,b,a,d); X(d,a,d,a,d,b); X(d,a,d,a,b,d);
													X(d,a,b,d,d,a); X(d,a,b,d,a,d); X(d,a,b,a,d,d);
													X(d,a,a,d,d,b); X(d,a,a,d,b,d); X(d,a,a,b,d,d);
													X(b,d,d,d,a,a); X(b,d,d,a,d,a); X(b,d,d,a,a,d);
													X(b,d,a,d,d,a); X(b,d,a,d,a,d); X(b,d,a,a,d,d);
													X(b,a,d,d,d,a); X(b,a,d,d,a,d); X(b,a,d,a,d,d);
													X(b,a,a,d,d,d); X(a,d,d,d,b,a); X(a,d,d,d,a,b);
													X(a,d,d,b,d,a); X(a,d,d,b,a,d); X(a,d,d,a,d,b);
													X(a,d,d,a,b,d); X(a,d,b,d,d,a); X(a,d,b,d,a,d);
													X(a,d,b,a,d,d); X(a,d,a,d,d,b); X(a,d,a,d,b,d);
													X(a,d,a,b,d,d); X(a,b,d,d,d,a); X(a,b,d,d,a,d);
													X(a,b,d,a,d,d); X(a,b,a,d,d,d); X(a,a,d,d,d,b);
													X(a,a,d,d,b,d); X(a,a,d,b,d,d); X(a,a,b,d,d,d);
												}
											}
										} else {
											// c != d
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												if (auto ep = dp * (e + 2); d != e) {
													DECLARE_POSITION_VALUES(e);
													X(e,d,c,b,a,a); X(e,d,c,a,b,a); X(e,d,c,a,a,b);
													X(e,d,b,c,a,a); X(e,d,b,a,c,a); X(e,d,b,a,a,c);
													X(e,d,a,c,b,a); X(e,d,a,c,a,b); X(e,d,a,b,c,a);
													X(e,d,a,b,a,c); X(e,d,a,a,c,b); X(e,d,a,a,b,c);
													X(e,c,d,b,a,a); X(e,c,d,a,b,a); X(e,c,d,a,a,b);
													X(e,c,b,d,a,a); X(e,c,b,a,d,a); X(e,c,b,a,a,d);
													X(e,c,a,d,b,a); X(e,c,a,d,a,b); X(e,c,a,b,d,a);
													X(e,c,a,b,a,d); X(e,c,a,a,d,b); X(e,c,a,a,b,d);
													X(e,b,d,c,a,a); X(e,b,d,a,c,a); X(e,b,d,a,a,c);
													X(e,b,c,d,a,a); X(e,b,c,a,d,a); X(e,b,c,a,a,d);
													X(e,b,a,d,c,a); X(e,b,a,d,a,c); X(e,b,a,c,d,a);
													X(e,b,a,c,a,d); X(e,b,a,a,d,c); X(e,b,a,a,c,d);
													X(e,a,d,c,b,a); X(e,a,d,c,a,b); X(e,a,d,b,c,a);
													X(e,a,d,b,a,c); X(e,a,d,a,c,b); X(e,a,d,a,b,c);
													X(e,a,c,d,b,a); X(e,a,c,d,a,b); X(e,a,c,b,d,a);
													X(e,a,c,b,a,d); X(e,a,c,a,d,b); X(e,a,c,a,b,d);
													X(e,a,b,d,c,a); X(e,a,b,d,a,c); X(e,a,b,c,d,a);
													X(e,a,b,c,a,d); X(e,a,b,a,d,c); X(e,a,b,a,c,d);
													X(e,a,a,d,c,b); X(e,a,a,d,b,c); X(e,a,a,c,d,b);
													X(e,a,a,c,b,d); X(e,a,a,b,d,c); X(e,a,a,b,c,d);
													X(d,e,c,b,a,a); X(d,e,c,a,b,a); X(d,e,c,a,a,b);
													X(d,e,b,c,a,a); X(d,e,b,a,c,a); X(d,e,b,a,a,c);
													X(d,e,a,c,b,a); X(d,e,a,c,a,b); X(d,e,a,b,c,a);
													X(d,e,a,b,a,c); X(d,e,a,a,c,b); X(d,e,a,a,b,c);
													X(d,c,e,b,a,a); X(d,c,e,a,b,a); X(d,c,e,a,a,b);
													X(d,c,b,e,a,a); X(d,c,b,a,e,a); X(d,c,b,a,a,e);
													X(d,c,a,e,b,a); X(d,c,a,e,a,b); X(d,c,a,b,e,a);
													X(d,c,a,b,a,e); X(d,c,a,a,e,b); X(d,c,a,a,b,e);
													X(d,b,e,c,a,a); X(d,b,e,a,c,a); X(d,b,e,a,a,c);
													X(d,b,c,e,a,a); X(d,b,c,a,e,a); X(d,b,c,a,a,e);
													X(d,b,a,e,c,a); X(d,b,a,e,a,c); X(d,b,a,c,e,a);
													X(d,b,a,c,a,e); X(d,b,a,a,e,c); X(d,b,a,a,c,e);
													X(d,a,e,c,b,a); X(d,a,e,c,a,b); X(d,a,e,b,c,a);
													X(d,a,e,b,a,c); X(d,a,e,a,c,b); X(d,a,e,a,b,c);
													X(d,a,c,e,b,a); X(d,a,c,e,a,b); X(d,a,c,b,e,a);
													X(d,a,c,b,a,e); X(d,a,c,a,e,b); X(d,a,c,a,b,e);
													X(d,a,b,e,c,a); X(d,a,b,e,a,c); X(d,a,b,c,e,a);
													X(d,a,b,c,a,e); X(d,a,b,a,e,c); X(d,a,b,a,c,e);
													X(d,a,a,e,c,b); X(d,a,a,e,b,c); X(d,a,a,c,e,b);
													X(d,a,a,c,b,e); X(d,a,a,b,e,c); X(d,a,a,b,c,e);
													X(c,e,d,b,a,a); X(c,e,d,a,b,a); X(c,e,d,a,a,b);
													X(c,e,b,d,a,a); X(c,e,b,a,d,a); X(c,e,b,a,a,d);
													X(c,e,a,d,b,a); X(c,e,a,d,a,b); X(c,e,a,b,d,a);
													X(c,e,a,b,a,d); X(c,e,a,a,d,b); X(c,e,a,a,b,d);
													X(c,d,e,b,a,a); X(c,d,e,a,b,a); X(c,d,e,a,a,b);
													X(c,d,b,e,a,a); X(c,d,b,a,e,a); X(c,d,b,a,a,e);
													X(c,d,a,e,b,a); X(c,d,a,e,a,b); X(c,d,a,b,e,a);
													X(c,d,a,b,a,e); X(c,d,a,a,e,b); X(c,d,a,a,b,e);
													X(c,b,e,d,a,a); X(c,b,e,a,d,a); X(c,b,e,a,a,d);
													X(c,b,d,e,a,a); X(c,b,d,a,e,a); X(c,b,d,a,a,e);
													X(c,b,a,e,d,a); X(c,b,a,e,a,d); X(c,b,a,d,e,a);
													X(c,b,a,d,a,e); X(c,b,a,a,e,d); X(c,b,a,a,d,e);
													X(c,a,e,d,b,a); X(c,a,e,d,a,b); X(c,a,e,b,d,a);
													X(c,a,e,b,a,d); X(c,a,e,a,d,b); X(c,a,e,a,b,d);
													X(c,a,d,e,b,a); X(c,a,d,e,a,b); X(c,a,d,b,e,a);
													X(c,a,d,b,a,e); X(c,a,d,a,e,b); X(c,a,d,a,b,e);
													X(c,a,b,e,d,a); X(c,a,b,e,a,d); X(c,a,b,d,e,a);
													X(c,a,b,d,a,e); X(c,a,b,a,e,d); X(c,a,b,a,d,e);
													X(c,a,a,e,d,b); X(c,a,a,e,b,d); X(c,a,a,d,e,b);
													X(c,a,a,d,b,e); X(c,a,a,b,e,d); X(c,a,a,b,d,e);
													X(b,e,d,c,a,a); X(b,e,d,a,c,a); X(b,e,d,a,a,c);
													X(b,e,c,d,a,a); X(b,e,c,a,d,a); X(b,e,c,a,a,d);
													X(b,e,a,d,c,a); X(b,e,a,d,a,c); X(b,e,a,c,d,a);
													X(b,e,a,c,a,d); X(b,e,a,a,d,c); X(b,e,a,a,c,d);
													X(b,d,e,c,a,a); X(b,d,e,a,c,a); X(b,d,e,a,a,c);
													X(b,d,c,e,a,a); X(b,d,c,a,e,a); X(b,d,c,a,a,e);
													X(b,d,a,e,c,a); X(b,d,a,e,a,c); X(b,d,a,c,e,a);
													X(b,d,a,c,a,e); X(b,d,a,a,e,c); X(b,d,a,a,c,e);
													X(b,c,e,d,a,a); X(b,c,e,a,d,a); X(b,c,e,a,a,d);
													X(b,c,d,e,a,a); X(b,c,d,a,e,a); X(b,c,d,a,a,e);
													X(b,c,a,e,d,a); X(b,c,a,e,a,d); X(b,c,a,d,e,a);
													X(b,c,a,d,a,e); X(b,c,a,a,e,d); X(b,c,a,a,d,e);
													X(b,a,e,d,c,a); X(b,a,e,d,a,c); X(b,a,e,c,d,a);
													X(b,a,e,c,a,d); X(b,a,e,a,d,c); X(b,a,e,a,c,d);
													X(b,a,d,e,c,a); X(b,a,d,e,a,c); X(b,a,d,c,e,a);
													X(b,a,d,c,a,e); X(b,a,d,a,e,c); X(b,a,d,a,c,e);
													X(b,a,c,e,d,a); X(b,a,c,e,a,d); X(b,a,c,d,e,a);
													X(b,a,c,d,a,e); X(b,a,c,a,e,d); X(b,a,c,a,d,e);
													X(b,a,a,e,d,c); X(b,a,a,e,c,d); X(b,a,a,d,e,c);
													X(b,a,a,d,c,e); X(b,a,a,c,e,d); X(b,a,a,c,d,e);
													X(a,e,d,c,b,a); X(a,e,d,c,a,b); X(a,e,d,b,c,a);
													X(a,e,d,b,a,c); X(a,e,d,a,c,b); X(a,e,d,a,b,c);
													X(a,e,c,d,b,a); X(a,e,c,d,a,b); X(a,e,c,b,d,a);
													X(a,e,c,b,a,d); X(a,e,c,a,d,b); X(a,e,c,a,b,d);
													X(a,e,b,d,c,a); X(a,e,b,d,a,c); X(a,e,b,c,d,a);
													X(a,e,b,c,a,d); X(a,e,b,a,d,c); X(a,e,b,a,c,d);
													X(a,e,a,d,c,b); X(a,e,a,d,b,c); X(a,e,a,c,d,b);
													X(a,e,a,c,b,d); X(a,e,a,b,d,c); X(a,e,a,b,c,d);
													X(a,d,e,c,b,a); X(a,d,e,c,a,b); X(a,d,e,b,c,a);
													X(a,d,e,b,a,c); X(a,d,e,a,c,b); X(a,d,e,a,b,c);
													X(a,d,c,e,b,a); X(a,d,c,e,a,b); X(a,d,c,b,e,a);
													X(a,d,c,b,a,e); X(a,d,c,a,e,b); X(a,d,c,a,b,e);
													X(a,d,b,e,c,a); X(a,d,b,e,a,c); X(a,d,b,c,e,a);
													X(a,d,b,c,a,e); X(a,d,b,a,e,c); X(a,d,b,a,c,e);
													X(a,d,a,e,c,b); X(a,d,a,e,b,c); X(a,d,a,c,e,b);
													X(a,d,a,c,b,e); X(a,d,a,b,e,c); X(a,d,a,b,c,e);
													X(a,c,e,d,b,a); X(a,c,e,d,a,b); X(a,c,e,b,d,a);
													X(a,c,e,b,a,d); X(a,c,e,a,d,b); X(a,c,e,a,b,d);
													X(a,c,d,e,b,a); X(a,c,d,e,a,b); X(a,c,d,b,e,a);
													X(a,c,d,b,a,e); X(a,c,d,a,e,b); X(a,c,d,a,b,e);
													X(a,c,b,e,d,a); X(a,c,b,e,a,d); X(a,c,b,d,e,a);
													X(a,c,b,d,a,e); X(a,c,b,a,e,d); X(a,c,b,a,d,e);
													X(a,c,a,e,d,b); X(a,c,a,e,b,d); X(a,c,a,d,e,b);
													X(a,c,a,d,b,e); X(a,c,a,b,e,d); X(a,c,a,b,d,e);
													X(a,b,e,d,c,a); X(a,b,e,d,a,c); X(a,b,e,c,d,a);
													X(a,b,e,c,a,d); X(a,b,e,a,d,c); X(a,b,e,a,c,d);
													X(a,b,d,e,c,a); X(a,b,d,e,a,c); X(a,b,d,c,e,a);
													X(a,b,d,c,a,e); X(a,b,d,a,e,c); X(a,b,d,a,c,e);
													X(a,b,c,e,d,a); X(a,b,c,e,a,d); X(a,b,c,d,e,a);
													X(a,b,c,d,a,e); X(a,b,c,a,e,d); X(a,b,c,a,d,e);
													X(a,b,a,e,d,c); X(a,b,a,e,c,d); X(a,b,a,d,e,c);
													X(a,b,a,d,c,e); X(a,b,a,c,e,d); X(a,b,a,c,d,e);
													X(a,a,e,d,c,b); X(a,a,e,d,b,c); X(a,a,e,c,d,b);
													X(a,a,e,c,b,d); X(a,a,e,b,d,c); X(a,a,e,b,c,d);
													X(a,a,d,e,c,b); X(a,a,d,e,b,c); X(a,a,d,c,e,b);
													X(a,a,d,c,b,e); X(a,a,d,b,e,c); X(a,a,d,b,c,e);
													X(a,a,c,e,d,b); X(a,a,c,e,b,d); X(a,a,c,d,e,b);
													X(a,a,c,d,b,e); X(a,a,c,b,e,d); X(a,a,c,b,d,e);
													X(a,a,b,e,d,c); X(a,a,b,e,c,d); X(a,a,b,d,e,c);
													X(a,a,b,d,c,e); X(a,a,b,c,e,d); X(a,a,b,c,d,e);
												} else {
													X(d,d,c,b,a,a); X(d,d,c,a,b,a); X(d,d,c,a,a,b);
													X(d,d,b,c,a,a); X(d,d,b,a,c,a); X(d,d,b,a,a,c);
													X(d,d,a,c,b,a); X(d,d,a,c,a,b); X(d,d,a,b,c,a);
													X(d,d,a,b,a,c); X(d,d,a,a,c,b); X(d,d,a,a,b,c);
													X(d,c,d,b,a,a); X(d,c,d,a,b,a); X(d,c,d,a,a,b);
													X(d,c,b,d,a,a); X(d,c,b,a,d,a); X(d,c,b,a,a,d);
													X(d,c,a,d,b,a); X(d,c,a,d,a,b); X(d,c,a,b,d,a);
													X(d,c,a,b,a,d); X(d,c,a,a,d,b); X(d,c,a,a,b,d);
													X(d,b,d,c,a,a); X(d,b,d,a,c,a); X(d,b,d,a,a,c);
													X(d,b,c,d,a,a); X(d,b,c,a,d,a); X(d,b,c,a,a,d);
													X(d,b,a,d,c,a); X(d,b,a,d,a,c); X(d,b,a,c,d,a);
													X(d,b,a,c,a,d); X(d,b,a,a,d,c); X(d,b,a,a,c,d);
													X(d,a,d,c,b,a); X(d,a,d,c,a,b); X(d,a,d,b,c,a);
													X(d,a,d,b,a,c); X(d,a,d,a,c,b); X(d,a,d,a,b,c);
													X(d,a,c,d,b,a); X(d,a,c,d,a,b); X(d,a,c,b,d,a);
													X(d,a,c,b,a,d); X(d,a,c,a,d,b); X(d,a,c,a,b,d);
													X(d,a,b,d,c,a); X(d,a,b,d,a,c); X(d,a,b,c,d,a);
													X(d,a,b,c,a,d); X(d,a,b,a,d,c); X(d,a,b,a,c,d);
													X(d,a,a,d,c,b); X(d,a,a,d,b,c); X(d,a,a,c,d,b);
													X(d,a,a,c,b,d); X(d,a,a,b,d,c); X(d,a,a,b,c,d);
													X(c,d,d,b,a,a); X(c,d,d,a,b,a); X(c,d,d,a,a,b);
													X(c,d,b,d,a,a); X(c,d,b,a,d,a); X(c,d,b,a,a,d);
													X(c,d,a,d,b,a); X(c,d,a,d,a,b); X(c,d,a,b,d,a);
													X(c,d,a,b,a,d); X(c,d,a,a,d,b); X(c,d,a,a,b,d);
													X(c,b,d,d,a,a); X(c,b,d,a,d,a); X(c,b,d,a,a,d);
													X(c,b,a,d,d,a); X(c,b,a,d,a,d); X(c,b,a,a,d,d);
													X(c,a,d,d,b,a); X(c,a,d,d,a,b); X(c,a,d,b,d,a);
													X(c,a,d,b,a,d); X(c,a,d,a,d,b); X(c,a,d,a,b,d);
													X(c,a,b,d,d,a); X(c,a,b,d,a,d); X(c,a,b,a,d,d);
													X(c,a,a,d,d,b); X(c,a,a,d,b,d); X(c,a,a,b,d,d);
													X(b,d,d,c,a,a); X(b,d,d,a,c,a); X(b,d,d,a,a,c);
													X(b,d,c,d,a,a); X(b,d,c,a,d,a); X(b,d,c,a,a,d);
													X(b,d,a,d,c,a); X(b,d,a,d,a,c); X(b,d,a,c,d,a);
													X(b,d,a,c,a,d); X(b,d,a,a,d,c); X(b,d,a,a,c,d);
													X(b,c,d,d,a,a); X(b,c,d,a,d,a); X(b,c,d,a,a,d);
													X(b,c,a,d,d,a); X(b,c,a,d,a,d); X(b,c,a,a,d,d);
													X(b,a,d,d,c,a); X(b,a,d,d,a,c); X(b,a,d,c,d,a);
													X(b,a,d,c,a,d); X(b,a,d,a,d,c); X(b,a,d,a,c,d);
													X(b,a,c,d,d,a); X(b,a,c,d,a,d); X(b,a,c,a,d,d);
													X(b,a,a,d,d,c); X(b,a,a,d,c,d); X(b,a,a,c,d,d);
													X(a,d,d,c,b,a); X(a,d,d,c,a,b); X(a,d,d,b,c,a);
													X(a,d,d,b,a,c); X(a,d,d,a,c,b); X(a,d,d,a,b,c);
													X(a,d,c,d,b,a); X(a,d,c,d,a,b); X(a,d,c,b,d,a);
													X(a,d,c,b,a,d); X(a,d,c,a,d,b); X(a,d,c,a,b,d);
													X(a,d,b,d,c,a); X(a,d,b,d,a,c); X(a,d,b,c,d,a);
													X(a,d,b,c,a,d); X(a,d,b,a,d,c); X(a,d,b,a,c,d);
													X(a,d,a,d,c,b); X(a,d,a,d,b,c); X(a,d,a,c,d,b);
													X(a,d,a,c,b,d); X(a,d,a,b,d,c); X(a,d,a,b,c,d);
													X(a,c,d,d,b,a); X(a,c,d,d,a,b); X(a,c,d,b,d,a);
													X(a,c,d,b,a,d); X(a,c,d,a,d,b); X(a,c,d,a,b,d);
													X(a,c,b,d,d,a); X(a,c,b,d,a,d); X(a,c,b,a,d,d);
													X(a,c,a,d,d,b); X(a,c,a,d,b,d); X(a,c,a,b,d,d);
													X(a,b,d,d,c,a); X(a,b,d,d,a,c); X(a,b,d,c,d,a);
													X(a,b,d,c,a,d); X(a,b,d,a,d,c); X(a,b,d,a,c,d);
													X(a,b,c,d,d,a); X(a,b,c,d,a,d); X(a,b,c,a,d,d);
													X(a,b,a,d,d,c); X(a,b,a,d,c,d); X(a,b,a,c,d,d);
													X(a,a,d,d,c,b); X(a,a,d,d,b,c); X(a,a,d,c,d,b);
													X(a,a,d,c,b,d); X(a,a,d,b,d,c); X(a,a,d,b,c,d);
													X(a,a,c,d,d,b); X(a,a,c,d,b,d); X(a,a,c,b,d,d);
													X(a,a,b,d,d,c); X(a,a,b,d,c,d); X(a,a,b,c,d,d);
												}
											}
										}
									}
								}
							}
						}
					}
				} else {
					// a != f
					for (auto b = a; b < 8ul; ++b) {
						SKIP5s(b);
						DECLARE_POSITION_VALUES(b);
						if (auto bs = as + b, bp = ap * (b + 2); a == b) {
							for (auto c = b; c < 8ul; ++c) {
								SKIP5s(c);
								DECLARE_POSITION_VALUES(c);
								if (auto cs = bs + c, cp = bp * (c + 2); b == c) {
									for (auto d = c; d < 8ul; ++d) {
										SKIP5s(d);
										DECLARE_POSITION_VALUES(d);
										if (auto ds = cs + d, dp = cp * (d + 2); c == d) {
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												if (auto ep = dp * (e + 2); d != e) {
													DECLARE_POSITION_VALUES(e);
													X(e,d,d,d,d,f); X(e,d,d,d,f,d); X(e,d,d,f,d,d);
													X(e,d,f,d,d,d); X(e,f,d,d,d,d); X(d,e,d,d,d,f);
													X(d,e,d,d,f,d); X(d,e,d,f,d,d); X(d,e,f,d,d,d);
													X(d,d,e,d,d,f); X(d,d,e,d,f,d); X(d,d,e,f,d,d);
													X(d,d,d,e,d,f); X(d,d,d,e,f,d); X(d,d,d,d,e,f);
													X(d,d,d,d,f,e); X(d,d,d,f,e,d); X(d,d,d,f,d,e);
													X(d,d,f,e,d,d); X(d,d,f,d,e,d); X(d,d,f,d,d,e);
													X(d,f,e,d,d,d); X(d,f,d,e,d,d); X(d,f,d,d,e,d);
													X(d,f,d,d,d,e); X(f,e,d,d,d,d); X(f,d,e,d,d,d);
													X(f,d,d,e,d,d); X(f,d,d,d,e,d); X(f,d,d,d,d,e);
												} else {
													X(d,d,d,d,d,f); X(d,d,d,d,f,d); X(d,d,d,f,d,d);
													X(d,d,f,d,d,d); X(d,f,d,d,d,d); X(f,d,d,d,d,d);
												}
											}
										} else {
											// c != d
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												if (auto ep = dp * (e + 2); d != e) {
													DECLARE_POSITION_VALUES(e);
													X(e,d,c,c,c,f); X(e,d,c,c,f,c); X(e,d,c,f,c,c);
													X(e,d,f,c,c,c); X(e,c,d,c,c,f); X(e,c,d,c,f,c);
													X(e,c,d,f,c,c); X(e,c,c,d,c,f); X(e,c,c,d,f,c);
													X(e,c,c,c,d,f); X(e,c,c,c,f,d); X(e,c,c,f,d,c);
													X(e,c,c,f,c,d); X(e,c,f,d,c,c); X(e,c,f,c,d,c);
													X(e,c,f,c,c,d); X(e,f,d,c,c,c); X(e,f,c,d,c,c);
													X(e,f,c,c,d,c); X(e,f,c,c,c,d); X(d,e,c,c,c,f);
													X(d,e,c,c,f,c); X(d,e,c,f,c,c); X(d,e,f,c,c,c);
													X(d,c,e,c,c,f); X(d,c,e,c,f,c); X(d,c,e,f,c,c);
													X(d,c,c,e,c,f); X(d,c,c,e,f,c); X(d,c,c,c,e,f);
													X(d,c,c,c,f,e); X(d,c,c,f,e,c); X(d,c,c,f,c,e);
													X(d,c,f,e,c,c); X(d,c,f,c,e,c); X(d,c,f,c,c,e);
													X(d,f,e,c,c,c); X(d,f,c,e,c,c); X(d,f,c,c,e,c);
													X(d,f,c,c,c,e); X(c,e,d,c,c,f); X(c,e,d,c,f,c);
													X(c,e,d,f,c,c); X(c,e,c,d,c,f); X(c,e,c,d,f,c);
													X(c,e,c,c,d,f); X(c,e,c,c,f,d); X(c,e,c,f,d,c);
													X(c,e,c,f,c,d); X(c,e,f,d,c,c); X(c,e,f,c,d,c);
													X(c,e,f,c,c,d); X(c,d,e,c,c,f); X(c,d,e,c,f,c);
													X(c,d,e,f,c,c); X(c,d,c,e,c,f); X(c,d,c,e,f,c);
													X(c,d,c,c,e,f); X(c,d,c,c,f,e); X(c,d,c,f,e,c);
													X(c,d,c,f,c,e); X(c,d,f,e,c,c); X(c,d,f,c,e,c);
													X(c,d,f,c,c,e); X(c,c,e,d,c,f); X(c,c,e,d,f,c);
													X(c,c,e,c,d,f); X(c,c,e,c,f,d); X(c,c,e,f,d,c);
													X(c,c,e,f,c,d); X(c,c,d,e,c,f); X(c,c,d,e,f,c);
													X(c,c,d,c,e,f); X(c,c,d,c,f,e); X(c,c,d,f,e,c);
													X(c,c,d,f,c,e); X(c,c,c,e,d,f); X(c,c,c,e,f,d);
													X(c,c,c,d,e,f); X(c,c,c,d,f,e); X(c,c,c,f,e,d);
													X(c,c,c,f,d,e); X(c,c,f,e,d,c); X(c,c,f,e,c,d);
													X(c,c,f,d,e,c); X(c,c,f,d,c,e); X(c,c,f,c,e,d);
													X(c,c,f,c,d,e); X(c,f,e,d,c,c); X(c,f,e,c,d,c);
													X(c,f,e,c,c,d); X(c,f,d,e,c,c); X(c,f,d,c,e,c);
													X(c,f,d,c,c,e); X(c,f,c,e,d,c); X(c,f,c,e,c,d);
													X(c,f,c,d,e,c); X(c,f,c,d,c,e); X(c,f,c,c,e,d);
													X(c,f,c,c,d,e); X(f,e,d,c,c,c); X(f,e,c,d,c,c);
													X(f,e,c,c,d,c); X(f,e,c,c,c,d); X(f,d,e,c,c,c);
													X(f,d,c,e,c,c); X(f,d,c,c,e,c); X(f,d,c,c,c,e);
													X(f,c,e,d,c,c); X(f,c,e,c,d,c); X(f,c,e,c,c,d);
													X(f,c,d,e,c,c); X(f,c,d,c,e,c); X(f,c,d,c,c,e);
													X(f,c,c,e,d,c); X(f,c,c,e,c,d); X(f,c,c,d,e,c);
													X(f,c,c,d,c,e); X(f,c,c,c,e,d); X(f,c,c,c,d,e);
												} else {
													X(d,d,c,c,c,f); X(d,d,c,c,f,c); X(d,d,c,f,c,c);
													X(d,d,f,c,c,c); X(d,c,d,c,c,f); X(d,c,d,c,f,c);
													X(d,c,d,f,c,c); X(d,c,c,d,c,f); X(d,c,c,d,f,c);
													X(d,c,c,c,d,f); X(d,c,c,c,f,d); X(d,c,c,f,d,c);
													X(d,c,c,f,c,d); X(d,c,f,d,c,c); X(d,c,f,c,d,c);
													X(d,c,f,c,c,d); X(d,f,d,c,c,c); X(d,f,c,d,c,c);
													X(d,f,c,c,d,c); X(d,f,c,c,c,d); X(c,d,d,c,c,f);
													X(c,d,d,c,f,c); X(c,d,d,f,c,c); X(c,d,c,d,c,f);
													X(c,d,c,d,f,c); X(c,d,c,c,d,f); X(c,d,c,c,f,d);
													X(c,d,c,f,d,c); X(c,d,c,f,c,d); X(c,d,f,d,c,c);
													X(c,d,f,c,d,c); X(c,d,f,c,c,d); X(c,c,d,d,c,f);
													X(c,c,d,d,f,c); X(c,c,d,c,d,f); X(c,c,d,c,f,d);
													X(c,c,d,f,d,c); X(c,c,d,f,c,d); X(c,c,c,d,d,f);
													X(c,c,c,d,f,d); X(c,c,c,f,d,d); X(c,c,f,d,d,c);
													X(c,c,f,d,c,d); X(c,c,f,c,d,d); X(c,f,d,d,c,c);
													X(c,f,d,c,d,c); X(c,f,d,c,c,d); X(c,f,c,d,d,c);
													X(c,f,c,d,c,d); X(c,f,c,c,d,d); X(f,d,d,c,c,c);
													X(f,d,c,d,c,c); X(f,d,c,c,d,c); X(f,d,c,c,c,d);
													X(f,c,d,d,c,c); X(f,c,d,c,d,c); X(f,c,d,c,c,d);
													X(f,c,c,d,d,c); X(f,c,c,d,c,d); X(f,c,c,c,d,d);
												}
											}
										}
									}
								} else {
									// b != c
									for (auto d = c; d < 8ul; ++d) {
										SKIP5s(d);
										DECLARE_POSITION_VALUES(d);
										if (auto ds = cs + d, dp = cp * (d + 2); c == d) {
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												if (auto ep = dp * (e + 2); d != e) {
													DECLARE_POSITION_VALUES(e);
													X(e,d,d,b,b,f); X(e,d,d,b,f,b); X(e,d,d,f,b,b);
													X(e,d,b,d,b,f); X(e,d,b,d,f,b); X(e,d,b,b,d,f);
													X(e,d,b,b,f,d); X(e,d,b,f,d,b); X(e,d,b,f,b,d);
													X(e,d,f,d,b,b); X(e,d,f,b,d,b); X(e,d,f,b,b,d);
													X(e,b,d,d,b,f); X(e,b,d,d,f,b); X(e,b,d,b,d,f);
													X(e,b,d,b,f,d); X(e,b,d,f,d,b); X(e,b,d,f,b,d);
													X(e,b,b,d,d,f); X(e,b,b,d,f,d); X(e,b,b,f,d,d);
													X(e,b,f,d,d,b); X(e,b,f,d,b,d); X(e,b,f,b,d,d);
													X(e,f,d,d,b,b); X(e,f,d,b,d,b); X(e,f,d,b,b,d);
													X(e,f,b,d,d,b); X(e,f,b,d,b,d); X(e,f,b,b,d,d);
													X(d,e,d,b,b,f); X(d,e,d,b,f,b); X(d,e,d,f,b,b);
													X(d,e,b,d,b,f); X(d,e,b,d,f,b); X(d,e,b,b,d,f);
													X(d,e,b,b,f,d); X(d,e,b,f,d,b); X(d,e,b,f,b,d);
													X(d,e,f,d,b,b); X(d,e,f,b,d,b); X(d,e,f,b,b,d);
													X(d,d,e,b,b,f); X(d,d,e,b,f,b); X(d,d,e,f,b,b);
													X(d,d,b,e,b,f); X(d,d,b,e,f,b); X(d,d,b,b,e,f);
													X(d,d,b,b,f,e); X(d,d,b,f,e,b); X(d,d,b,f,b,e);
													X(d,d,f,e,b,b); X(d,d,f,b,e,b); X(d,d,f,b,b,e);
													X(d,b,e,d,b,f); X(d,b,e,d,f,b); X(d,b,e,b,d,f);
													X(d,b,e,b,f,d); X(d,b,e,f,d,b); X(d,b,e,f,b,d);
													X(d,b,d,e,b,f); X(d,b,d,e,f,b); X(d,b,d,b,e,f);
													X(d,b,d,b,f,e); X(d,b,d,f,e,b); X(d,b,d,f,b,e);
													X(d,b,b,e,d,f); X(d,b,b,e,f,d); X(d,b,b,d,e,f);
													X(d,b,b,d,f,e); X(d,b,b,f,e,d); X(d,b,b,f,d,e);
													X(d,b,f,e,d,b); X(d,b,f,e,b,d); X(d,b,f,d,e,b);
													X(d,b,f,d,b,e); X(d,b,f,b,e,d); X(d,b,f,b,d,e);
													X(d,f,e,d,b,b); X(d,f,e,b,d,b); X(d,f,e,b,b,d);
													X(d,f,d,e,b,b); X(d,f,d,b,e,b); X(d,f,d,b,b,e);
													X(d,f,b,e,d,b); X(d,f,b,e,b,d); X(d,f,b,d,e,b);
													X(d,f,b,d,b,e); X(d,f,b,b,e,d); X(d,f,b,b,d,e);
													X(b,e,d,d,b,f); X(b,e,d,d,f,b); X(b,e,d,b,d,f);
													X(b,e,d,b,f,d); X(b,e,d,f,d,b); X(b,e,d,f,b,d);
													X(b,e,b,d,d,f); X(b,e,b,d,f,d); X(b,e,b,f,d,d);
													X(b,e,f,d,d,b); X(b,e,f,d,b,d); X(b,e,f,b,d,d);
													X(b,d,e,d,b,f); X(b,d,e,d,f,b); X(b,d,e,b,d,f);
													X(b,d,e,b,f,d); X(b,d,e,f,d,b); X(b,d,e,f,b,d);
													X(b,d,d,e,b,f); X(b,d,d,e,f,b); X(b,d,d,b,e,f);
													X(b,d,d,b,f,e); X(b,d,d,f,e,b); X(b,d,d,f,b,e);
													X(b,d,b,e,d,f); X(b,d,b,e,f,d); X(b,d,b,d,e,f);
													X(b,d,b,d,f,e); X(b,d,b,f,e,d); X(b,d,b,f,d,e);
													X(b,d,f,e,d,b); X(b,d,f,e,b,d); X(b,d,f,d,e,b);
													X(b,d,f,d,b,e); X(b,d,f,b,e,d); X(b,d,f,b,d,e);
													X(b,b,e,d,d,f); X(b,b,e,d,f,d); X(b,b,e,f,d,d);
													X(b,b,d,e,d,f); X(b,b,d,e,f,d); X(b,b,d,d,e,f);
													X(b,b,d,d,f,e); X(b,b,d,f,e,d); X(b,b,d,f,d,e);
													X(b,b,f,e,d,d); X(b,b,f,d,e,d); X(b,b,f,d,d,e);
													X(b,f,e,d,d,b); X(b,f,e,d,b,d); X(b,f,e,b,d,d);
													X(b,f,d,e,d,b); X(b,f,d,e,b,d); X(b,f,d,d,e,b);
													X(b,f,d,d,b,e); X(b,f,d,b,e,d); X(b,f,d,b,d,e);
													X(b,f,b,e,d,d); X(b,f,b,d,e,d); X(b,f,b,d,d,e);
													X(f,e,d,d,b,b); X(f,e,d,b,d,b); X(f,e,d,b,b,d);
													X(f,e,b,d,d,b); X(f,e,b,d,b,d); X(f,e,b,b,d,d);
													X(f,d,e,d,b,b); X(f,d,e,b,d,b); X(f,d,e,b,b,d);
													X(f,d,d,e,b,b); X(f,d,d,b,e,b); X(f,d,d,b,b,e);
													X(f,d,b,e,d,b); X(f,d,b,e,b,d); X(f,d,b,d,e,b);
													X(f,d,b,d,b,e); X(f,d,b,b,e,d); X(f,d,b,b,d,e);
													X(f,b,e,d,d,b); X(f,b,e,d,b,d); X(f,b,e,b,d,d);
													X(f,b,d,e,d,b); X(f,b,d,e,b,d); X(f,b,d,d,e,b);
													X(f,b,d,d,b,e); X(f,b,d,b,e,d); X(f,b,d,b,d,e);
													X(f,b,b,e,d,d); X(f,b,b,d,e,d); X(f,b,b,d,d,e);
												} else {
													X(d,d,d,b,b,f); X(d,d,d,b,f,b); X(d,d,d,f,b,b);
													X(d,d,b,d,b,f); X(d,d,b,d,f,b); X(d,d,b,b,d,f);
													X(d,d,b,b,f,d); X(d,d,b,f,d,b); X(d,d,b,f,b,d);
													X(d,d,f,d,b,b); X(d,d,f,b,d,b); X(d,d,f,b,b,d);
													X(d,b,d,d,b,f); X(d,b,d,d,f,b); X(d,b,d,b,d,f);
													X(d,b,d,b,f,d); X(d,b,d,f,d,b); X(d,b,d,f,b,d);
													X(d,b,b,d,d,f); X(d,b,b,d,f,d); X(d,b,b,f,d,d);
													X(d,b,f,d,d,b); X(d,b,f,d,b,d); X(d,b,f,b,d,d);
													X(d,f,d,d,b,b); X(d,f,d,b,d,b); X(d,f,d,b,b,d);
													X(d,f,b,d,d,b); X(d,f,b,d,b,d); X(d,f,b,b,d,d);
													X(b,d,d,d,b,f); X(b,d,d,d,f,b); X(b,d,d,b,d,f);
													X(b,d,d,b,f,d); X(b,d,d,f,d,b); X(b,d,d,f,b,d);
													X(b,d,b,d,d,f); X(b,d,b,d,f,d); X(b,d,b,f,d,d);
													X(b,d,f,d,d,b); X(b,d,f,d,b,d); X(b,d,f,b,d,d);
													X(b,b,d,d,d,f); X(b,b,d,d,f,d); X(b,b,d,f,d,d);
													X(b,b,f,d,d,d); X(b,f,d,d,d,b); X(b,f,d,d,b,d);
													X(b,f,d,b,d,d); X(b,f,b,d,d,d); X(f,d,d,d,b,b);
													X(f,d,d,b,d,b); X(f,d,d,b,b,d); X(f,d,b,d,d,b);
													X(f,d,b,d,b,d); X(f,d,b,b,d,d); X(f,b,d,d,d,b);
													X(f,b,d,d,b,d); X(f,b,d,b,d,d); X(f,b,b,d,d,d);
												}
											}
										} else {
											// c != d
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												if (auto ep = dp * (e + 2); d != e) {
													DECLARE_POSITION_VALUES(e);
													X(e,d,c,b,b,f); X(e,d,c,b,f,b); X(e,d,c,f,b,b);
													X(e,d,b,c,b,f); X(e,d,b,c,f,b); X(e,d,b,b,c,f);
													X(e,d,b,b,f,c); X(e,d,b,f,c,b); X(e,d,b,f,b,c);
													X(e,d,f,c,b,b); X(e,d,f,b,c,b); X(e,d,f,b,b,c);
													X(e,c,d,b,b,f); X(e,c,d,b,f,b); X(e,c,d,f,b,b);
													X(e,c,b,d,b,f); X(e,c,b,d,f,b); X(e,c,b,b,d,f);
													X(e,c,b,b,f,d); X(e,c,b,f,d,b); X(e,c,b,f,b,d);
													X(e,c,f,d,b,b); X(e,c,f,b,d,b); X(e,c,f,b,b,d);
													X(e,b,d,c,b,f); X(e,b,d,c,f,b); X(e,b,d,b,c,f);
													X(e,b,d,b,f,c); X(e,b,d,f,c,b); X(e,b,d,f,b,c);
													X(e,b,c,d,b,f); X(e,b,c,d,f,b); X(e,b,c,b,d,f);
													X(e,b,c,b,f,d); X(e,b,c,f,d,b); X(e,b,c,f,b,d);
													X(e,b,b,d,c,f); X(e,b,b,d,f,c); X(e,b,b,c,d,f);
													X(e,b,b,c,f,d); X(e,b,b,f,d,c); X(e,b,b,f,c,d);
													X(e,b,f,d,c,b); X(e,b,f,d,b,c); X(e,b,f,c,d,b);
													X(e,b,f,c,b,d); X(e,b,f,b,d,c); X(e,b,f,b,c,d);
													X(e,f,d,c,b,b); X(e,f,d,b,c,b); X(e,f,d,b,b,c);
													X(e,f,c,d,b,b); X(e,f,c,b,d,b); X(e,f,c,b,b,d);
													X(e,f,b,d,c,b); X(e,f,b,d,b,c); X(e,f,b,c,d,b);
													X(e,f,b,c,b,d); X(e,f,b,b,d,c); X(e,f,b,b,c,d);
													X(d,e,c,b,b,f); X(d,e,c,b,f,b); X(d,e,c,f,b,b);
													X(d,e,b,c,b,f); X(d,e,b,c,f,b); X(d,e,b,b,c,f);
													X(d,e,b,b,f,c); X(d,e,b,f,c,b); X(d,e,b,f,b,c);
													X(d,e,f,c,b,b); X(d,e,f,b,c,b); X(d,e,f,b,b,c);
													X(d,c,e,b,b,f); X(d,c,e,b,f,b); X(d,c,e,f,b,b);
													X(d,c,b,e,b,f); X(d,c,b,e,f,b); X(d,c,b,b,e,f);
													X(d,c,b,b,f,e); X(d,c,b,f,e,b); X(d,c,b,f,b,e);
													X(d,c,f,e,b,b); X(d,c,f,b,e,b); X(d,c,f,b,b,e);
													X(d,b,e,c,b,f); X(d,b,e,c,f,b); X(d,b,e,b,c,f);
													X(d,b,e,b,f,c); X(d,b,e,f,c,b); X(d,b,e,f,b,c);
													X(d,b,c,e,b,f); X(d,b,c,e,f,b); X(d,b,c,b,e,f);
													X(d,b,c,b,f,e); X(d,b,c,f,e,b); X(d,b,c,f,b,e);
													X(d,b,b,e,c,f); X(d,b,b,e,f,c); X(d,b,b,c,e,f);
													X(d,b,b,c,f,e); X(d,b,b,f,e,c); X(d,b,b,f,c,e);
													X(d,b,f,e,c,b); X(d,b,f,e,b,c); X(d,b,f,c,e,b);
													X(d,b,f,c,b,e); X(d,b,f,b,e,c); X(d,b,f,b,c,e);
													X(d,f,e,c,b,b); X(d,f,e,b,c,b); X(d,f,e,b,b,c);
													X(d,f,c,e,b,b); X(d,f,c,b,e,b); X(d,f,c,b,b,e);
													X(d,f,b,e,c,b); X(d,f,b,e,b,c); X(d,f,b,c,e,b);
													X(d,f,b,c,b,e); X(d,f,b,b,e,c); X(d,f,b,b,c,e);
													X(c,e,d,b,b,f); X(c,e,d,b,f,b); X(c,e,d,f,b,b);
													X(c,e,b,d,b,f); X(c,e,b,d,f,b); X(c,e,b,b,d,f);
													X(c,e,b,b,f,d); X(c,e,b,f,d,b); X(c,e,b,f,b,d);
													X(c,e,f,d,b,b); X(c,e,f,b,d,b); X(c,e,f,b,b,d);
													X(c,d,e,b,b,f); X(c,d,e,b,f,b); X(c,d,e,f,b,b);
													X(c,d,b,e,b,f); X(c,d,b,e,f,b); X(c,d,b,b,e,f);
													X(c,d,b,b,f,e); X(c,d,b,f,e,b); X(c,d,b,f,b,e);
													X(c,d,f,e,b,b); X(c,d,f,b,e,b); X(c,d,f,b,b,e);
													X(c,b,e,d,b,f); X(c,b,e,d,f,b); X(c,b,e,b,d,f);
													X(c,b,e,b,f,d); X(c,b,e,f,d,b); X(c,b,e,f,b,d);
													X(c,b,d,e,b,f); X(c,b,d,e,f,b); X(c,b,d,b,e,f);
													X(c,b,d,b,f,e); X(c,b,d,f,e,b); X(c,b,d,f,b,e);
													X(c,b,b,e,d,f); X(c,b,b,e,f,d); X(c,b,b,d,e,f);
													X(c,b,b,d,f,e); X(c,b,b,f,e,d); X(c,b,b,f,d,e);
													X(c,b,f,e,d,b); X(c,b,f,e,b,d); X(c,b,f,d,e,b);
													X(c,b,f,d,b,e); X(c,b,f,b,e,d); X(c,b,f,b,d,e);
													X(c,f,e,d,b,b); X(c,f,e,b,d,b); X(c,f,e,b,b,d);
													X(c,f,d,e,b,b); X(c,f,d,b,e,b); X(c,f,d,b,b,e);
													X(c,f,b,e,d,b); X(c,f,b,e,b,d); X(c,f,b,d,e,b);
													X(c,f,b,d,b,e); X(c,f,b,b,e,d); X(c,f,b,b,d,e);
													X(b,e,d,c,b,f); X(b,e,d,c,f,b); X(b,e,d,b,c,f);
													X(b,e,d,b,f,c); X(b,e,d,f,c,b); X(b,e,d,f,b,c);
													X(b,e,c,d,b,f); X(b,e,c,d,f,b); X(b,e,c,b,d,f);
													X(b,e,c,b,f,d); X(b,e,c,f,d,b); X(b,e,c,f,b,d);
													X(b,e,b,d,c,f); X(b,e,b,d,f,c); X(b,e,b,c,d,f);
													X(b,e,b,c,f,d); X(b,e,b,f,d,c); X(b,e,b,f,c,d);
													X(b,e,f,d,c,b); X(b,e,f,d,b,c); X(b,e,f,c,d,b);
													X(b,e,f,c,b,d); X(b,e,f,b,d,c); X(b,e,f,b,c,d);
													X(b,d,e,c,b,f); X(b,d,e,c,f,b); X(b,d,e,b,c,f);
													X(b,d,e,b,f,c); X(b,d,e,f,c,b); X(b,d,e,f,b,c);
													X(b,d,c,e,b,f); X(b,d,c,e,f,b); X(b,d,c,b,e,f);
													X(b,d,c,b,f,e); X(b,d,c,f,e,b); X(b,d,c,f,b,e);
													X(b,d,b,e,c,f); X(b,d,b,e,f,c); X(b,d,b,c,e,f);
													X(b,d,b,c,f,e); X(b,d,b,f,e,c); X(b,d,b,f,c,e);
													X(b,d,f,e,c,b); X(b,d,f,e,b,c); X(b,d,f,c,e,b);
													X(b,d,f,c,b,e); X(b,d,f,b,e,c); X(b,d,f,b,c,e);
													X(b,c,e,d,b,f); X(b,c,e,d,f,b); X(b,c,e,b,d,f);
													X(b,c,e,b,f,d); X(b,c,e,f,d,b); X(b,c,e,f,b,d);
													X(b,c,d,e,b,f); X(b,c,d,e,f,b); X(b,c,d,b,e,f);
													X(b,c,d,b,f,e); X(b,c,d,f,e,b); X(b,c,d,f,b,e);
													X(b,c,b,e,d,f); X(b,c,b,e,f,d); X(b,c,b,d,e,f);
													X(b,c,b,d,f,e); X(b,c,b,f,e,d); X(b,c,b,f,d,e);
													X(b,c,f,e,d,b); X(b,c,f,e,b,d); X(b,c,f,d,e,b);
													X(b,c,f,d,b,e); X(b,c,f,b,e,d); X(b,c,f,b,d,e);
													X(b,b,e,d,c,f); X(b,b,e,d,f,c); X(b,b,e,c,d,f);
													X(b,b,e,c,f,d); X(b,b,e,f,d,c); X(b,b,e,f,c,d);
													X(b,b,d,e,c,f); X(b,b,d,e,f,c); X(b,b,d,c,e,f);
													X(b,b,d,c,f,e); X(b,b,d,f,e,c); X(b,b,d,f,c,e);
													X(b,b,c,e,d,f); X(b,b,c,e,f,d); X(b,b,c,d,e,f);
													X(b,b,c,d,f,e); X(b,b,c,f,e,d); X(b,b,c,f,d,e);
													X(b,b,f,e,d,c); X(b,b,f,e,c,d); X(b,b,f,d,e,c);
													X(b,b,f,d,c,e); X(b,b,f,c,e,d); X(b,b,f,c,d,e);
													X(b,f,e,d,c,b); X(b,f,e,d,b,c); X(b,f,e,c,d,b);
													X(b,f,e,c,b,d); X(b,f,e,b,d,c); X(b,f,e,b,c,d);
													X(b,f,d,e,c,b); X(b,f,d,e,b,c); X(b,f,d,c,e,b);
													X(b,f,d,c,b,e); X(b,f,d,b,e,c); X(b,f,d,b,c,e);
													X(b,f,c,e,d,b); X(b,f,c,e,b,d); X(b,f,c,d,e,b);
													X(b,f,c,d,b,e); X(b,f,c,b,e,d); X(b,f,c,b,d,e);
													X(b,f,b,e,d,c); X(b,f,b,e,c,d); X(b,f,b,d,e,c);
													X(b,f,b,d,c,e); X(b,f,b,c,e,d); X(b,f,b,c,d,e);
													X(f,e,d,c,b,b); X(f,e,d,b,c,b); X(f,e,d,b,b,c);
													X(f,e,c,d,b,b); X(f,e,c,b,d,b); X(f,e,c,b,b,d);
													X(f,e,b,d,c,b); X(f,e,b,d,b,c); X(f,e,b,c,d,b);
													X(f,e,b,c,b,d); X(f,e,b,b,d,c); X(f,e,b,b,c,d);
													X(f,d,e,c,b,b); X(f,d,e,b,c,b); X(f,d,e,b,b,c);
													X(f,d,c,e,b,b); X(f,d,c,b,e,b); X(f,d,c,b,b,e);
													X(f,d,b,e,c,b); X(f,d,b,e,b,c); X(f,d,b,c,e,b);
													X(f,d,b,c,b,e); X(f,d,b,b,e,c); X(f,d,b,b,c,e);
													X(f,c,e,d,b,b); X(f,c,e,b,d,b); X(f,c,e,b,b,d);
													X(f,c,d,e,b,b); X(f,c,d,b,e,b); X(f,c,d,b,b,e);
													X(f,c,b,e,d,b); X(f,c,b,e,b,d); X(f,c,b,d,e,b);
													X(f,c,b,d,b,e); X(f,c,b,b,e,d); X(f,c,b,b,d,e);
													X(f,b,e,d,c,b); X(f,b,e,d,b,c); X(f,b,e,c,d,b);
													X(f,b,e,c,b,d); X(f,b,e,b,d,c); X(f,b,e,b,c,d);
													X(f,b,d,e,c,b); X(f,b,d,e,b,c); X(f,b,d,c,e,b);
													X(f,b,d,c,b,e); X(f,b,d,b,e,c); X(f,b,d,b,c,e);
													X(f,b,c,e,d,b); X(f,b,c,e,b,d); X(f,b,c,d,e,b);
													X(f,b,c,d,b,e); X(f,b,c,b,e,d); X(f,b,c,b,d,e);
													X(f,b,b,e,d,c); X(f,b,b,e,c,d); X(f,b,b,d,e,c);
													X(f,b,b,d,c,e); X(f,b,b,c,e,d); X(f,b,b,c,d,e);
												} else {
													X(d,d,c,b,b,f); X(d,d,c,b,f,b); X(d,d,c,f,b,b);
													X(d,d,b,c,b,f); X(d,d,b,c,f,b); X(d,d,b,b,c,f);
													X(d,d,b,b,f,c); X(d,d,b,f,c,b); X(d,d,b,f,b,c);
													X(d,d,f,c,b,b); X(d,d,f,b,c,b); X(d,d,f,b,b,c);
													X(d,c,d,b,b,f); X(d,c,d,b,f,b); X(d,c,d,f,b,b);
													X(d,c,b,d,b,f); X(d,c,b,d,f,b); X(d,c,b,b,d,f);
													X(d,c,b,b,f,d); X(d,c,b,f,d,b); X(d,c,b,f,b,d);
													X(d,c,f,d,b,b); X(d,c,f,b,d,b); X(d,c,f,b,b,d);
													X(d,b,d,c,b,f); X(d,b,d,c,f,b); X(d,b,d,b,c,f);
													X(d,b,d,b,f,c); X(d,b,d,f,c,b); X(d,b,d,f,b,c);
													X(d,b,c,d,b,f); X(d,b,c,d,f,b); X(d,b,c,b,d,f);
													X(d,b,c,b,f,d); X(d,b,c,f,d,b); X(d,b,c,f,b,d);
													X(d,b,b,d,c,f); X(d,b,b,d,f,c); X(d,b,b,c,d,f);
													X(d,b,b,c,f,d); X(d,b,b,f,d,c); X(d,b,b,f,c,d);
													X(d,b,f,d,c,b); X(d,b,f,d,b,c); X(d,b,f,c,d,b);
													X(d,b,f,c,b,d); X(d,b,f,b,d,c); X(d,b,f,b,c,d);
													X(d,f,d,c,b,b); X(d,f,d,b,c,b); X(d,f,d,b,b,c);
													X(d,f,c,d,b,b); X(d,f,c,b,d,b); X(d,f,c,b,b,d);
													X(d,f,b,d,c,b); X(d,f,b,d,b,c); X(d,f,b,c,d,b);
													X(d,f,b,c,b,d); X(d,f,b,b,d,c); X(d,f,b,b,c,d);
													X(c,d,d,b,b,f); X(c,d,d,b,f,b); X(c,d,d,f,b,b);
													X(c,d,b,d,b,f); X(c,d,b,d,f,b); X(c,d,b,b,d,f);
													X(c,d,b,b,f,d); X(c,d,b,f,d,b); X(c,d,b,f,b,d);
													X(c,d,f,d,b,b); X(c,d,f,b,d,b); X(c,d,f,b,b,d);
													X(c,b,d,d,b,f); X(c,b,d,d,f,b); X(c,b,d,b,d,f);
													X(c,b,d,b,f,d); X(c,b,d,f,d,b); X(c,b,d,f,b,d);
													X(c,b,b,d,d,f); X(c,b,b,d,f,d); X(c,b,b,f,d,d);
													X(c,b,f,d,d,b); X(c,b,f,d,b,d); X(c,b,f,b,d,d);
													X(c,f,d,d,b,b); X(c,f,d,b,d,b); X(c,f,d,b,b,d);
													X(c,f,b,d,d,b); X(c,f,b,d,b,d); X(c,f,b,b,d,d);
													X(b,d,d,c,b,f); X(b,d,d,c,f,b); X(b,d,d,b,c,f);
													X(b,d,d,b,f,c); X(b,d,d,f,c,b); X(b,d,d,f,b,c);
													X(b,d,c,d,b,f); X(b,d,c,d,f,b); X(b,d,c,b,d,f);
													X(b,d,c,b,f,d); X(b,d,c,f,d,b); X(b,d,c,f,b,d);
													X(b,d,b,d,c,f); X(b,d,b,d,f,c); X(b,d,b,c,d,f);
													X(b,d,b,c,f,d); X(b,d,b,f,d,c); X(b,d,b,f,c,d);
													X(b,d,f,d,c,b); X(b,d,f,d,b,c); X(b,d,f,c,d,b);
													X(b,d,f,c,b,d); X(b,d,f,b,d,c); X(b,d,f,b,c,d);
													X(b,c,d,d,b,f); X(b,c,d,d,f,b); X(b,c,d,b,d,f);
													X(b,c,d,b,f,d); X(b,c,d,f,d,b); X(b,c,d,f,b,d);
													X(b,c,b,d,d,f); X(b,c,b,d,f,d); X(b,c,b,f,d,d);
													X(b,c,f,d,d,b); X(b,c,f,d,b,d); X(b,c,f,b,d,d);
													X(b,b,d,d,c,f); X(b,b,d,d,f,c); X(b,b,d,c,d,f);
													X(b,b,d,c,f,d); X(b,b,d,f,d,c); X(b,b,d,f,c,d);
													X(b,b,c,d,d,f); X(b,b,c,d,f,d); X(b,b,c,f,d,d);
													X(b,b,f,d,d,c); X(b,b,f,d,c,d); X(b,b,f,c,d,d);
													X(b,f,d,d,c,b); X(b,f,d,d,b,c); X(b,f,d,c,d,b);
													X(b,f,d,c,b,d); X(b,f,d,b,d,c); X(b,f,d,b,c,d);
													X(b,f,c,d,d,b); X(b,f,c,d,b,d); X(b,f,c,b,d,d);
													X(b,f,b,d,d,c); X(b,f,b,d,c,d); X(b,f,b,c,d,d);
													X(f,d,d,c,b,b); X(f,d,d,b,c,b); X(f,d,d,b,b,c);
													X(f,d,c,d,b,b); X(f,d,c,b,d,b); X(f,d,c,b,b,d);
													X(f,d,b,d,c,b); X(f,d,b,d,b,c); X(f,d,b,c,d,b);
													X(f,d,b,c,b,d); X(f,d,b,b,d,c); X(f,d,b,b,c,d);
													X(f,c,d,d,b,b); X(f,c,d,b,d,b); X(f,c,d,b,b,d);
													X(f,c,b,d,d,b); X(f,c,b,d,b,d); X(f,c,b,b,d,d);
													X(f,b,d,d,c,b); X(f,b,d,d,b,c); X(f,b,d,c,d,b);
													X(f,b,d,c,b,d); X(f,b,d,b,d,c); X(f,b,d,b,c,d);
													X(f,b,c,d,d,b); X(f,b,c,d,b,d); X(f,b,c,b,d,d);
													X(f,b,b,d,d,c); X(f,b,b,d,c,d); X(f,b,b,c,d,d);
												}
											}
										}
									}
								}
							}
						} else {
							// a != b
							for (auto c = b; c < 8ul; ++c) {
								SKIP5s(c);
								DECLARE_POSITION_VALUES(c);
								if (auto cs = bs + c, cp = bp * (c + 2); b == c) {
									for (auto d = c; d < 8ul; ++d) {
										SKIP5s(d);
										DECLARE_POSITION_VALUES(d);
										if (auto ds = cs + d, dp = cp * (d + 2); c == d) {
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												if (auto ep = dp * (e + 2); d != e) {
													DECLARE_POSITION_VALUES(e);
													X(e,d,d,d,a,f); X(e,d,d,d,f,a); X(e,d,d,a,d,f);
													X(e,d,d,a,f,d); X(e,d,d,f,d,a); X(e,d,d,f,a,d);
													X(e,d,a,d,d,f); X(e,d,a,d,f,d); X(e,d,a,f,d,d);
													X(e,d,f,d,d,a); X(e,d,f,d,a,d); X(e,d,f,a,d,d);
													X(e,a,d,d,d,f); X(e,a,d,d,f,d); X(e,a,d,f,d,d);
													X(e,a,f,d,d,d); X(e,f,d,d,d,a); X(e,f,d,d,a,d);
													X(e,f,d,a,d,d); X(e,f,a,d,d,d); X(d,e,d,d,a,f);
													X(d,e,d,d,f,a); X(d,e,d,a,d,f); X(d,e,d,a,f,d);
													X(d,e,d,f,d,a); X(d,e,d,f,a,d); X(d,e,a,d,d,f);
													X(d,e,a,d,f,d); X(d,e,a,f,d,d); X(d,e,f,d,d,a);
													X(d,e,f,d,a,d); X(d,e,f,a,d,d); X(d,d,e,d,a,f);
													X(d,d,e,d,f,a); X(d,d,e,a,d,f); X(d,d,e,a,f,d);
													X(d,d,e,f,d,a); X(d,d,e,f,a,d); X(d,d,d,e,a,f);
													X(d,d,d,e,f,a); X(d,d,d,a,e,f); X(d,d,d,a,f,e);
													X(d,d,d,f,e,a); X(d,d,d,f,a,e); X(d,d,a,e,d,f);
													X(d,d,a,e,f,d); X(d,d,a,d,e,f); X(d,d,a,d,f,e);
													X(d,d,a,f,e,d); X(d,d,a,f,d,e); X(d,d,f,e,d,a);
													X(d,d,f,e,a,d); X(d,d,f,d,e,a); X(d,d,f,d,a,e);
													X(d,d,f,a,e,d); X(d,d,f,a,d,e); X(d,a,e,d,d,f);
													X(d,a,e,d,f,d); X(d,a,e,f,d,d); X(d,a,d,e,d,f);
													X(d,a,d,e,f,d); X(d,a,d,d,e,f); X(d,a,d,d,f,e);
													X(d,a,d,f,e,d); X(d,a,d,f,d,e); X(d,a,f,e,d,d);
													X(d,a,f,d,e,d); X(d,a,f,d,d,e); X(d,f,e,d,d,a);
													X(d,f,e,d,a,d); X(d,f,e,a,d,d); X(d,f,d,e,d,a);
													X(d,f,d,e,a,d); X(d,f,d,d,e,a); X(d,f,d,d,a,e);
													X(d,f,d,a,e,d); X(d,f,d,a,d,e); X(d,f,a,e,d,d);
													X(d,f,a,d,e,d); X(d,f,a,d,d,e); X(a,e,d,d,d,f);
													X(a,e,d,d,f,d); X(a,e,d,f,d,d); X(a,e,f,d,d,d);
													X(a,d,e,d,d,f); X(a,d,e,d,f,d); X(a,d,e,f,d,d);
													X(a,d,d,e,d,f); X(a,d,d,e,f,d); X(a,d,d,d,e,f);
													X(a,d,d,d,f,e); X(a,d,d,f,e,d); X(a,d,d,f,d,e);
													X(a,d,f,e,d,d); X(a,d,f,d,e,d); X(a,d,f,d,d,e);
													X(a,f,e,d,d,d); X(a,f,d,e,d,d); X(a,f,d,d,e,d);
													X(a,f,d,d,d,e); X(f,e,d,d,d,a); X(f,e,d,d,a,d);
													X(f,e,d,a,d,d); X(f,e,a,d,d,d); X(f,d,e,d,d,a);
													X(f,d,e,d,a,d); X(f,d,e,a,d,d); X(f,d,d,e,d,a);
													X(f,d,d,e,a,d); X(f,d,d,d,e,a); X(f,d,d,d,a,e);
													X(f,d,d,a,e,d); X(f,d,d,a,d,e); X(f,d,a,e,d,d);
													X(f,d,a,d,e,d); X(f,d,a,d,d,e); X(f,a,e,d,d,d);
													X(f,a,d,e,d,d); X(f,a,d,d,e,d); X(f,a,d,d,d,e);
												} else {
													X(d,d,d,d,a,f); X(d,d,d,d,f,a); X(d,d,d,a,d,f);
													X(d,d,d,a,f,d); X(d,d,d,f,d,a); X(d,d,d,f,a,d);
													X(d,d,a,d,d,f); X(d,d,a,d,f,d); X(d,d,a,f,d,d);
													X(d,d,f,d,d,a); X(d,d,f,d,a,d); X(d,d,f,a,d,d);
													X(d,a,d,d,d,f); X(d,a,d,d,f,d); X(d,a,d,f,d,d);
													X(d,a,f,d,d,d); X(d,f,d,d,d,a); X(d,f,d,d,a,d);
													X(d,f,d,a,d,d); X(d,f,a,d,d,d); X(a,d,d,d,d,f);
													X(a,d,d,d,f,d); X(a,d,d,f,d,d); X(a,d,f,d,d,d);
													X(a,f,d,d,d,d); X(f,d,d,d,d,a); X(f,d,d,d,a,d);
													X(f,d,d,a,d,d); X(f,d,a,d,d,d); X(f,a,d,d,d,d);
												}
											}
										} else {
											// c != d
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												if (auto ep = dp * (e + 2); d != e) {
													DECLARE_POSITION_VALUES(e);
													X(e,d,c,c,a,f); X(e,d,c,c,f,a); X(e,d,c,a,c,f);
													X(e,d,c,a,f,c); X(e,d,c,f,c,a); X(e,d,c,f,a,c);
													X(e,d,a,c,c,f); X(e,d,a,c,f,c); X(e,d,a,f,c,c);
													X(e,d,f,c,c,a); X(e,d,f,c,a,c); X(e,d,f,a,c,c);
													X(e,c,d,c,a,f); X(e,c,d,c,f,a); X(e,c,d,a,c,f);
													X(e,c,d,a,f,c); X(e,c,d,f,c,a); X(e,c,d,f,a,c);
													X(e,c,c,d,a,f); X(e,c,c,d,f,a); X(e,c,c,a,d,f);
													X(e,c,c,a,f,d); X(e,c,c,f,d,a); X(e,c,c,f,a,d);
													X(e,c,a,d,c,f); X(e,c,a,d,f,c); X(e,c,a,c,d,f);
													X(e,c,a,c,f,d); X(e,c,a,f,d,c); X(e,c,a,f,c,d);
													X(e,c,f,d,c,a); X(e,c,f,d,a,c); X(e,c,f,c,d,a);
													X(e,c,f,c,a,d); X(e,c,f,a,d,c); X(e,c,f,a,c,d);
													X(e,a,d,c,c,f); X(e,a,d,c,f,c); X(e,a,d,f,c,c);
													X(e,a,c,d,c,f); X(e,a,c,d,f,c); X(e,a,c,c,d,f);
													X(e,a,c,c,f,d); X(e,a,c,f,d,c); X(e,a,c,f,c,d);
													X(e,a,f,d,c,c); X(e,a,f,c,d,c); X(e,a,f,c,c,d);
													X(e,f,d,c,c,a); X(e,f,d,c,a,c); X(e,f,d,a,c,c);
													X(e,f,c,d,c,a); X(e,f,c,d,a,c); X(e,f,c,c,d,a);
													X(e,f,c,c,a,d); X(e,f,c,a,d,c); X(e,f,c,a,c,d);
													X(e,f,a,d,c,c); X(e,f,a,c,d,c); X(e,f,a,c,c,d);
													X(d,e,c,c,a,f); X(d,e,c,c,f,a); X(d,e,c,a,c,f);
													X(d,e,c,a,f,c); X(d,e,c,f,c,a); X(d,e,c,f,a,c);
													X(d,e,a,c,c,f); X(d,e,a,c,f,c); X(d,e,a,f,c,c);
													X(d,e,f,c,c,a); X(d,e,f,c,a,c); X(d,e,f,a,c,c);
													X(d,c,e,c,a,f); X(d,c,e,c,f,a); X(d,c,e,a,c,f);
													X(d,c,e,a,f,c); X(d,c,e,f,c,a); X(d,c,e,f,a,c);
													X(d,c,c,e,a,f); X(d,c,c,e,f,a); X(d,c,c,a,e,f);
													X(d,c,c,a,f,e); X(d,c,c,f,e,a); X(d,c,c,f,a,e);
													X(d,c,a,e,c,f); X(d,c,a,e,f,c); X(d,c,a,c,e,f);
													X(d,c,a,c,f,e); X(d,c,a,f,e,c); X(d,c,a,f,c,e);
													X(d,c,f,e,c,a); X(d,c,f,e,a,c); X(d,c,f,c,e,a);
													X(d,c,f,c,a,e); X(d,c,f,a,e,c); X(d,c,f,a,c,e);
													X(d,a,e,c,c,f); X(d,a,e,c,f,c); X(d,a,e,f,c,c);
													X(d,a,c,e,c,f); X(d,a,c,e,f,c); X(d,a,c,c,e,f);
													X(d,a,c,c,f,e); X(d,a,c,f,e,c); X(d,a,c,f,c,e);
													X(d,a,f,e,c,c); X(d,a,f,c,e,c); X(d,a,f,c,c,e);
													X(d,f,e,c,c,a); X(d,f,e,c,a,c); X(d,f,e,a,c,c);
													X(d,f,c,e,c,a); X(d,f,c,e,a,c); X(d,f,c,c,e,a);
													X(d,f,c,c,a,e); X(d,f,c,a,e,c); X(d,f,c,a,c,e);
													X(d,f,a,e,c,c); X(d,f,a,c,e,c); X(d,f,a,c,c,e);
													X(c,e,d,c,a,f); X(c,e,d,c,f,a); X(c,e,d,a,c,f);
													X(c,e,d,a,f,c); X(c,e,d,f,c,a); X(c,e,d,f,a,c);
													X(c,e,c,d,a,f); X(c,e,c,d,f,a); X(c,e,c,a,d,f);
													X(c,e,c,a,f,d); X(c,e,c,f,d,a); X(c,e,c,f,a,d);
													X(c,e,a,d,c,f); X(c,e,a,d,f,c); X(c,e,a,c,d,f);
													X(c,e,a,c,f,d); X(c,e,a,f,d,c); X(c,e,a,f,c,d);
													X(c,e,f,d,c,a); X(c,e,f,d,a,c); X(c,e,f,c,d,a);
													X(c,e,f,c,a,d); X(c,e,f,a,d,c); X(c,e,f,a,c,d);
													X(c,d,e,c,a,f); X(c,d,e,c,f,a); X(c,d,e,a,c,f);
													X(c,d,e,a,f,c); X(c,d,e,f,c,a); X(c,d,e,f,a,c);
													X(c,d,c,e,a,f); X(c,d,c,e,f,a); X(c,d,c,a,e,f);
													X(c,d,c,a,f,e); X(c,d,c,f,e,a); X(c,d,c,f,a,e);
													X(c,d,a,e,c,f); X(c,d,a,e,f,c); X(c,d,a,c,e,f);
													X(c,d,a,c,f,e); X(c,d,a,f,e,c); X(c,d,a,f,c,e);
													X(c,d,f,e,c,a); X(c,d,f,e,a,c); X(c,d,f,c,e,a);
													X(c,d,f,c,a,e); X(c,d,f,a,e,c); X(c,d,f,a,c,e);
													X(c,c,e,d,a,f); X(c,c,e,d,f,a); X(c,c,e,a,d,f);
													X(c,c,e,a,f,d); X(c,c,e,f,d,a); X(c,c,e,f,a,d);
													X(c,c,d,e,a,f); X(c,c,d,e,f,a); X(c,c,d,a,e,f);
													X(c,c,d,a,f,e); X(c,c,d,f,e,a); X(c,c,d,f,a,e);
													X(c,c,a,e,d,f); X(c,c,a,e,f,d); X(c,c,a,d,e,f);
													X(c,c,a,d,f,e); X(c,c,a,f,e,d); X(c,c,a,f,d,e);
													X(c,c,f,e,d,a); X(c,c,f,e,a,d); X(c,c,f,d,e,a);
													X(c,c,f,d,a,e); X(c,c,f,a,e,d); X(c,c,f,a,d,e);
													X(c,a,e,d,c,f); X(c,a,e,d,f,c); X(c,a,e,c,d,f);
													X(c,a,e,c,f,d); X(c,a,e,f,d,c); X(c,a,e,f,c,d);
													X(c,a,d,e,c,f); X(c,a,d,e,f,c); X(c,a,d,c,e,f);
													X(c,a,d,c,f,e); X(c,a,d,f,e,c); X(c,a,d,f,c,e);
													X(c,a,c,e,d,f); X(c,a,c,e,f,d); X(c,a,c,d,e,f);
													X(c,a,c,d,f,e); X(c,a,c,f,e,d); X(c,a,c,f,d,e);
													X(c,a,f,e,d,c); X(c,a,f,e,c,d); X(c,a,f,d,e,c);
													X(c,a,f,d,c,e); X(c,a,f,c,e,d); X(c,a,f,c,d,e);
													X(c,f,e,d,c,a); X(c,f,e,d,a,c); X(c,f,e,c,d,a);
													X(c,f,e,c,a,d); X(c,f,e,a,d,c); X(c,f,e,a,c,d);
													X(c,f,d,e,c,a); X(c,f,d,e,a,c); X(c,f,d,c,e,a);
													X(c,f,d,c,a,e); X(c,f,d,a,e,c); X(c,f,d,a,c,e);
													X(c,f,c,e,d,a); X(c,f,c,e,a,d); X(c,f,c,d,e,a);
													X(c,f,c,d,a,e); X(c,f,c,a,e,d); X(c,f,c,a,d,e);
													X(c,f,a,e,d,c); X(c,f,a,e,c,d); X(c,f,a,d,e,c);
													X(c,f,a,d,c,e); X(c,f,a,c,e,d); X(c,f,a,c,d,e);
													X(a,e,d,c,c,f); X(a,e,d,c,f,c); X(a,e,d,f,c,c);
													X(a,e,c,d,c,f); X(a,e,c,d,f,c); X(a,e,c,c,d,f);
													X(a,e,c,c,f,d); X(a,e,c,f,d,c); X(a,e,c,f,c,d);
													X(a,e,f,d,c,c); X(a,e,f,c,d,c); X(a,e,f,c,c,d);
													X(a,d,e,c,c,f); X(a,d,e,c,f,c); X(a,d,e,f,c,c);
													X(a,d,c,e,c,f); X(a,d,c,e,f,c); X(a,d,c,c,e,f);
													X(a,d,c,c,f,e); X(a,d,c,f,e,c); X(a,d,c,f,c,e);
													X(a,d,f,e,c,c); X(a,d,f,c,e,c); X(a,d,f,c,c,e);
													X(a,c,e,d,c,f); X(a,c,e,d,f,c); X(a,c,e,c,d,f);
													X(a,c,e,c,f,d); X(a,c,e,f,d,c); X(a,c,e,f,c,d);
													X(a,c,d,e,c,f); X(a,c,d,e,f,c); X(a,c,d,c,e,f);
													X(a,c,d,c,f,e); X(a,c,d,f,e,c); X(a,c,d,f,c,e);
													X(a,c,c,e,d,f); X(a,c,c,e,f,d); X(a,c,c,d,e,f);
													X(a,c,c,d,f,e); X(a,c,c,f,e,d); X(a,c,c,f,d,e);
													X(a,c,f,e,d,c); X(a,c,f,e,c,d); X(a,c,f,d,e,c);
													X(a,c,f,d,c,e); X(a,c,f,c,e,d); X(a,c,f,c,d,e);
													X(a,f,e,d,c,c); X(a,f,e,c,d,c); X(a,f,e,c,c,d);
													X(a,f,d,e,c,c); X(a,f,d,c,e,c); X(a,f,d,c,c,e);
													X(a,f,c,e,d,c); X(a,f,c,e,c,d); X(a,f,c,d,e,c);
													X(a,f,c,d,c,e); X(a,f,c,c,e,d); X(a,f,c,c,d,e);
													X(f,e,d,c,c,a); X(f,e,d,c,a,c); X(f,e,d,a,c,c);
													X(f,e,c,d,c,a); X(f,e,c,d,a,c); X(f,e,c,c,d,a);
													X(f,e,c,c,a,d); X(f,e,c,a,d,c); X(f,e,c,a,c,d);
													X(f,e,a,d,c,c); X(f,e,a,c,d,c); X(f,e,a,c,c,d);
													X(f,d,e,c,c,a); X(f,d,e,c,a,c); X(f,d,e,a,c,c);
													X(f,d,c,e,c,a); X(f,d,c,e,a,c); X(f,d,c,c,e,a);
													X(f,d,c,c,a,e); X(f,d,c,a,e,c); X(f,d,c,a,c,e);
													X(f,d,a,e,c,c); X(f,d,a,c,e,c); X(f,d,a,c,c,e);
													X(f,c,e,d,c,a); X(f,c,e,d,a,c); X(f,c,e,c,d,a);
													X(f,c,e,c,a,d); X(f,c,e,a,d,c); X(f,c,e,a,c,d);
													X(f,c,d,e,c,a); X(f,c,d,e,a,c); X(f,c,d,c,e,a);
													X(f,c,d,c,a,e); X(f,c,d,a,e,c); X(f,c,d,a,c,e);
													X(f,c,c,e,d,a); X(f,c,c,e,a,d); X(f,c,c,d,e,a);
													X(f,c,c,d,a,e); X(f,c,c,a,e,d); X(f,c,c,a,d,e);
													X(f,c,a,e,d,c); X(f,c,a,e,c,d); X(f,c,a,d,e,c);
													X(f,c,a,d,c,e); X(f,c,a,c,e,d); X(f,c,a,c,d,e);
													X(f,a,e,d,c,c); X(f,a,e,c,d,c); X(f,a,e,c,c,d);
													X(f,a,d,e,c,c); X(f,a,d,c,e,c); X(f,a,d,c,c,e);
													X(f,a,c,e,d,c); X(f,a,c,e,c,d); X(f,a,c,d,e,c);
													X(f,a,c,d,c,e); X(f,a,c,c,e,d); X(f,a,c,c,d,e);
												} else {
													X(d,d,c,c,a,f); X(d,d,c,c,f,a); X(d,d,c,a,c,f);
													X(d,d,c,a,f,c); X(d,d,c,f,c,a); X(d,d,c,f,a,c);
													X(d,d,a,c,c,f); X(d,d,a,c,f,c); X(d,d,a,f,c,c);
													X(d,d,f,c,c,a); X(d,d,f,c,a,c); X(d,d,f,a,c,c);
													X(d,c,d,c,a,f); X(d,c,d,c,f,a); X(d,c,d,a,c,f);
													X(d,c,d,a,f,c); X(d,c,d,f,c,a); X(d,c,d,f,a,c);
													X(d,c,c,d,a,f); X(d,c,c,d,f,a); X(d,c,c,a,d,f);
													X(d,c,c,a,f,d); X(d,c,c,f,d,a); X(d,c,c,f,a,d);
													X(d,c,a,d,c,f); X(d,c,a,d,f,c); X(d,c,a,c,d,f);
													X(d,c,a,c,f,d); X(d,c,a,f,d,c); X(d,c,a,f,c,d);
													X(d,c,f,d,c,a); X(d,c,f,d,a,c); X(d,c,f,c,d,a);
													X(d,c,f,c,a,d); X(d,c,f,a,d,c); X(d,c,f,a,c,d);
													X(d,a,d,c,c,f); X(d,a,d,c,f,c); X(d,a,d,f,c,c);
													X(d,a,c,d,c,f); X(d,a,c,d,f,c); X(d,a,c,c,d,f);
													X(d,a,c,c,f,d); X(d,a,c,f,d,c); X(d,a,c,f,c,d);
													X(d,a,f,d,c,c); X(d,a,f,c,d,c); X(d,a,f,c,c,d);
													X(d,f,d,c,c,a); X(d,f,d,c,a,c); X(d,f,d,a,c,c);
													X(d,f,c,d,c,a); X(d,f,c,d,a,c); X(d,f,c,c,d,a);
													X(d,f,c,c,a,d); X(d,f,c,a,d,c); X(d,f,c,a,c,d);
													X(d,f,a,d,c,c); X(d,f,a,c,d,c); X(d,f,a,c,c,d);
													X(c,d,d,c,a,f); X(c,d,d,c,f,a); X(c,d,d,a,c,f);
													X(c,d,d,a,f,c); X(c,d,d,f,c,a); X(c,d,d,f,a,c);
													X(c,d,c,d,a,f); X(c,d,c,d,f,a); X(c,d,c,a,d,f);
													X(c,d,c,a,f,d); X(c,d,c,f,d,a); X(c,d,c,f,a,d);
													X(c,d,a,d,c,f); X(c,d,a,d,f,c); X(c,d,a,c,d,f);
													X(c,d,a,c,f,d); X(c,d,a,f,d,c); X(c,d,a,f,c,d);
													X(c,d,f,d,c,a); X(c,d,f,d,a,c); X(c,d,f,c,d,a);
													X(c,d,f,c,a,d); X(c,d,f,a,d,c); X(c,d,f,a,c,d);
													X(c,c,d,d,a,f); X(c,c,d,d,f,a); X(c,c,d,a,d,f);
													X(c,c,d,a,f,d); X(c,c,d,f,d,a); X(c,c,d,f,a,d);
													X(c,c,a,d,d,f); X(c,c,a,d,f,d); X(c,c,a,f,d,d);
													X(c,c,f,d,d,a); X(c,c,f,d,a,d); X(c,c,f,a,d,d);
													X(c,a,d,d,c,f); X(c,a,d,d,f,c); X(c,a,d,c,d,f);
													X(c,a,d,c,f,d); X(c,a,d,f,d,c); X(c,a,d,f,c,d);
													X(c,a,c,d,d,f); X(c,a,c,d,f,d); X(c,a,c,f,d,d);
													X(c,a,f,d,d,c); X(c,a,f,d,c,d); X(c,a,f,c,d,d);
													X(c,f,d,d,c,a); X(c,f,d,d,a,c); X(c,f,d,c,d,a);
													X(c,f,d,c,a,d); X(c,f,d,a,d,c); X(c,f,d,a,c,d);
													X(c,f,c,d,d,a); X(c,f,c,d,a,d); X(c,f,c,a,d,d);
													X(c,f,a,d,d,c); X(c,f,a,d,c,d); X(c,f,a,c,d,d);
													X(a,d,d,c,c,f); X(a,d,d,c,f,c); X(a,d,d,f,c,c);
													X(a,d,c,d,c,f); X(a,d,c,d,f,c); X(a,d,c,c,d,f);
													X(a,d,c,c,f,d); X(a,d,c,f,d,c); X(a,d,c,f,c,d);
													X(a,d,f,d,c,c); X(a,d,f,c,d,c); X(a,d,f,c,c,d);
													X(a,c,d,d,c,f); X(a,c,d,d,f,c); X(a,c,d,c,d,f);
													X(a,c,d,c,f,d); X(a,c,d,f,d,c); X(a,c,d,f,c,d);
													X(a,c,c,d,d,f); X(a,c,c,d,f,d); X(a,c,c,f,d,d);
													X(a,c,f,d,d,c); X(a,c,f,d,c,d); X(a,c,f,c,d,d);
													X(a,f,d,d,c,c); X(a,f,d,c,d,c); X(a,f,d,c,c,d);
													X(a,f,c,d,d,c); X(a,f,c,d,c,d); X(a,f,c,c,d,d);
													X(f,d,d,c,c,a); X(f,d,d,c,a,c); X(f,d,d,a,c,c);
													X(f,d,c,d,c,a); X(f,d,c,d,a,c); X(f,d,c,c,d,a);
													X(f,d,c,c,a,d); X(f,d,c,a,d,c); X(f,d,c,a,c,d);
													X(f,d,a,d,c,c); X(f,d,a,c,d,c); X(f,d,a,c,c,d);
													X(f,c,d,d,c,a); X(f,c,d,d,a,c); X(f,c,d,c,d,a);
													X(f,c,d,c,a,d); X(f,c,d,a,d,c); X(f,c,d,a,c,d);
													X(f,c,c,d,d,a); X(f,c,c,d,a,d); X(f,c,c,a,d,d);
													X(f,c,a,d,d,c); X(f,c,a,d,c,d); X(f,c,a,c,d,d);
													X(f,a,d,d,c,c); X(f,a,d,c,d,c); X(f,a,d,c,c,d);
													X(f,a,c,d,d,c); X(f,a,c,d,c,d); X(f,a,c,c,d,d);
												}
											}
										}
									}
								} else {
									// b != c
									for (auto d = c; d < 8ul; ++d) {
										SKIP5s(d);
										DECLARE_POSITION_VALUES(d);
										if (auto ds = cs + d, dp = cp * (d + 2); c == d) {
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												if (auto ep = dp * (e + 2); d != e) {
													DECLARE_POSITION_VALUES(e);
													X(e,d,d,b,a,f); X(e,d,d,b,f,a); X(e,d,d,a,b,f);
													X(e,d,d,a,f,b); X(e,d,d,f,b,a); X(e,d,d,f,a,b);
													X(e,d,b,d,a,f); X(e,d,b,d,f,a); X(e,d,b,a,d,f);
													X(e,d,b,a,f,d); X(e,d,b,f,d,a); X(e,d,b,f,a,d);
													X(e,d,a,d,b,f); X(e,d,a,d,f,b); X(e,d,a,b,d,f);
													X(e,d,a,b,f,d); X(e,d,a,f,d,b); X(e,d,a,f,b,d);
													X(e,d,f,d,b,a); X(e,d,f,d,a,b); X(e,d,f,b,d,a);
													X(e,d,f,b,a,d); X(e,d,f,a,d,b); X(e,d,f,a,b,d);
													X(e,b,d,d,a,f); X(e,b,d,d,f,a); X(e,b,d,a,d,f);
													X(e,b,d,a,f,d); X(e,b,d,f,d,a); X(e,b,d,f,a,d);
													X(e,b,a,d,d,f); X(e,b,a,d,f,d); X(e,b,a,f,d,d);
													X(e,b,f,d,d,a); X(e,b,f,d,a,d); X(e,b,f,a,d,d);
													X(e,a,d,d,b,f); X(e,a,d,d,f,b); X(e,a,d,b,d,f);
													X(e,a,d,b,f,d); X(e,a,d,f,d,b); X(e,a,d,f,b,d);
													X(e,a,b,d,d,f); X(e,a,b,d,f,d); X(e,a,b,f,d,d);
													X(e,a,f,d,d,b); X(e,a,f,d,b,d); X(e,a,f,b,d,d);
													X(e,f,d,d,b,a); X(e,f,d,d,a,b); X(e,f,d,b,d,a);
													X(e,f,d,b,a,d); X(e,f,d,a,d,b); X(e,f,d,a,b,d);
													X(e,f,b,d,d,a); X(e,f,b,d,a,d); X(e,f,b,a,d,d);
													X(e,f,a,d,d,b); X(e,f,a,d,b,d); X(e,f,a,b,d,d);
													X(d,e,d,b,a,f); X(d,e,d,b,f,a); X(d,e,d,a,b,f);
													X(d,e,d,a,f,b); X(d,e,d,f,b,a); X(d,e,d,f,a,b);
													X(d,e,b,d,a,f); X(d,e,b,d,f,a); X(d,e,b,a,d,f);
													X(d,e,b,a,f,d); X(d,e,b,f,d,a); X(d,e,b,f,a,d);
													X(d,e,a,d,b,f); X(d,e,a,d,f,b); X(d,e,a,b,d,f);
													X(d,e,a,b,f,d); X(d,e,a,f,d,b); X(d,e,a,f,b,d);
													X(d,e,f,d,b,a); X(d,e,f,d,a,b); X(d,e,f,b,d,a);
													X(d,e,f,b,a,d); X(d,e,f,a,d,b); X(d,e,f,a,b,d);
													X(d,d,e,b,a,f); X(d,d,e,b,f,a); X(d,d,e,a,b,f);
													X(d,d,e,a,f,b); X(d,d,e,f,b,a); X(d,d,e,f,a,b);
													X(d,d,b,e,a,f); X(d,d,b,e,f,a); X(d,d,b,a,e,f);
													X(d,d,b,a,f,e); X(d,d,b,f,e,a); X(d,d,b,f,a,e);
													X(d,d,a,e,b,f); X(d,d,a,e,f,b); X(d,d,a,b,e,f);
													X(d,d,a,b,f,e); X(d,d,a,f,e,b); X(d,d,a,f,b,e);
													X(d,d,f,e,b,a); X(d,d,f,e,a,b); X(d,d,f,b,e,a);
													X(d,d,f,b,a,e); X(d,d,f,a,e,b); X(d,d,f,a,b,e);
													X(d,b,e,d,a,f); X(d,b,e,d,f,a); X(d,b,e,a,d,f);
													X(d,b,e,a,f,d); X(d,b,e,f,d,a); X(d,b,e,f,a,d);
													X(d,b,d,e,a,f); X(d,b,d,e,f,a); X(d,b,d,a,e,f);
													X(d,b,d,a,f,e); X(d,b,d,f,e,a); X(d,b,d,f,a,e);
													X(d,b,a,e,d,f); X(d,b,a,e,f,d); X(d,b,a,d,e,f);
													X(d,b,a,d,f,e); X(d,b,a,f,e,d); X(d,b,a,f,d,e);
													X(d,b,f,e,d,a); X(d,b,f,e,a,d); X(d,b,f,d,e,a);
													X(d,b,f,d,a,e); X(d,b,f,a,e,d); X(d,b,f,a,d,e);
													X(d,a,e,d,b,f); X(d,a,e,d,f,b); X(d,a,e,b,d,f);
													X(d,a,e,b,f,d); X(d,a,e,f,d,b); X(d,a,e,f,b,d);
													X(d,a,d,e,b,f); X(d,a,d,e,f,b); X(d,a,d,b,e,f);
													X(d,a,d,b,f,e); X(d,a,d,f,e,b); X(d,a,d,f,b,e);
													X(d,a,b,e,d,f); X(d,a,b,e,f,d); X(d,a,b,d,e,f);
													X(d,a,b,d,f,e); X(d,a,b,f,e,d); X(d,a,b,f,d,e);
													X(d,a,f,e,d,b); X(d,a,f,e,b,d); X(d,a,f,d,e,b);
													X(d,a,f,d,b,e); X(d,a,f,b,e,d); X(d,a,f,b,d,e);
													X(d,f,e,d,b,a); X(d,f,e,d,a,b); X(d,f,e,b,d,a);
													X(d,f,e,b,a,d); X(d,f,e,a,d,b); X(d,f,e,a,b,d);
													X(d,f,d,e,b,a); X(d,f,d,e,a,b); X(d,f,d,b,e,a);
													X(d,f,d,b,a,e); X(d,f,d,a,e,b); X(d,f,d,a,b,e);
													X(d,f,b,e,d,a); X(d,f,b,e,a,d); X(d,f,b,d,e,a);
													X(d,f,b,d,a,e); X(d,f,b,a,e,d); X(d,f,b,a,d,e);
													X(d,f,a,e,d,b); X(d,f,a,e,b,d); X(d,f,a,d,e,b);
													X(d,f,a,d,b,e); X(d,f,a,b,e,d); X(d,f,a,b,d,e);
													X(b,e,d,d,a,f); X(b,e,d,d,f,a); X(b,e,d,a,d,f);
													X(b,e,d,a,f,d); X(b,e,d,f,d,a); X(b,e,d,f,a,d);
													X(b,e,a,d,d,f); X(b,e,a,d,f,d); X(b,e,a,f,d,d);
													X(b,e,f,d,d,a); X(b,e,f,d,a,d); X(b,e,f,a,d,d);
													X(b,d,e,d,a,f); X(b,d,e,d,f,a); X(b,d,e,a,d,f);
													X(b,d,e,a,f,d); X(b,d,e,f,d,a); X(b,d,e,f,a,d);
													X(b,d,d,e,a,f); X(b,d,d,e,f,a); X(b,d,d,a,e,f);
													X(b,d,d,a,f,e); X(b,d,d,f,e,a); X(b,d,d,f,a,e);
													X(b,d,a,e,d,f); X(b,d,a,e,f,d); X(b,d,a,d,e,f);
													X(b,d,a,d,f,e); X(b,d,a,f,e,d); X(b,d,a,f,d,e);
													X(b,d,f,e,d,a); X(b,d,f,e,a,d); X(b,d,f,d,e,a);
													X(b,d,f,d,a,e); X(b,d,f,a,e,d); X(b,d,f,a,d,e);
													X(b,a,e,d,d,f); X(b,a,e,d,f,d); X(b,a,e,f,d,d);
													X(b,a,d,e,d,f); X(b,a,d,e,f,d); X(b,a,d,d,e,f);
													X(b,a,d,d,f,e); X(b,a,d,f,e,d); X(b,a,d,f,d,e);
													X(b,a,f,e,d,d); X(b,a,f,d,e,d); X(b,a,f,d,d,e);
													X(b,f,e,d,d,a); X(b,f,e,d,a,d); X(b,f,e,a,d,d);
													X(b,f,d,e,d,a); X(b,f,d,e,a,d); X(b,f,d,d,e,a);
													X(b,f,d,d,a,e); X(b,f,d,a,e,d); X(b,f,d,a,d,e);
													X(b,f,a,e,d,d); X(b,f,a,d,e,d); X(b,f,a,d,d,e);
													X(a,e,d,d,b,f); X(a,e,d,d,f,b); X(a,e,d,b,d,f);
													X(a,e,d,b,f,d); X(a,e,d,f,d,b); X(a,e,d,f,b,d);
													X(a,e,b,d,d,f); X(a,e,b,d,f,d); X(a,e,b,f,d,d);
													X(a,e,f,d,d,b); X(a,e,f,d,b,d); X(a,e,f,b,d,d);
													X(a,d,e,d,b,f); X(a,d,e,d,f,b); X(a,d,e,b,d,f);
													X(a,d,e,b,f,d); X(a,d,e,f,d,b); X(a,d,e,f,b,d);
													X(a,d,d,e,b,f); X(a,d,d,e,f,b); X(a,d,d,b,e,f);
													X(a,d,d,b,f,e); X(a,d,d,f,e,b); X(a,d,d,f,b,e);
													X(a,d,b,e,d,f); X(a,d,b,e,f,d); X(a,d,b,d,e,f);
													X(a,d,b,d,f,e); X(a,d,b,f,e,d); X(a,d,b,f,d,e);
													X(a,d,f,e,d,b); X(a,d,f,e,b,d); X(a,d,f,d,e,b);
													X(a,d,f,d,b,e); X(a,d,f,b,e,d); X(a,d,f,b,d,e);
													X(a,b,e,d,d,f); X(a,b,e,d,f,d); X(a,b,e,f,d,d);
													X(a,b,d,e,d,f); X(a,b,d,e,f,d); X(a,b,d,d,e,f);
													X(a,b,d,d,f,e); X(a,b,d,f,e,d); X(a,b,d,f,d,e);
													X(a,b,f,e,d,d); X(a,b,f,d,e,d); X(a,b,f,d,d,e);
													X(a,f,e,d,d,b); X(a,f,e,d,b,d); X(a,f,e,b,d,d);
													X(a,f,d,e,d,b); X(a,f,d,e,b,d); X(a,f,d,d,e,b);
													X(a,f,d,d,b,e); X(a,f,d,b,e,d); X(a,f,d,b,d,e);
													X(a,f,b,e,d,d); X(a,f,b,d,e,d); X(a,f,b,d,d,e);
													X(f,e,d,d,b,a); X(f,e,d,d,a,b); X(f,e,d,b,d,a);
													X(f,e,d,b,a,d); X(f,e,d,a,d,b); X(f,e,d,a,b,d);
													X(f,e,b,d,d,a); X(f,e,b,d,a,d); X(f,e,b,a,d,d);
													X(f,e,a,d,d,b); X(f,e,a,d,b,d); X(f,e,a,b,d,d);
													X(f,d,e,d,b,a); X(f,d,e,d,a,b); X(f,d,e,b,d,a);
													X(f,d,e,b,a,d); X(f,d,e,a,d,b); X(f,d,e,a,b,d);
													X(f,d,d,e,b,a); X(f,d,d,e,a,b); X(f,d,d,b,e,a);
													X(f,d,d,b,a,e); X(f,d,d,a,e,b); X(f,d,d,a,b,e);
													X(f,d,b,e,d,a); X(f,d,b,e,a,d); X(f,d,b,d,e,a);
													X(f,d,b,d,a,e); X(f,d,b,a,e,d); X(f,d,b,a,d,e);
													X(f,d,a,e,d,b); X(f,d,a,e,b,d); X(f,d,a,d,e,b);
													X(f,d,a,d,b,e); X(f,d,a,b,e,d); X(f,d,a,b,d,e);
													X(f,b,e,d,d,a); X(f,b,e,d,a,d); X(f,b,e,a,d,d);
													X(f,b,d,e,d,a); X(f,b,d,e,a,d); X(f,b,d,d,e,a);
													X(f,b,d,d,a,e); X(f,b,d,a,e,d); X(f,b,d,a,d,e);
													X(f,b,a,e,d,d); X(f,b,a,d,e,d); X(f,b,a,d,d,e);
													X(f,a,e,d,d,b); X(f,a,e,d,b,d); X(f,a,e,b,d,d);
													X(f,a,d,e,d,b); X(f,a,d,e,b,d); X(f,a,d,d,e,b);
													X(f,a,d,d,b,e); X(f,a,d,b,e,d); X(f,a,d,b,d,e);
													X(f,a,b,e,d,d); X(f,a,b,d,e,d); X(f,a,b,d,d,e);
												} else {
													X(d,d,d,b,a,f); X(d,d,d,b,f,a); X(d,d,d,a,b,f);
													X(d,d,d,a,f,b); X(d,d,d,f,b,a); X(d,d,d,f,a,b);
													X(d,d,b,d,a,f); X(d,d,b,d,f,a); X(d,d,b,a,d,f);
													X(d,d,b,a,f,d); X(d,d,b,f,d,a); X(d,d,b,f,a,d);
													X(d,d,a,d,b,f); X(d,d,a,d,f,b); X(d,d,a,b,d,f);
													X(d,d,a,b,f,d); X(d,d,a,f,d,b); X(d,d,a,f,b,d);
													X(d,d,f,d,b,a); X(d,d,f,d,a,b); X(d,d,f,b,d,a);
													X(d,d,f,b,a,d); X(d,d,f,a,d,b); X(d,d,f,a,b,d);
													X(d,b,d,d,a,f); X(d,b,d,d,f,a); X(d,b,d,a,d,f);
													X(d,b,d,a,f,d); X(d,b,d,f,d,a); X(d,b,d,f,a,d);
													X(d,b,a,d,d,f); X(d,b,a,d,f,d); X(d,b,a,f,d,d);
													X(d,b,f,d,d,a); X(d,b,f,d,a,d); X(d,b,f,a,d,d);
													X(d,a,d,d,b,f); X(d,a,d,d,f,b); X(d,a,d,b,d,f);
													X(d,a,d,b,f,d); X(d,a,d,f,d,b); X(d,a,d,f,b,d);
													X(d,a,b,d,d,f); X(d,a,b,d,f,d); X(d,a,b,f,d,d);
													X(d,a,f,d,d,b); X(d,a,f,d,b,d); X(d,a,f,b,d,d);
													X(d,f,d,d,b,a); X(d,f,d,d,a,b); X(d,f,d,b,d,a);
													X(d,f,d,b,a,d); X(d,f,d,a,d,b); X(d,f,d,a,b,d);
													X(d,f,b,d,d,a); X(d,f,b,d,a,d); X(d,f,b,a,d,d);
													X(d,f,a,d,d,b); X(d,f,a,d,b,d); X(d,f,a,b,d,d);
													X(b,d,d,d,a,f); X(b,d,d,d,f,a); X(b,d,d,a,d,f);
													X(b,d,d,a,f,d); X(b,d,d,f,d,a); X(b,d,d,f,a,d);
													X(b,d,a,d,d,f); X(b,d,a,d,f,d); X(b,d,a,f,d,d);
													X(b,d,f,d,d,a); X(b,d,f,d,a,d); X(b,d,f,a,d,d);
													X(b,a,d,d,d,f); X(b,a,d,d,f,d); X(b,a,d,f,d,d);
													X(b,a,f,d,d,d); X(b,f,d,d,d,a); X(b,f,d,d,a,d);
													X(b,f,d,a,d,d); X(b,f,a,d,d,d); X(a,d,d,d,b,f);
													X(a,d,d,d,f,b); X(a,d,d,b,d,f); X(a,d,d,b,f,d);
													X(a,d,d,f,d,b); X(a,d,d,f,b,d); X(a,d,b,d,d,f);
													X(a,d,b,d,f,d); X(a,d,b,f,d,d); X(a,d,f,d,d,b);
													X(a,d,f,d,b,d); X(a,d,f,b,d,d); X(a,b,d,d,d,f);
													X(a,b,d,d,f,d); X(a,b,d,f,d,d); X(a,b,f,d,d,d);
													X(a,f,d,d,d,b); X(a,f,d,d,b,d); X(a,f,d,b,d,d);
													X(a,f,b,d,d,d); X(f,d,d,d,b,a); X(f,d,d,d,a,b);
													X(f,d,d,b,d,a); X(f,d,d,b,a,d); X(f,d,d,a,d,b);
													X(f,d,d,a,b,d); X(f,d,b,d,d,a); X(f,d,b,d,a,d);
													X(f,d,b,a,d,d); X(f,d,a,d,d,b); X(f,d,a,d,b,d);
													X(f,d,a,b,d,d); X(f,b,d,d,d,a); X(f,b,d,d,a,d);
													X(f,b,d,a,d,d); X(f,b,a,d,d,d); X(f,a,d,d,d,b);
													X(f,a,d,d,b,d); X(f,a,d,b,d,d); X(f,a,b,d,d,d);
												}
											}
										} else {
											// c != d
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												if (auto ep = dp * (e + 2); d != e) {
													DECLARE_POSITION_VALUES(e);
													X(e,d,c,b,a,f); X(e,d,c,b,f,a); X(e,d,c,a,b,f);
													X(e,d,c,a,f,b); X(e,d,c,f,b,a); X(e,d,c,f,a,b);
													X(e,d,b,c,a,f); X(e,d,b,c,f,a); X(e,d,b,a,c,f);
													X(e,d,b,a,f,c); X(e,d,b,f,c,a); X(e,d,b,f,a,c);
													X(e,d,a,c,b,f); X(e,d,a,c,f,b); X(e,d,a,b,c,f);
													X(e,d,a,b,f,c); X(e,d,a,f,c,b); X(e,d,a,f,b,c);
													X(e,d,f,c,b,a); X(e,d,f,c,a,b); X(e,d,f,b,c,a);
													X(e,d,f,b,a,c); X(e,d,f,a,c,b); X(e,d,f,a,b,c);
													X(e,c,d,b,a,f); X(e,c,d,b,f,a); X(e,c,d,a,b,f);
													X(e,c,d,a,f,b); X(e,c,d,f,b,a); X(e,c,d,f,a,b);
													X(e,c,b,d,a,f); X(e,c,b,d,f,a); X(e,c,b,a,d,f);
													X(e,c,b,a,f,d); X(e,c,b,f,d,a); X(e,c,b,f,a,d);
													X(e,c,a,d,b,f); X(e,c,a,d,f,b); X(e,c,a,b,d,f);
													X(e,c,a,b,f,d); X(e,c,a,f,d,b); X(e,c,a,f,b,d);
													X(e,c,f,d,b,a); X(e,c,f,d,a,b); X(e,c,f,b,d,a);
													X(e,c,f,b,a,d); X(e,c,f,a,d,b); X(e,c,f,a,b,d);
													X(e,b,d,c,a,f); X(e,b,d,c,f,a); X(e,b,d,a,c,f);
													X(e,b,d,a,f,c); X(e,b,d,f,c,a); X(e,b,d,f,a,c);
													X(e,b,c,d,a,f); X(e,b,c,d,f,a); X(e,b,c,a,d,f);
													X(e,b,c,a,f,d); X(e,b,c,f,d,a); X(e,b,c,f,a,d);
													X(e,b,a,d,c,f); X(e,b,a,d,f,c); X(e,b,a,c,d,f);
													X(e,b,a,c,f,d); X(e,b,a,f,d,c); X(e,b,a,f,c,d);
													X(e,b,f,d,c,a); X(e,b,f,d,a,c); X(e,b,f,c,d,a);
													X(e,b,f,c,a,d); X(e,b,f,a,d,c); X(e,b,f,a,c,d);
													X(e,a,d,c,b,f); X(e,a,d,c,f,b); X(e,a,d,b,c,f);
													X(e,a,d,b,f,c); X(e,a,d,f,c,b); X(e,a,d,f,b,c);
													X(e,a,c,d,b,f); X(e,a,c,d,f,b); X(e,a,c,b,d,f);
													X(e,a,c,b,f,d); X(e,a,c,f,d,b); X(e,a,c,f,b,d);
													X(e,a,b,d,c,f); X(e,a,b,d,f,c); X(e,a,b,c,d,f);
													X(e,a,b,c,f,d); X(e,a,b,f,d,c); X(e,a,b,f,c,d);
													X(e,a,f,d,c,b); X(e,a,f,d,b,c); X(e,a,f,c,d,b);
													X(e,a,f,c,b,d); X(e,a,f,b,d,c); X(e,a,f,b,c,d);
													X(e,f,d,c,b,a); X(e,f,d,c,a,b); X(e,f,d,b,c,a);
													X(e,f,d,b,a,c); X(e,f,d,a,c,b); X(e,f,d,a,b,c);
													X(e,f,c,d,b,a); X(e,f,c,d,a,b); X(e,f,c,b,d,a);
													X(e,f,c,b,a,d); X(e,f,c,a,d,b); X(e,f,c,a,b,d);
													X(e,f,b,d,c,a); X(e,f,b,d,a,c); X(e,f,b,c,d,a);
													X(e,f,b,c,a,d); X(e,f,b,a,d,c); X(e,f,b,a,c,d);
													X(e,f,a,d,c,b); X(e,f,a,d,b,c); X(e,f,a,c,d,b);
													X(e,f,a,c,b,d); X(e,f,a,b,d,c); X(e,f,a,b,c,d);
													X(d,e,c,b,a,f); X(d,e,c,b,f,a); X(d,e,c,a,b,f);
													X(d,e,c,a,f,b); X(d,e,c,f,b,a); X(d,e,c,f,a,b);
													X(d,e,b,c,a,f); X(d,e,b,c,f,a); X(d,e,b,a,c,f);
													X(d,e,b,a,f,c); X(d,e,b,f,c,a); X(d,e,b,f,a,c);
													X(d,e,a,c,b,f); X(d,e,a,c,f,b); X(d,e,a,b,c,f);
													X(d,e,a,b,f,c); X(d,e,a,f,c,b); X(d,e,a,f,b,c);
													X(d,e,f,c,b,a); X(d,e,f,c,a,b); X(d,e,f,b,c,a);
													X(d,e,f,b,a,c); X(d,e,f,a,c,b); X(d,e,f,a,b,c);
													X(d,c,e,b,a,f); X(d,c,e,b,f,a); X(d,c,e,a,b,f);
													X(d,c,e,a,f,b); X(d,c,e,f,b,a); X(d,c,e,f,a,b);
													X(d,c,b,e,a,f); X(d,c,b,e,f,a); X(d,c,b,a,e,f);
													X(d,c,b,a,f,e); X(d,c,b,f,e,a); X(d,c,b,f,a,e);
													X(d,c,a,e,b,f); X(d,c,a,e,f,b); X(d,c,a,b,e,f);
													X(d,c,a,b,f,e); X(d,c,a,f,e,b); X(d,c,a,f,b,e);
													X(d,c,f,e,b,a); X(d,c,f,e,a,b); X(d,c,f,b,e,a);
													X(d,c,f,b,a,e); X(d,c,f,a,e,b); X(d,c,f,a,b,e);
													X(d,b,e,c,a,f); X(d,b,e,c,f,a); X(d,b,e,a,c,f);
													X(d,b,e,a,f,c); X(d,b,e,f,c,a); X(d,b,e,f,a,c);
													X(d,b,c,e,a,f); X(d,b,c,e,f,a); X(d,b,c,a,e,f);
													X(d,b,c,a,f,e); X(d,b,c,f,e,a); X(d,b,c,f,a,e);
													X(d,b,a,e,c,f); X(d,b,a,e,f,c); X(d,b,a,c,e,f);
													X(d,b,a,c,f,e); X(d,b,a,f,e,c); X(d,b,a,f,c,e);
													X(d,b,f,e,c,a); X(d,b,f,e,a,c); X(d,b,f,c,e,a);
													X(d,b,f,c,a,e); X(d,b,f,a,e,c); X(d,b,f,a,c,e);
													X(d,a,e,c,b,f); X(d,a,e,c,f,b); X(d,a,e,b,c,f);
													X(d,a,e,b,f,c); X(d,a,e,f,c,b); X(d,a,e,f,b,c);
													X(d,a,c,e,b,f); X(d,a,c,e,f,b); X(d,a,c,b,e,f);
													X(d,a,c,b,f,e); X(d,a,c,f,e,b); X(d,a,c,f,b,e);
													X(d,a,b,e,c,f); X(d,a,b,e,f,c); X(d,a,b,c,e,f);
													X(d,a,b,c,f,e); X(d,a,b,f,e,c); X(d,a,b,f,c,e);
													X(d,a,f,e,c,b); X(d,a,f,e,b,c); X(d,a,f,c,e,b);
													X(d,a,f,c,b,e); X(d,a,f,b,e,c); X(d,a,f,b,c,e);
													X(d,f,e,c,b,a); X(d,f,e,c,a,b); X(d,f,e,b,c,a);
													X(d,f,e,b,a,c); X(d,f,e,a,c,b); X(d,f,e,a,b,c);
													X(d,f,c,e,b,a); X(d,f,c,e,a,b); X(d,f,c,b,e,a);
													X(d,f,c,b,a,e); X(d,f,c,a,e,b); X(d,f,c,a,b,e);
													X(d,f,b,e,c,a); X(d,f,b,e,a,c); X(d,f,b,c,e,a);
													X(d,f,b,c,a,e); X(d,f,b,a,e,c); X(d,f,b,a,c,e);
													X(d,f,a,e,c,b); X(d,f,a,e,b,c); X(d,f,a,c,e,b);
													X(d,f,a,c,b,e); X(d,f,a,b,e,c); X(d,f,a,b,c,e);
													X(c,e,d,b,a,f); X(c,e,d,b,f,a); X(c,e,d,a,b,f);
													X(c,e,d,a,f,b); X(c,e,d,f,b,a); X(c,e,d,f,a,b);
													X(c,e,b,d,a,f); X(c,e,b,d,f,a); X(c,e,b,a,d,f);
													X(c,e,b,a,f,d); X(c,e,b,f,d,a); X(c,e,b,f,a,d);
													X(c,e,a,d,b,f); X(c,e,a,d,f,b); X(c,e,a,b,d,f);
													X(c,e,a,b,f,d); X(c,e,a,f,d,b); X(c,e,a,f,b,d);
													X(c,e,f,d,b,a); X(c,e,f,d,a,b); X(c,e,f,b,d,a);
													X(c,e,f,b,a,d); X(c,e,f,a,d,b); X(c,e,f,a,b,d);
													X(c,d,e,b,a,f); X(c,d,e,b,f,a); X(c,d,e,a,b,f);
													X(c,d,e,a,f,b); X(c,d,e,f,b,a); X(c,d,e,f,a,b);
													X(c,d,b,e,a,f); X(c,d,b,e,f,a); X(c,d,b,a,e,f);
													X(c,d,b,a,f,e); X(c,d,b,f,e,a); X(c,d,b,f,a,e);
													X(c,d,a,e,b,f); X(c,d,a,e,f,b); X(c,d,a,b,e,f);
													X(c,d,a,b,f,e); X(c,d,a,f,e,b); X(c,d,a,f,b,e);
													X(c,d,f,e,b,a); X(c,d,f,e,a,b); X(c,d,f,b,e,a);
													X(c,d,f,b,a,e); X(c,d,f,a,e,b); X(c,d,f,a,b,e);
													X(c,b,e,d,a,f); X(c,b,e,d,f,a); X(c,b,e,a,d,f);
													X(c,b,e,a,f,d); X(c,b,e,f,d,a); X(c,b,e,f,a,d);
													X(c,b,d,e,a,f); X(c,b,d,e,f,a); X(c,b,d,a,e,f);
													X(c,b,d,a,f,e); X(c,b,d,f,e,a); X(c,b,d,f,a,e);
													X(c,b,a,e,d,f); X(c,b,a,e,f,d); X(c,b,a,d,e,f);
													X(c,b,a,d,f,e); X(c,b,a,f,e,d); X(c,b,a,f,d,e);
													X(c,b,f,e,d,a); X(c,b,f,e,a,d); X(c,b,f,d,e,a);
													X(c,b,f,d,a,e); X(c,b,f,a,e,d); X(c,b,f,a,d,e);
													X(c,a,e,d,b,f); X(c,a,e,d,f,b); X(c,a,e,b,d,f);
													X(c,a,e,b,f,d); X(c,a,e,f,d,b); X(c,a,e,f,b,d);
													X(c,a,d,e,b,f); X(c,a,d,e,f,b); X(c,a,d,b,e,f);
													X(c,a,d,b,f,e); X(c,a,d,f,e,b); X(c,a,d,f,b,e);
													X(c,a,b,e,d,f); X(c,a,b,e,f,d); X(c,a,b,d,e,f);
													X(c,a,b,d,f,e); X(c,a,b,f,e,d); X(c,a,b,f,d,e);
													X(c,a,f,e,d,b); X(c,a,f,e,b,d); X(c,a,f,d,e,b);
													X(c,a,f,d,b,e); X(c,a,f,b,e,d); X(c,a,f,b,d,e);
													X(c,f,e,d,b,a); X(c,f,e,d,a,b); X(c,f,e,b,d,a);
													X(c,f,e,b,a,d); X(c,f,e,a,d,b); X(c,f,e,a,b,d);
													X(c,f,d,e,b,a); X(c,f,d,e,a,b); X(c,f,d,b,e,a);
													X(c,f,d,b,a,e); X(c,f,d,a,e,b); X(c,f,d,a,b,e);
													X(c,f,b,e,d,a); X(c,f,b,e,a,d); X(c,f,b,d,e,a);
													X(c,f,b,d,a,e); X(c,f,b,a,e,d); X(c,f,b,a,d,e);
													X(c,f,a,e,d,b); X(c,f,a,e,b,d); X(c,f,a,d,e,b);
													X(c,f,a,d,b,e); X(c,f,a,b,e,d); X(c,f,a,b,d,e);
													X(b,e,d,c,a,f); X(b,e,d,c,f,a); X(b,e,d,a,c,f);
													X(b,e,d,a,f,c); X(b,e,d,f,c,a); X(b,e,d,f,a,c);
													X(b,e,c,d,a,f); X(b,e,c,d,f,a); X(b,e,c,a,d,f);
													X(b,e,c,a,f,d); X(b,e,c,f,d,a); X(b,e,c,f,a,d);
													X(b,e,a,d,c,f); X(b,e,a,d,f,c); X(b,e,a,c,d,f);
													X(b,e,a,c,f,d); X(b,e,a,f,d,c); X(b,e,a,f,c,d);
													X(b,e,f,d,c,a); X(b,e,f,d,a,c); X(b,e,f,c,d,a);
													X(b,e,f,c,a,d); X(b,e,f,a,d,c); X(b,e,f,a,c,d);
													X(b,d,e,c,a,f); X(b,d,e,c,f,a); X(b,d,e,a,c,f);
													X(b,d,e,a,f,c); X(b,d,e,f,c,a); X(b,d,e,f,a,c);
													X(b,d,c,e,a,f); X(b,d,c,e,f,a); X(b,d,c,a,e,f);
													X(b,d,c,a,f,e); X(b,d,c,f,e,a); X(b,d,c,f,a,e);
													X(b,d,a,e,c,f); X(b,d,a,e,f,c); X(b,d,a,c,e,f);
													X(b,d,a,c,f,e); X(b,d,a,f,e,c); X(b,d,a,f,c,e);
													X(b,d,f,e,c,a); X(b,d,f,e,a,c); X(b,d,f,c,e,a);
													X(b,d,f,c,a,e); X(b,d,f,a,e,c); X(b,d,f,a,c,e);
													X(b,c,e,d,a,f); X(b,c,e,d,f,a); X(b,c,e,a,d,f);
													X(b,c,e,a,f,d); X(b,c,e,f,d,a); X(b,c,e,f,a,d);
													X(b,c,d,e,a,f); X(b,c,d,e,f,a); X(b,c,d,a,e,f);
													X(b,c,d,a,f,e); X(b,c,d,f,e,a); X(b,c,d,f,a,e);
													X(b,c,a,e,d,f); X(b,c,a,e,f,d); X(b,c,a,d,e,f);
													X(b,c,a,d,f,e); X(b,c,a,f,e,d); X(b,c,a,f,d,e);
													X(b,c,f,e,d,a); X(b,c,f,e,a,d); X(b,c,f,d,e,a);
													X(b,c,f,d,a,e); X(b,c,f,a,e,d); X(b,c,f,a,d,e);
													X(b,a,e,d,c,f); X(b,a,e,d,f,c); X(b,a,e,c,d,f);
													X(b,a,e,c,f,d); X(b,a,e,f,d,c); X(b,a,e,f,c,d);
													X(b,a,d,e,c,f); X(b,a,d,e,f,c); X(b,a,d,c,e,f);
													X(b,a,d,c,f,e); X(b,a,d,f,e,c); X(b,a,d,f,c,e);
													X(b,a,c,e,d,f); X(b,a,c,e,f,d); X(b,a,c,d,e,f);
													X(b,a,c,d,f,e); X(b,a,c,f,e,d); X(b,a,c,f,d,e);
													X(b,a,f,e,d,c); X(b,a,f,e,c,d); X(b,a,f,d,e,c);
													X(b,a,f,d,c,e); X(b,a,f,c,e,d); X(b,a,f,c,d,e);
													X(b,f,e,d,c,a); X(b,f,e,d,a,c); X(b,f,e,c,d,a);
													X(b,f,e,c,a,d); X(b,f,e,a,d,c); X(b,f,e,a,c,d);
													X(b,f,d,e,c,a); X(b,f,d,e,a,c); X(b,f,d,c,e,a);
													X(b,f,d,c,a,e); X(b,f,d,a,e,c); X(b,f,d,a,c,e);
													X(b,f,c,e,d,a); X(b,f,c,e,a,d); X(b,f,c,d,e,a);
													X(b,f,c,d,a,e); X(b,f,c,a,e,d); X(b,f,c,a,d,e);
													X(b,f,a,e,d,c); X(b,f,a,e,c,d); X(b,f,a,d,e,c);
													X(b,f,a,d,c,e); X(b,f,a,c,e,d); X(b,f,a,c,d,e);
													X(a,e,d,c,b,f); X(a,e,d,c,f,b); X(a,e,d,b,c,f);
													X(a,e,d,b,f,c); X(a,e,d,f,c,b); X(a,e,d,f,b,c);
													X(a,e,c,d,b,f); X(a,e,c,d,f,b); X(a,e,c,b,d,f);
													X(a,e,c,b,f,d); X(a,e,c,f,d,b); X(a,e,c,f,b,d);
													X(a,e,b,d,c,f); X(a,e,b,d,f,c); X(a,e,b,c,d,f);
													X(a,e,b,c,f,d); X(a,e,b,f,d,c); X(a,e,b,f,c,d);
													X(a,e,f,d,c,b); X(a,e,f,d,b,c); X(a,e,f,c,d,b);
													X(a,e,f,c,b,d); X(a,e,f,b,d,c); X(a,e,f,b,c,d);
													X(a,d,e,c,b,f); X(a,d,e,c,f,b); X(a,d,e,b,c,f);
													X(a,d,e,b,f,c); X(a,d,e,f,c,b); X(a,d,e,f,b,c);
													X(a,d,c,e,b,f); X(a,d,c,e,f,b); X(a,d,c,b,e,f);
													X(a,d,c,b,f,e); X(a,d,c,f,e,b); X(a,d,c,f,b,e);
													X(a,d,b,e,c,f); X(a,d,b,e,f,c); X(a,d,b,c,e,f);
													X(a,d,b,c,f,e); X(a,d,b,f,e,c); X(a,d,b,f,c,e);
													X(a,d,f,e,c,b); X(a,d,f,e,b,c); X(a,d,f,c,e,b);
													X(a,d,f,c,b,e); X(a,d,f,b,e,c); X(a,d,f,b,c,e);
													X(a,c,e,d,b,f); X(a,c,e,d,f,b); X(a,c,e,b,d,f);
													X(a,c,e,b,f,d); X(a,c,e,f,d,b); X(a,c,e,f,b,d);
													X(a,c,d,e,b,f); X(a,c,d,e,f,b); X(a,c,d,b,e,f);
													X(a,c,d,b,f,e); X(a,c,d,f,e,b); X(a,c,d,f,b,e);
													X(a,c,b,e,d,f); X(a,c,b,e,f,d); X(a,c,b,d,e,f);
													X(a,c,b,d,f,e); X(a,c,b,f,e,d); X(a,c,b,f,d,e);
													X(a,c,f,e,d,b); X(a,c,f,e,b,d); X(a,c,f,d,e,b);
													X(a,c,f,d,b,e); X(a,c,f,b,e,d); X(a,c,f,b,d,e);
													X(a,b,e,d,c,f); X(a,b,e,d,f,c); X(a,b,e,c,d,f);
													X(a,b,e,c,f,d); X(a,b,e,f,d,c); X(a,b,e,f,c,d);
													X(a,b,d,e,c,f); X(a,b,d,e,f,c); X(a,b,d,c,e,f);
													X(a,b,d,c,f,e); X(a,b,d,f,e,c); X(a,b,d,f,c,e);
													X(a,b,c,e,d,f); X(a,b,c,e,f,d); X(a,b,c,d,e,f);
													X(a,b,c,d,f,e); X(a,b,c,f,e,d); X(a,b,c,f,d,e);
													X(a,b,f,e,d,c); X(a,b,f,e,c,d); X(a,b,f,d,e,c);
													X(a,b,f,d,c,e); X(a,b,f,c,e,d); X(a,b,f,c,d,e);
													X(a,f,e,d,c,b); X(a,f,e,d,b,c); X(a,f,e,c,d,b);
													X(a,f,e,c,b,d); X(a,f,e,b,d,c); X(a,f,e,b,c,d);
													X(a,f,d,e,c,b); X(a,f,d,e,b,c); X(a,f,d,c,e,b);
													X(a,f,d,c,b,e); X(a,f,d,b,e,c); X(a,f,d,b,c,e);
													X(a,f,c,e,d,b); X(a,f,c,e,b,d); X(a,f,c,d,e,b);
													X(a,f,c,d,b,e); X(a,f,c,b,e,d); X(a,f,c,b,d,e);
													X(a,f,b,e,d,c); X(a,f,b,e,c,d); X(a,f,b,d,e,c);
													X(a,f,b,d,c,e); X(a,f,b,c,e,d); X(a,f,b,c,d,e);
													X(f,e,d,c,b,a); X(f,e,d,c,a,b); X(f,e,d,b,c,a);
													X(f,e,d,b,a,c); X(f,e,d,a,c,b); X(f,e,d,a,b,c);
													X(f,e,c,d,b,a); X(f,e,c,d,a,b); X(f,e,c,b,d,a);
													X(f,e,c,b,a,d); X(f,e,c,a,d,b); X(f,e,c,a,b,d);
													X(f,e,b,d,c,a); X(f,e,b,d,a,c); X(f,e,b,c,d,a);
													X(f,e,b,c,a,d); X(f,e,b,a,d,c); X(f,e,b,a,c,d);
													X(f,e,a,d,c,b); X(f,e,a,d,b,c); X(f,e,a,c,d,b);
													X(f,e,a,c,b,d); X(f,e,a,b,d,c); X(f,e,a,b,c,d);
													X(f,d,e,c,b,a); X(f,d,e,c,a,b); X(f,d,e,b,c,a);
													X(f,d,e,b,a,c); X(f,d,e,a,c,b); X(f,d,e,a,b,c);
													X(f,d,c,e,b,a); X(f,d,c,e,a,b); X(f,d,c,b,e,a);
													X(f,d,c,b,a,e); X(f,d,c,a,e,b); X(f,d,c,a,b,e);
													X(f,d,b,e,c,a); X(f,d,b,e,a,c); X(f,d,b,c,e,a);
													X(f,d,b,c,a,e); X(f,d,b,a,e,c); X(f,d,b,a,c,e);
													X(f,d,a,e,c,b); X(f,d,a,e,b,c); X(f,d,a,c,e,b);
													X(f,d,a,c,b,e); X(f,d,a,b,e,c); X(f,d,a,b,c,e);
													X(f,c,e,d,b,a); X(f,c,e,d,a,b); X(f,c,e,b,d,a);
													X(f,c,e,b,a,d); X(f,c,e,a,d,b); X(f,c,e,a,b,d);
													X(f,c,d,e,b,a); X(f,c,d,e,a,b); X(f,c,d,b,e,a);
													X(f,c,d,b,a,e); X(f,c,d,a,e,b); X(f,c,d,a,b,e);
													X(f,c,b,e,d,a); X(f,c,b,e,a,d); X(f,c,b,d,e,a);
													X(f,c,b,d,a,e); X(f,c,b,a,e,d); X(f,c,b,a,d,e);
													X(f,c,a,e,d,b); X(f,c,a,e,b,d); X(f,c,a,d,e,b);
													X(f,c,a,d,b,e); X(f,c,a,b,e,d); X(f,c,a,b,d,e);
													X(f,b,e,d,c,a); X(f,b,e,d,a,c); X(f,b,e,c,d,a);
													X(f,b,e,c,a,d); X(f,b,e,a,d,c); X(f,b,e,a,c,d);
													X(f,b,d,e,c,a); X(f,b,d,e,a,c); X(f,b,d,c,e,a);
													X(f,b,d,c,a,e); X(f,b,d,a,e,c); X(f,b,d,a,c,e);
													X(f,b,c,e,d,a); X(f,b,c,e,a,d); X(f,b,c,d,e,a);
													X(f,b,c,d,a,e); X(f,b,c,a,e,d); X(f,b,c,a,d,e);
													X(f,b,a,e,d,c); X(f,b,a,e,c,d); X(f,b,a,d,e,c);
													X(f,b,a,d,c,e); X(f,b,a,c,e,d); X(f,b,a,c,d,e);
													X(f,a,e,d,c,b); X(f,a,e,d,b,c); X(f,a,e,c,d,b);
													X(f,a,e,c,b,d); X(f,a,e,b,d,c); X(f,a,e,b,c,d);
													X(f,a,d,e,c,b); X(f,a,d,e,b,c); X(f,a,d,c,e,b);
													X(f,a,d,c,b,e); X(f,a,d,b,e,c); X(f,a,d,b,c,e);
													X(f,a,c,e,d,b); X(f,a,c,e,b,d); X(f,a,c,d,e,b);
													X(f,a,c,d,b,e); X(f,a,c,b,e,d); X(f,a,c,b,d,e);
													X(f,a,b,e,d,c); X(f,a,b,e,c,d); X(f,a,b,d,e,c);
													X(f,a,b,d,c,e); X(f,a,b,c,e,d); X(f,a,b,c,d,e);
												} else {
													X(d,d,c,b,a,f); X(d,d,c,b,f,a); X(d,d,c,a,b,f);
													X(d,d,c,a,f,b); X(d,d,c,f,b,a); X(d,d,c,f,a,b);
													X(d,d,b,c,a,f); X(d,d,b,c,f,a); X(d,d,b,a,c,f);
													X(d,d,b,a,f,c); X(d,d,b,f,c,a); X(d,d,b,f,a,c);
													X(d,d,a,c,b,f); X(d,d,a,c,f,b); X(d,d,a,b,c,f);
													X(d,d,a,b,f,c); X(d,d,a,f,c,b); X(d,d,a,f,b,c);
													X(d,d,f,c,b,a); X(d,d,f,c,a,b); X(d,d,f,b,c,a);
													X(d,d,f,b,a,c); X(d,d,f,a,c,b); X(d,d,f,a,b,c);
													X(d,c,d,b,a,f); X(d,c,d,b,f,a); X(d,c,d,a,b,f);
													X(d,c,d,a,f,b); X(d,c,d,f,b,a); X(d,c,d,f,a,b);
													X(d,c,b,d,a,f); X(d,c,b,d,f,a); X(d,c,b,a,d,f);
													X(d,c,b,a,f,d); X(d,c,b,f,d,a); X(d,c,b,f,a,d);
													X(d,c,a,d,b,f); X(d,c,a,d,f,b); X(d,c,a,b,d,f);
													X(d,c,a,b,f,d); X(d,c,a,f,d,b); X(d,c,a,f,b,d);
													X(d,c,f,d,b,a); X(d,c,f,d,a,b); X(d,c,f,b,d,a);
													X(d,c,f,b,a,d); X(d,c,f,a,d,b); X(d,c,f,a,b,d);
													X(d,b,d,c,a,f); X(d,b,d,c,f,a); X(d,b,d,a,c,f);
													X(d,b,d,a,f,c); X(d,b,d,f,c,a); X(d,b,d,f,a,c);
													X(d,b,c,d,a,f); X(d,b,c,d,f,a); X(d,b,c,a,d,f);
													X(d,b,c,a,f,d); X(d,b,c,f,d,a); X(d,b,c,f,a,d);
													X(d,b,a,d,c,f); X(d,b,a,d,f,c); X(d,b,a,c,d,f);
													X(d,b,a,c,f,d); X(d,b,a,f,d,c); X(d,b,a,f,c,d);
													X(d,b,f,d,c,a); X(d,b,f,d,a,c); X(d,b,f,c,d,a);
													X(d,b,f,c,a,d); X(d,b,f,a,d,c); X(d,b,f,a,c,d);
													X(d,a,d,c,b,f); X(d,a,d,c,f,b); X(d,a,d,b,c,f);
													X(d,a,d,b,f,c); X(d,a,d,f,c,b); X(d,a,d,f,b,c);
													X(d,a,c,d,b,f); X(d,a,c,d,f,b); X(d,a,c,b,d,f);
													X(d,a,c,b,f,d); X(d,a,c,f,d,b); X(d,a,c,f,b,d);
													X(d,a,b,d,c,f); X(d,a,b,d,f,c); X(d,a,b,c,d,f);
													X(d,a,b,c,f,d); X(d,a,b,f,d,c); X(d,a,b,f,c,d);
													X(d,a,f,d,c,b); X(d,a,f,d,b,c); X(d,a,f,c,d,b);
													X(d,a,f,c,b,d); X(d,a,f,b,d,c); X(d,a,f,b,c,d);
													X(d,f,d,c,b,a); X(d,f,d,c,a,b); X(d,f,d,b,c,a);
													X(d,f,d,b,a,c); X(d,f,d,a,c,b); X(d,f,d,a,b,c);
													X(d,f,c,d,b,a); X(d,f,c,d,a,b); X(d,f,c,b,d,a);
													X(d,f,c,b,a,d); X(d,f,c,a,d,b); X(d,f,c,a,b,d);
													X(d,f,b,d,c,a); X(d,f,b,d,a,c); X(d,f,b,c,d,a);
													X(d,f,b,c,a,d); X(d,f,b,a,d,c); X(d,f,b,a,c,d);
													X(d,f,a,d,c,b); X(d,f,a,d,b,c); X(d,f,a,c,d,b);
													X(d,f,a,c,b,d); X(d,f,a,b,d,c); X(d,f,a,b,c,d);
													X(c,d,d,b,a,f); X(c,d,d,b,f,a); X(c,d,d,a,b,f);
													X(c,d,d,a,f,b); X(c,d,d,f,b,a); X(c,d,d,f,a,b);
													X(c,d,b,d,a,f); X(c,d,b,d,f,a); X(c,d,b,a,d,f);
													X(c,d,b,a,f,d); X(c,d,b,f,d,a); X(c,d,b,f,a,d);
													X(c,d,a,d,b,f); X(c,d,a,d,f,b); X(c,d,a,b,d,f);
													X(c,d,a,b,f,d); X(c,d,a,f,d,b); X(c,d,a,f,b,d);
													X(c,d,f,d,b,a); X(c,d,f,d,a,b); X(c,d,f,b,d,a);
													X(c,d,f,b,a,d); X(c,d,f,a,d,b); X(c,d,f,a,b,d);
													X(c,b,d,d,a,f); X(c,b,d,d,f,a); X(c,b,d,a,d,f);
													X(c,b,d,a,f,d); X(c,b,d,f,d,a); X(c,b,d,f,a,d);
													X(c,b,a,d,d,f); X(c,b,a,d,f,d); X(c,b,a,f,d,d);
													X(c,b,f,d,d,a); X(c,b,f,d,a,d); X(c,b,f,a,d,d);
													X(c,a,d,d,b,f); X(c,a,d,d,f,b); X(c,a,d,b,d,f);
													X(c,a,d,b,f,d); X(c,a,d,f,d,b); X(c,a,d,f,b,d);
													X(c,a,b,d,d,f); X(c,a,b,d,f,d); X(c,a,b,f,d,d);
													X(c,a,f,d,d,b); X(c,a,f,d,b,d); X(c,a,f,b,d,d);
													X(c,f,d,d,b,a); X(c,f,d,d,a,b); X(c,f,d,b,d,a);
													X(c,f,d,b,a,d); X(c,f,d,a,d,b); X(c,f,d,a,b,d);
													X(c,f,b,d,d,a); X(c,f,b,d,a,d); X(c,f,b,a,d,d);
													X(c,f,a,d,d,b); X(c,f,a,d,b,d); X(c,f,a,b,d,d);
													X(b,d,d,c,a,f); X(b,d,d,c,f,a); X(b,d,d,a,c,f);
													X(b,d,d,a,f,c); X(b,d,d,f,c,a); X(b,d,d,f,a,c);
													X(b,d,c,d,a,f); X(b,d,c,d,f,a); X(b,d,c,a,d,f);
													X(b,d,c,a,f,d); X(b,d,c,f,d,a); X(b,d,c,f,a,d);
													X(b,d,a,d,c,f); X(b,d,a,d,f,c); X(b,d,a,c,d,f);
													X(b,d,a,c,f,d); X(b,d,a,f,d,c); X(b,d,a,f,c,d);
													X(b,d,f,d,c,a); X(b,d,f,d,a,c); X(b,d,f,c,d,a);
													X(b,d,f,c,a,d); X(b,d,f,a,d,c); X(b,d,f,a,c,d);
													X(b,c,d,d,a,f); X(b,c,d,d,f,a); X(b,c,d,a,d,f);
													X(b,c,d,a,f,d); X(b,c,d,f,d,a); X(b,c,d,f,a,d);
													X(b,c,a,d,d,f); X(b,c,a,d,f,d); X(b,c,a,f,d,d);
													X(b,c,f,d,d,a); X(b,c,f,d,a,d); X(b,c,f,a,d,d);
													X(b,a,d,d,c,f); X(b,a,d,d,f,c); X(b,a,d,c,d,f);
													X(b,a,d,c,f,d); X(b,a,d,f,d,c); X(b,a,d,f,c,d);
													X(b,a,c,d,d,f); X(b,a,c,d,f,d); X(b,a,c,f,d,d);
													X(b,a,f,d,d,c); X(b,a,f,d,c,d); X(b,a,f,c,d,d);
													X(b,f,d,d,c,a); X(b,f,d,d,a,c); X(b,f,d,c,d,a);
													X(b,f,d,c,a,d); X(b,f,d,a,d,c); X(b,f,d,a,c,d);
													X(b,f,c,d,d,a); X(b,f,c,d,a,d); X(b,f,c,a,d,d);
													X(b,f,a,d,d,c); X(b,f,a,d,c,d); X(b,f,a,c,d,d);
													X(a,d,d,c,b,f); X(a,d,d,c,f,b); X(a,d,d,b,c,f);
													X(a,d,d,b,f,c); X(a,d,d,f,c,b); X(a,d,d,f,b,c);
													X(a,d,c,d,b,f); X(a,d,c,d,f,b); X(a,d,c,b,d,f);
													X(a,d,c,b,f,d); X(a,d,c,f,d,b); X(a,d,c,f,b,d);
													X(a,d,b,d,c,f); X(a,d,b,d,f,c); X(a,d,b,c,d,f);
													X(a,d,b,c,f,d); X(a,d,b,f,d,c); X(a,d,b,f,c,d);
													X(a,d,f,d,c,b); X(a,d,f,d,b,c); X(a,d,f,c,d,b);
													X(a,d,f,c,b,d); X(a,d,f,b,d,c); X(a,d,f,b,c,d);
													X(a,c,d,d,b,f); X(a,c,d,d,f,b); X(a,c,d,b,d,f);
													X(a,c,d,b,f,d); X(a,c,d,f,d,b); X(a,c,d,f,b,d);
													X(a,c,b,d,d,f); X(a,c,b,d,f,d); X(a,c,b,f,d,d);
													X(a,c,f,d,d,b); X(a,c,f,d,b,d); X(a,c,f,b,d,d);
													X(a,b,d,d,c,f); X(a,b,d,d,f,c); X(a,b,d,c,d,f);
													X(a,b,d,c,f,d); X(a,b,d,f,d,c); X(a,b,d,f,c,d);
													X(a,b,c,d,d,f); X(a,b,c,d,f,d); X(a,b,c,f,d,d);
													X(a,b,f,d,d,c); X(a,b,f,d,c,d); X(a,b,f,c,d,d);
													X(a,f,d,d,c,b); X(a,f,d,d,b,c); X(a,f,d,c,d,b);
													X(a,f,d,c,b,d); X(a,f,d,b,d,c); X(a,f,d,b,c,d);
													X(a,f,c,d,d,b); X(a,f,c,d,b,d); X(a,f,c,b,d,d);
													X(a,f,b,d,d,c); X(a,f,b,d,c,d); X(a,f,b,c,d,d);
													X(f,d,d,c,b,a); X(f,d,d,c,a,b); X(f,d,d,b,c,a);
													X(f,d,d,b,a,c); X(f,d,d,a,c,b); X(f,d,d,a,b,c);
													X(f,d,c,d,b,a); X(f,d,c,d,a,b); X(f,d,c,b,d,a);
													X(f,d,c,b,a,d); X(f,d,c,a,d,b); X(f,d,c,a,b,d);
													X(f,d,b,d,c,a); X(f,d,b,d,a,c); X(f,d,b,c,d,a);
													X(f,d,b,c,a,d); X(f,d,b,a,d,c); X(f,d,b,a,c,d);
													X(f,d,a,d,c,b); X(f,d,a,d,b,c); X(f,d,a,c,d,b);
													X(f,d,a,c,b,d); X(f,d,a,b,d,c); X(f,d,a,b,c,d);
													X(f,c,d,d,b,a); X(f,c,d,d,a,b); X(f,c,d,b,d,a);
													X(f,c,d,b,a,d); X(f,c,d,a,d,b); X(f,c,d,a,b,d);
													X(f,c,b,d,d,a); X(f,c,b,d,a,d); X(f,c,b,a,d,d);
													X(f,c,a,d,d,b); X(f,c,a,d,b,d); X(f,c,a,b,d,d);
													X(f,b,d,d,c,a); X(f,b,d,d,a,c); X(f,b,d,c,d,a);
													X(f,b,d,c,a,d); X(f,b,d,a,d,c); X(f,b,d,a,c,d);
													X(f,b,c,d,d,a); X(f,b,c,d,a,d); X(f,b,c,a,d,d);
													X(f,b,a,d,d,c); X(f,b,a,d,c,d); X(f,b,a,c,d,d);
													X(f,a,d,d,c,b); X(f,a,d,d,b,c); X(f,a,d,c,d,b);
													X(f,a,d,c,b,d); X(f,a,d,b,d,c); X(f,a,d,b,c,d);
													X(f,a,c,d,d,b); X(f,a,c,d,b,d); X(f,a,c,b,d,d);
													X(f,a,b,d,d,c); X(f,a,b,d,c,d); X(f,a,b,c,d,d);
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
#undef DECLARE_POSITION_VALUES
#undef SUMCHECK
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
		if constexpr (width == 19) {
			auto printSplice = [](auto& thing) {
				auto r = thing.get();
				for (const auto& v : r) {
					std::cout << v << std::endl;
				}
			};
			printSplice(t0); printSplice(t1);
			printSplice(t2); printSplice(t3);
			printSplice(t4); printSplice(t5);
			printSplice(t6);
		} else {
			auto getnsplice = [&list](auto& thing) {
				auto r = thing.get();
				list.splice(list.cbegin(), r);
			};
			getnsplice(t0); getnsplice(t1);
			getnsplice(t2); getnsplice(t3);
			getnsplice(t4); getnsplice(t5);
			getnsplice(t6);
		}
	} 
	if constexpr (width != 19) {
		list.sort();
		for (const auto& v : list) {
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
#define X(ind) case ind : initialBody< ind > (); break;
				X(1);  X(2);  X(3);  X(4);  X(5);
				X(6);  X(7);  X(8);  X(9);  X(10);
				X(11); X(12); X(13); X(14); X(15);
				X(16); X(17); X(18); X(19); 
#undef X
				default:
				std::cerr << "Illegal index " << currentIndex << std::endl;
				return 1;
			}
			std::cout << std::endl;
		}
	}
	return 0;
}
