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
	} else if constexpr (length > 10 && (length - position) == 6) {

		// this will generate a partial number but reduce the number of conversions
		// required greatly!
		// The last two digits are handled in a base 10 fashion without the +2 added
		// This will make the partial converison correct (remember that a 0 becomes a 2
		// in this model).
		//
		// Thus we implicitly add the offsets for each position to this base10 2 value :D
		//

		auto outerConverted = convertNumber<length>(index);
#define Y(x,y,z,w,h,q) if (auto n = x ## 1 + y ## 2 + z ## 3 + w ## 4 + h ## 5 + q ## 6; ((n % ep == 0) && (n % es == 0))) { list.emplace_back(n); }
#define X(x,y,z,w,h) Y(x,y,z,w,h,f)
#define SUMCHECK if (es % 3 != 0) { continue; }
#define DECLARE_POSITION_VALUES(var) \
		auto var ## 1 = outerConverted + ( var * p10a ); \
		auto var ## 2 = var * p10b; \
		auto var ## 3 = var * p10c; \
		auto var ## 4 = var * p10d; \
		auto var ## 5 = var * p10e; \
		auto var ## 6 = var * p10f
		for (auto f = 0ul; f < 8ul; ++f) {
			SKIP5s(f);
			DECLARE_POSITION_VALUES(f);
			auto fs = sum + f;
			auto fp = product * (f + 2);
			for (auto a = f; a < 8ul; ++a) {
				SKIP5s(a);
				DECLARE_POSITION_VALUES(a);
				auto as = fs + a;
				auto ap = fp * (a + 2);
				if (a == f) {
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
												SUMCHECK;
												auto ep = dp * (e + 2);
												DECLARE_POSITION_VALUES(e);
												if (d != e) {
													Y(e,d,d,d,d,d); 
													Y(d,e,d,d,d,d); 
													Y(d,d,e,d,d,d); 
													Y(d,d,d,e,d,d); 
													Y(d,d,d,d,e,d); 
													Y(d,d,d,d,d,e); 
												} else {
													Y(d,d,d,d,d,d);
												}
											}
										} else {
											// c != d
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												auto ep = dp * (e + 2);
												if (d != e) {
													DECLARE_POSITION_VALUES(e);
													Y(e,d,c,c,c,c);
													Y(e,c,d,c,c,c);
													Y(e,c,c,d,c,c);
													Y(e,c,c,c,d,c);
													Y(e,c,c,c,c,d);
													Y(d,e,c,c,c,c);
													Y(d,c,e,c,c,c);
													Y(d,c,c,e,c,c);
													Y(d,c,c,c,e,c);
													Y(d,c,c,c,c,e);
													Y(c,e,d,c,c,c);
													Y(c,e,c,d,c,c);
													Y(c,e,c,c,d,c);
													Y(c,e,c,c,c,d);
													Y(c,d,e,c,c,c);
													Y(c,d,c,e,c,c);
													Y(c,d,c,c,e,c);
													Y(c,d,c,c,c,e);
													Y(c,c,e,d,c,c);
													Y(c,c,e,c,d,c);
													Y(c,c,e,c,c,d);
													Y(c,c,d,e,c,c);
													Y(c,c,d,c,e,c);
													Y(c,c,d,c,c,e);
													Y(c,c,c,e,d,c);
													Y(c,c,c,e,c,d);
													Y(c,c,c,d,e,c);
													Y(c,c,c,d,c,e);
													Y(c,c,c,c,e,d);
													Y(c,c,c,c,d,e);
												} else {
													Y(d,d,c,c,c,c);
													Y(d,c,d,c,c,c);
													Y(d,c,c,d,c,c);
													Y(d,c,c,c,d,c);
													Y(d,c,c,c,c,d);
													Y(c,d,d,c,c,c);
													Y(c,d,c,d,c,c);
													Y(c,d,c,c,d,c);
													Y(c,d,c,c,c,d);
													Y(c,c,d,d,c,c);
													Y(c,c,d,c,d,c);
													Y(c,c,d,c,c,d);
													Y(c,c,c,d,d,c);
													Y(c,c,c,d,c,d);
													Y(c,c,c,c,d,d);
												}
											}
										}
									}
								} else {
									// b != c
									for (auto d = c; d < 8ul; ++d) {
										SKIP5s(d);
										auto ds = cs + d;
										auto dp = cp * (d + 2);
										DECLARE_POSITION_VALUES(d);
										if (c == d) {
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												auto ep = dp * (e + 2);
												if (d != e) {
													DECLARE_POSITION_VALUES(e);
													Y(e,c,c,b,b,b);
													Y(e,c,b,c,b,b);
													Y(e,c,b,b,c,b);
													Y(e,c,b,b,b,c);
													Y(e,b,c,c,b,b);
													Y(e,b,c,b,c,b);
													Y(e,b,c,b,b,c);
													Y(e,b,b,c,c,b);
													Y(e,b,b,c,b,c);
													Y(e,b,b,b,c,c);
													Y(c,e,c,b,b,b);
													Y(c,e,b,c,b,b);
													Y(c,e,b,b,c,b);
													Y(c,e,b,b,b,c);
													Y(c,c,e,b,b,b);
													Y(c,c,b,e,b,b);
													Y(c,c,b,b,e,b);
													Y(c,c,b,b,b,e);
													Y(c,b,e,c,b,b);
													Y(c,b,e,b,c,b);
													Y(c,b,e,b,b,c);
													Y(c,b,c,e,b,b);
													Y(c,b,c,b,e,b);
													Y(c,b,c,b,b,e);
													Y(c,b,b,e,c,b);
													Y(c,b,b,e,b,c);
													Y(c,b,b,c,e,b);
													Y(c,b,b,c,b,e);
													Y(c,b,b,b,e,c);
													Y(c,b,b,b,c,e);
													Y(b,e,c,c,b,b);
													Y(b,e,c,b,c,b);
													Y(b,e,c,b,b,c);
													Y(b,e,b,c,c,b);
													Y(b,e,b,c,b,c);
													Y(b,e,b,b,c,c);
													Y(b,c,e,c,b,b);
													Y(b,c,e,b,c,b);
													Y(b,c,e,b,b,c);
													Y(b,c,c,e,b,b);
													Y(b,c,c,b,e,b);
													Y(b,c,c,b,b,e);
													Y(b,c,b,e,c,b);
													Y(b,c,b,e,b,c);
													Y(b,c,b,c,e,b);
													Y(b,c,b,c,b,e);
													Y(b,c,b,b,e,c);
													Y(b,c,b,b,c,e);
													Y(b,b,e,c,c,b);
													Y(b,b,e,c,b,c);
													Y(b,b,e,b,c,c);
													Y(b,b,c,e,c,b);
													Y(b,b,c,e,b,c);
													Y(b,b,c,c,e,b);
													Y(b,b,c,c,b,e);
													Y(b,b,c,b,e,c);
													Y(b,b,c,b,c,e);
													Y(b,b,b,e,c,c);
													Y(b,b,b,c,e,c);
													Y(b,b,b,c,c,e);
												} else {
													Y(c,c,c,b,b,b);
													Y(c,c,b,c,b,b);
													Y(c,c,b,b,c,b);
													Y(c,c,b,b,b,c);
													Y(c,b,c,c,b,b);
													Y(c,b,c,b,c,b);
													Y(c,b,c,b,b,c);
													Y(c,b,b,c,c,b);
													Y(c,b,b,c,b,c);
													Y(c,b,b,b,c,c);
													Y(b,c,c,c,b,b);
													Y(b,c,c,b,c,b);
													Y(b,c,c,b,b,c);
													Y(b,c,b,c,c,b);
													Y(b,c,b,c,b,c);
													Y(b,c,b,b,c,c);
													Y(b,b,c,c,c,b);
													Y(b,b,c,c,b,c);
													Y(b,b,c,b,c,c);
													Y(b,b,b,c,c,c);
												}
											}
										} else {
											// c != d
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												auto ep = dp * (e + 2);
												if (d != e) {
													DECLARE_POSITION_VALUES(e);
													Y(e,d,c,b,b,b);
													Y(e,d,b,c,b,b);
													Y(e,d,b,b,c,b);
													Y(e,d,b,b,b,c);
													Y(e,c,d,b,b,b);
													Y(e,c,b,d,b,b);
													Y(e,c,b,b,d,b);
													Y(e,c,b,b,b,d);
													Y(e,b,d,c,b,b);
													Y(e,b,d,b,c,b);
													Y(e,b,d,b,b,c);
													Y(e,b,c,d,b,b);
													Y(e,b,c,b,d,b);
													Y(e,b,c,b,b,d);
													Y(e,b,b,d,c,b);
													Y(e,b,b,d,b,c);
													Y(e,b,b,c,d,b);
													Y(e,b,b,c,b,d);
													Y(e,b,b,b,d,c);
													Y(e,b,b,b,c,d);
													Y(d,e,c,b,b,b);
													Y(d,e,b,c,b,b);
													Y(d,e,b,b,c,b);
													Y(d,e,b,b,b,c);
													Y(d,c,e,b,b,b);
													Y(d,c,b,e,b,b);
													Y(d,c,b,b,e,b);
													Y(d,c,b,b,b,e);
													Y(d,b,e,c,b,b);
													Y(d,b,e,b,c,b);
													Y(d,b,e,b,b,c);
													Y(d,b,c,e,b,b);
													Y(d,b,c,b,e,b);
													Y(d,b,c,b,b,e);
													Y(d,b,b,e,c,b);
													Y(d,b,b,e,b,c);
													Y(d,b,b,c,e,b);
													Y(d,b,b,c,b,e);
													Y(d,b,b,b,e,c);
													Y(d,b,b,b,c,e);
													Y(c,e,d,b,b,b);
													Y(c,e,b,d,b,b);
													Y(c,e,b,b,d,b);
													Y(c,e,b,b,b,d);
													Y(c,d,e,b,b,b);
													Y(c,d,b,e,b,b);
													Y(c,d,b,b,e,b);
													Y(c,d,b,b,b,e);
													Y(c,b,e,d,b,b);
													Y(c,b,e,b,d,b);
													Y(c,b,e,b,b,d);
													Y(c,b,d,e,b,b);
													Y(c,b,d,b,e,b);
													Y(c,b,d,b,b,e);
													Y(c,b,b,e,d,b);
													Y(c,b,b,e,b,d);
													Y(c,b,b,d,e,b);
													Y(c,b,b,d,b,e);
													Y(c,b,b,b,e,d);
													Y(c,b,b,b,d,e);
													Y(b,e,d,c,b,b);
													Y(b,e,d,b,c,b);
													Y(b,e,d,b,b,c);
													Y(b,e,c,d,b,b);
													Y(b,e,c,b,d,b);
													Y(b,e,c,b,b,d);
													Y(b,e,b,d,c,b);
													Y(b,e,b,d,b,c);
													Y(b,e,b,c,d,b);
													Y(b,e,b,c,b,d);
													Y(b,e,b,b,d,c);
													Y(b,e,b,b,c,d);
													Y(b,d,e,c,b,b);
													Y(b,d,e,b,c,b);
													Y(b,d,e,b,b,c);
													Y(b,d,c,e,b,b);
													Y(b,d,c,b,e,b);
													Y(b,d,c,b,b,e);
													Y(b,d,b,e,c,b);
													Y(b,d,b,e,b,c);
													Y(b,d,b,c,e,b);
													Y(b,d,b,c,b,e);
													Y(b,d,b,b,e,c);
													Y(b,d,b,b,c,e);
													Y(b,c,e,d,b,b);
													Y(b,c,e,b,d,b);
													Y(b,c,e,b,b,d);
													Y(b,c,d,e,b,b);
													Y(b,c,d,b,e,b);
													Y(b,c,d,b,b,e);
													Y(b,c,b,e,d,b);
													Y(b,c,b,e,b,d);
													Y(b,c,b,d,e,b);
													Y(b,c,b,d,b,e);
													Y(b,c,b,b,e,d);
													Y(b,c,b,b,d,e);
													Y(b,b,e,d,c,b);
													Y(b,b,e,d,b,c);
													Y(b,b,e,c,d,b);
													Y(b,b,e,c,b,d);
													Y(b,b,e,b,d,c);
													Y(b,b,e,b,c,d);
													Y(b,b,d,e,c,b);
													Y(b,b,d,e,b,c);
													Y(b,b,d,c,e,b);
													Y(b,b,d,c,b,e);
													Y(b,b,d,b,e,c);
													Y(b,b,d,b,c,e);
													Y(b,b,c,e,d,b);
													Y(b,b,c,e,b,d);
													Y(b,b,c,d,e,b);
													Y(b,b,c,d,b,e);
													Y(b,b,c,b,e,d);
													Y(b,b,c,b,d,e);
													Y(b,b,b,e,d,c);
													Y(b,b,b,e,c,d);
													Y(b,b,b,d,e,c);
													Y(b,b,b,d,c,e);
													Y(b,b,b,c,e,d);
													Y(b,b,b,c,d,e);
												} else {
													// d == e
													Y(d,d,c,b,b,b);
													Y(d,d,b,c,b,b);
													Y(d,d,b,b,c,b);
													Y(d,d,b,b,b,c);
													Y(d,c,d,b,b,b);
													Y(d,c,b,d,b,b);
													Y(d,c,b,b,d,b);
													Y(d,c,b,b,b,d);
													Y(d,b,d,c,b,b);
													Y(d,b,d,b,c,b);
													Y(d,b,d,b,b,c);
													Y(d,b,c,d,b,b);
													Y(d,b,c,b,d,b);
													Y(d,b,c,b,b,d);
													Y(d,b,b,d,c,b);
													Y(d,b,b,d,b,c);
													Y(d,b,b,c,d,b);
													Y(d,b,b,c,b,d);
													Y(d,b,b,b,d,c);
													Y(d,b,b,b,c,d);
													Y(c,d,d,b,b,b);
													Y(c,d,b,d,b,b);
													Y(c,d,b,b,d,b);
													Y(c,d,b,b,b,d);
													Y(c,b,d,d,b,b);
													Y(c,b,d,b,d,b);
													Y(c,b,d,b,b,d);
													Y(c,b,b,d,d,b);
													Y(c,b,b,d,b,d);
													Y(c,b,b,b,d,d);
													Y(b,d,d,c,b,b);
													Y(b,d,d,b,c,b);
													Y(b,d,d,b,b,c);
													Y(b,d,c,d,b,b);
													Y(b,d,c,b,d,b);
													Y(b,d,c,b,b,d);
													Y(b,d,b,d,c,b);
													Y(b,d,b,d,b,c);
													Y(b,d,b,c,d,b);
													Y(b,d,b,c,b,d);
													Y(b,d,b,b,d,c);
													Y(b,d,b,b,c,d);
													Y(b,c,d,d,b,b);
													Y(b,c,d,b,d,b);
													Y(b,c,d,b,b,d);
													Y(b,c,b,d,d,b);
													Y(b,c,b,d,b,d);
													Y(b,c,b,b,d,d);
													Y(b,b,d,d,c,b);
													Y(b,b,d,d,b,c);
													Y(b,b,d,c,d,b);
													Y(b,b,d,c,b,d);
													Y(b,b,d,b,d,c);
													Y(b,b,d,b,c,d);
													Y(b,b,c,d,d,b);
													Y(b,b,c,d,b,d);
													Y(b,b,c,b,d,d);
													Y(b,b,b,d,d,c);
													Y(b,b,b,d,c,d);
													Y(b,b,b,c,d,d);
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
												SUMCHECK;
												auto ep = dp * (e + 2);
												DECLARE_POSITION_VALUES(e);
												if (d != e) {
													Y(e,d,d,d,a,a);
													Y(e,d,d,a,d,a);
													Y(e,d,d,a,a,d);
													Y(e,d,a,d,d,a);
													Y(e,d,a,d,a,d);
													Y(e,d,a,a,d,d);
													Y(e,a,d,d,d,a);
													Y(e,a,d,d,a,d);
													Y(e,a,d,a,d,d);
													Y(e,a,a,d,d,d);
													Y(d,e,d,d,a,a);
													Y(d,e,d,a,d,a);
													Y(d,e,d,a,a,d);
													Y(d,e,a,d,d,a);
													Y(d,e,a,d,a,d);
													Y(d,e,a,a,d,d);
													Y(d,d,e,d,a,a);
													Y(d,d,e,a,d,a);
													Y(d,d,e,a,a,d);
													Y(d,d,d,e,a,a);
													Y(d,d,d,a,e,a);
													Y(d,d,d,a,a,e);
													Y(d,d,a,e,d,a);
													Y(d,d,a,e,a,d);
													Y(d,d,a,d,e,a);
													Y(d,d,a,d,a,e);
													Y(d,d,a,a,e,d);
													Y(d,d,a,a,d,e);
													Y(d,a,e,d,d,a);
													Y(d,a,e,d,a,d);
													Y(d,a,e,a,d,d);
													Y(d,a,d,e,d,a);
													Y(d,a,d,e,a,d);
													Y(d,a,d,d,e,a);
													Y(d,a,d,d,a,e);
													Y(d,a,d,a,e,d);
													Y(d,a,d,a,d,e);
													Y(d,a,a,e,d,d);
													Y(d,a,a,d,e,d);
													Y(d,a,a,d,d,e);
													Y(a,e,d,d,d,a);
													Y(a,e,d,d,a,d);
													Y(a,e,d,a,d,d);
													Y(a,e,a,d,d,d);
													Y(a,d,e,d,d,a);
													Y(a,d,e,d,a,d);
													Y(a,d,e,a,d,d);
													Y(a,d,d,e,d,a);
													Y(a,d,d,e,a,d);
													Y(a,d,d,d,e,a);
													Y(a,d,d,d,a,e);
													Y(a,d,d,a,e,d);
													Y(a,d,d,a,d,e);
													Y(a,d,a,e,d,d);
													Y(a,d,a,d,e,d);
													Y(a,d,a,d,d,e);
													Y(a,a,e,d,d,d);
													Y(a,a,d,e,d,d);
													Y(a,a,d,d,e,d);
													Y(a,a,d,d,d,e);
												} else {

													Y(d,d,d,d,a,a);
													Y(d,d,d,a,d,a);
													Y(d,d,d,a,a,d);
													Y(d,d,a,d,d,a);
													Y(d,d,a,d,a,d);
													Y(d,d,a,a,d,d);
													Y(d,a,d,d,d,a);
													Y(d,a,d,d,a,d);
													Y(d,a,d,a,d,d);
													Y(d,a,a,d,d,d);
													Y(a,d,d,d,d,a);
													Y(a,d,d,d,a,d);
													Y(a,d,d,a,d,d);
													Y(a,d,a,d,d,d);
													Y(a,a,d,d,d,d);
												}
											}
										} else {
											// c != d
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												auto ep = dp * (e + 2);
												if (d != e) {
													DECLARE_POSITION_VALUES(e);

													Y(e,d,c,c,a,a);
													Y(e,d,c,a,c,a);
													Y(e,d,c,a,a,c);
													Y(e,d,a,c,c,a);
													Y(e,d,a,c,a,c);
													Y(e,d,a,a,c,c);
													Y(e,c,d,c,a,a);
													Y(e,c,d,a,c,a);
													Y(e,c,d,a,a,c);
													Y(e,c,c,d,a,a);
													Y(e,c,c,a,d,a);
													Y(e,c,c,a,a,d);
													Y(e,c,a,d,c,a);
													Y(e,c,a,d,a,c);
													Y(e,c,a,c,d,a);
													Y(e,c,a,c,a,d);
													Y(e,c,a,a,d,c);
													Y(e,c,a,a,c,d);
													Y(e,a,d,c,c,a);
													Y(e,a,d,c,a,c);
													Y(e,a,d,a,c,c);
													Y(e,a,c,d,c,a);
													Y(e,a,c,d,a,c);
													Y(e,a,c,c,d,a);
													Y(e,a,c,c,a,d);
													Y(e,a,c,a,d,c);
													Y(e,a,c,a,c,d);
													Y(e,a,a,d,c,c);
													Y(e,a,a,c,d,c);
													Y(e,a,a,c,c,d);
													Y(d,e,c,c,a,a);
													Y(d,e,c,a,c,a);
													Y(d,e,c,a,a,c);
													Y(d,e,a,c,c,a);
													Y(d,e,a,c,a,c);
													Y(d,e,a,a,c,c);
													Y(d,c,e,c,a,a);
													Y(d,c,e,a,c,a);
													Y(d,c,e,a,a,c);
													Y(d,c,c,e,a,a);
													Y(d,c,c,a,e,a);
													Y(d,c,c,a,a,e);
													Y(d,c,a,e,c,a);
													Y(d,c,a,e,a,c);
													Y(d,c,a,c,e,a);
													Y(d,c,a,c,a,e);
													Y(d,c,a,a,e,c);
													Y(d,c,a,a,c,e);
													Y(d,a,e,c,c,a);
													Y(d,a,e,c,a,c);
													Y(d,a,e,a,c,c);
													Y(d,a,c,e,c,a);
													Y(d,a,c,e,a,c);
													Y(d,a,c,c,e,a);
													Y(d,a,c,c,a,e);
													Y(d,a,c,a,e,c);
													Y(d,a,c,a,c,e);
													Y(d,a,a,e,c,c);
													Y(d,a,a,c,e,c);
													Y(d,a,a,c,c,e);
													Y(c,e,d,c,a,a);
													Y(c,e,d,a,c,a);
													Y(c,e,d,a,a,c);
													Y(c,e,c,d,a,a);
													Y(c,e,c,a,d,a);
													Y(c,e,c,a,a,d);
													Y(c,e,a,d,c,a);
													Y(c,e,a,d,a,c);
													Y(c,e,a,c,d,a);
													Y(c,e,a,c,a,d);
													Y(c,e,a,a,d,c);
													Y(c,e,a,a,c,d);
													Y(c,d,e,c,a,a);
													Y(c,d,e,a,c,a);
													Y(c,d,e,a,a,c);
													Y(c,d,c,e,a,a);
													Y(c,d,c,a,e,a);
													Y(c,d,c,a,a,e);
													Y(c,d,a,e,c,a);
													Y(c,d,a,e,a,c);
													Y(c,d,a,c,e,a);
													Y(c,d,a,c,a,e);
													Y(c,d,a,a,e,c);
													Y(c,d,a,a,c,e);
													Y(c,c,e,d,a,a);
													Y(c,c,e,a,d,a);
													Y(c,c,e,a,a,d);
													Y(c,c,d,e,a,a);
													Y(c,c,d,a,e,a);
													Y(c,c,d,a,a,e);
													Y(c,c,a,e,d,a);
													Y(c,c,a,e,a,d);
													Y(c,c,a,d,e,a);
													Y(c,c,a,d,a,e);
													Y(c,c,a,a,e,d);
													Y(c,c,a,a,d,e);
													Y(c,a,e,d,c,a);
													Y(c,a,e,d,a,c);
													Y(c,a,e,c,d,a);
													Y(c,a,e,c,a,d);
													Y(c,a,e,a,d,c);
													Y(c,a,e,a,c,d);
													Y(c,a,d,e,c,a);
													Y(c,a,d,e,a,c);
													Y(c,a,d,c,e,a);
													Y(c,a,d,c,a,e);
													Y(c,a,d,a,e,c);
													Y(c,a,d,a,c,e);
													Y(c,a,c,e,d,a);
													Y(c,a,c,e,a,d);
													Y(c,a,c,d,e,a);
													Y(c,a,c,d,a,e);
													Y(c,a,c,a,e,d);
													Y(c,a,c,a,d,e);
													Y(c,a,a,e,d,c);
													Y(c,a,a,e,c,d);
													Y(c,a,a,d,e,c);
													Y(c,a,a,d,c,e);
													Y(c,a,a,c,e,d);
													Y(c,a,a,c,d,e);
													Y(a,e,d,c,c,a);
													Y(a,e,d,c,a,c);
													Y(a,e,d,a,c,c);
													Y(a,e,c,d,c,a);
													Y(a,e,c,d,a,c);
													Y(a,e,c,c,d,a);
													Y(a,e,c,c,a,d);
													Y(a,e,c,a,d,c);
													Y(a,e,c,a,c,d);
													Y(a,e,a,d,c,c);
													Y(a,e,a,c,d,c);
													Y(a,e,a,c,c,d);
													Y(a,d,e,c,c,a);
													Y(a,d,e,c,a,c);
													Y(a,d,e,a,c,c);
													Y(a,d,c,e,c,a);
													Y(a,d,c,e,a,c);
													Y(a,d,c,c,e,a);
													Y(a,d,c,c,a,e);
													Y(a,d,c,a,e,c);
													Y(a,d,c,a,c,e);
													Y(a,d,a,e,c,c);
													Y(a,d,a,c,e,c);
													Y(a,d,a,c,c,e);
													Y(a,c,e,d,c,a);
													Y(a,c,e,d,a,c);
													Y(a,c,e,c,d,a);
													Y(a,c,e,c,a,d);
													Y(a,c,e,a,d,c);
													Y(a,c,e,a,c,d);
													Y(a,c,d,e,c,a);
													Y(a,c,d,e,a,c);
													Y(a,c,d,c,e,a);
													Y(a,c,d,c,a,e);
													Y(a,c,d,a,e,c);
													Y(a,c,d,a,c,e);
													Y(a,c,c,e,d,a);
													Y(a,c,c,e,a,d);
													Y(a,c,c,d,e,a);
													Y(a,c,c,d,a,e);
													Y(a,c,c,a,e,d);
													Y(a,c,c,a,d,e);
													Y(a,c,a,e,d,c);
													Y(a,c,a,e,c,d);
													Y(a,c,a,d,e,c);
													Y(a,c,a,d,c,e);
													Y(a,c,a,c,e,d);
													Y(a,c,a,c,d,e);
													Y(a,a,e,d,c,c);
													Y(a,a,e,c,d,c);
													Y(a,a,e,c,c,d);
													Y(a,a,d,e,c,c);
													Y(a,a,d,c,e,c);
													Y(a,a,d,c,c,e);
													Y(a,a,c,e,d,c);
													Y(a,a,c,e,c,d);
													Y(a,a,c,d,e,c);
													Y(a,a,c,d,c,e);
													Y(a,a,c,c,e,d);
													Y(a,a,c,c,d,e);
												} else {
													Y(d,d,c,c,a,a);
													Y(d,d,c,a,c,a);
													Y(d,d,c,a,a,c);
													Y(d,d,a,c,c,a);
													Y(d,d,a,c,a,c);
													Y(d,d,a,a,c,c);
													Y(d,c,d,c,a,a);
													Y(d,c,d,a,c,a);
													Y(d,c,d,a,a,c);
													Y(d,c,c,d,a,a);
													Y(d,c,c,a,d,a);
													Y(d,c,c,a,a,d);
													Y(d,c,a,d,c,a);
													Y(d,c,a,d,a,c);
													Y(d,c,a,c,d,a);
													Y(d,c,a,c,a,d);
													Y(d,c,a,a,d,c);
													Y(d,c,a,a,c,d);
													Y(d,a,d,c,c,a);
													Y(d,a,d,c,a,c);
													Y(d,a,d,a,c,c);
													Y(d,a,c,d,c,a);
													Y(d,a,c,d,a,c);
													Y(d,a,c,c,d,a);
													Y(d,a,c,c,a,d);
													Y(d,a,c,a,d,c);
													Y(d,a,c,a,c,d);
													Y(d,a,a,d,c,c);
													Y(d,a,a,c,d,c);
													Y(d,a,a,c,c,d);
													Y(c,d,d,c,a,a);
													Y(c,d,d,a,c,a);
													Y(c,d,d,a,a,c);
													Y(c,d,c,d,a,a);
													Y(c,d,c,a,d,a);
													Y(c,d,c,a,a,d);
													Y(c,d,a,d,c,a);
													Y(c,d,a,d,a,c);
													Y(c,d,a,c,d,a);
													Y(c,d,a,c,a,d);
													Y(c,d,a,a,d,c);
													Y(c,d,a,a,c,d);
													Y(c,c,d,d,a,a);
													Y(c,c,d,a,d,a);
													Y(c,c,d,a,a,d);
													Y(c,c,a,d,d,a);
													Y(c,c,a,d,a,d);
													Y(c,c,a,a,d,d);
													Y(c,a,d,d,c,a);
													Y(c,a,d,d,a,c);
													Y(c,a,d,c,d,a);
													Y(c,a,d,c,a,d);
													Y(c,a,d,a,d,c);
													Y(c,a,d,a,c,d);
													Y(c,a,c,d,d,a);
													Y(c,a,c,d,a,d);
													Y(c,a,c,a,d,d);
													Y(c,a,a,d,d,c);
													Y(c,a,a,d,c,d);
													Y(c,a,a,c,d,d);
													Y(a,d,d,c,c,a);
													Y(a,d,d,c,a,c);
													Y(a,d,d,a,c,c);
													Y(a,d,c,d,c,a);
													Y(a,d,c,d,a,c);
													Y(a,d,c,c,d,a);
													Y(a,d,c,c,a,d);
													Y(a,d,c,a,d,c);
													Y(a,d,c,a,c,d);
													Y(a,d,a,d,c,c);
													Y(a,d,a,c,d,c);
													Y(a,d,a,c,c,d);
													Y(a,c,d,d,c,a);
													Y(a,c,d,d,a,c);
													Y(a,c,d,c,d,a);
													Y(a,c,d,c,a,d);
													Y(a,c,d,a,d,c);
													Y(a,c,d,a,c,d);
													Y(a,c,c,d,d,a);
													Y(a,c,c,d,a,d);
													Y(a,c,c,a,d,d);
													Y(a,c,a,d,d,c);
													Y(a,c,a,d,c,d);
													Y(a,c,a,c,d,d);
													Y(a,a,d,d,c,c);
													Y(a,a,d,c,d,c);
													Y(a,a,d,c,c,d);
													Y(a,a,c,d,d,c);
													Y(a,a,c,d,c,d);
													Y(a,a,c,c,d,d);
												}
											}
										}
									}
								} else {
									// b != c
									for (auto d = c; d < 8ul; ++d) {
										SKIP5s(d);
										auto ds = cs + d;
										auto dp = cp * (d + 2);
										DECLARE_POSITION_VALUES(d);
										if (c == d) {
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												auto ep = dp * (e + 2);
												if (d != e) {
													DECLARE_POSITION_VALUES(e);

													Y(e,d,d,b,a,a);
													Y(e,d,d,a,b,a);
													Y(e,d,d,a,a,b);
													Y(e,d,b,d,a,a);
													Y(e,d,b,a,d,a);
													Y(e,d,b,a,a,d);
													Y(e,d,a,d,b,a);
													Y(e,d,a,d,a,b);
													Y(e,d,a,b,d,a);
													Y(e,d,a,b,a,d);
													Y(e,d,a,a,d,b);
													Y(e,d,a,a,b,d);
													Y(e,b,d,d,a,a);
													Y(e,b,d,a,d,a);
													Y(e,b,d,a,a,d);
													Y(e,b,a,d,d,a);
													Y(e,b,a,d,a,d);
													Y(e,b,a,a,d,d);
													Y(e,a,d,d,b,a);
													Y(e,a,d,d,a,b);
													Y(e,a,d,b,d,a);
													Y(e,a,d,b,a,d);
													Y(e,a,d,a,d,b);
													Y(e,a,d,a,b,d);
													Y(e,a,b,d,d,a);
													Y(e,a,b,d,a,d);
													Y(e,a,b,a,d,d);
													Y(e,a,a,d,d,b);
													Y(e,a,a,d,b,d);
													Y(e,a,a,b,d,d);
													Y(d,e,d,b,a,a);
													Y(d,e,d,a,b,a);
													Y(d,e,d,a,a,b);
													Y(d,e,b,d,a,a);
													Y(d,e,b,a,d,a);
													Y(d,e,b,a,a,d);
													Y(d,e,a,d,b,a);
													Y(d,e,a,d,a,b);
													Y(d,e,a,b,d,a);
													Y(d,e,a,b,a,d);
													Y(d,e,a,a,d,b);
													Y(d,e,a,a,b,d);
													Y(d,d,e,b,a,a);
													Y(d,d,e,a,b,a);
													Y(d,d,e,a,a,b);
													Y(d,d,b,e,a,a);
													Y(d,d,b,a,e,a);
													Y(d,d,b,a,a,e);
													Y(d,d,a,e,b,a);
													Y(d,d,a,e,a,b);
													Y(d,d,a,b,e,a);
													Y(d,d,a,b,a,e);
													Y(d,d,a,a,e,b);
													Y(d,d,a,a,b,e);
													Y(d,b,e,d,a,a);
													Y(d,b,e,a,d,a);
													Y(d,b,e,a,a,d);
													Y(d,b,d,e,a,a);
													Y(d,b,d,a,e,a);
													Y(d,b,d,a,a,e);
													Y(d,b,a,e,d,a);
													Y(d,b,a,e,a,d);
													Y(d,b,a,d,e,a);
													Y(d,b,a,d,a,e);
													Y(d,b,a,a,e,d);
													Y(d,b,a,a,d,e);
													Y(d,a,e,d,b,a);
													Y(d,a,e,d,a,b);
													Y(d,a,e,b,d,a);
													Y(d,a,e,b,a,d);
													Y(d,a,e,a,d,b);
													Y(d,a,e,a,b,d);
													Y(d,a,d,e,b,a);
													Y(d,a,d,e,a,b);
													Y(d,a,d,b,e,a);
													Y(d,a,d,b,a,e);
													Y(d,a,d,a,e,b);
													Y(d,a,d,a,b,e);
													Y(d,a,b,e,d,a);
													Y(d,a,b,e,a,d);
													Y(d,a,b,d,e,a);
													Y(d,a,b,d,a,e);
													Y(d,a,b,a,e,d);
													Y(d,a,b,a,d,e);
													Y(d,a,a,e,d,b);
													Y(d,a,a,e,b,d);
													Y(d,a,a,d,e,b);
													Y(d,a,a,d,b,e);
													Y(d,a,a,b,e,d);
													Y(d,a,a,b,d,e);
													Y(b,e,d,d,a,a);
													Y(b,e,d,a,d,a);
													Y(b,e,d,a,a,d);
													Y(b,e,a,d,d,a);
													Y(b,e,a,d,a,d);
													Y(b,e,a,a,d,d);
													Y(b,d,e,d,a,a);
													Y(b,d,e,a,d,a);
													Y(b,d,e,a,a,d);
													Y(b,d,d,e,a,a);
													Y(b,d,d,a,e,a);
													Y(b,d,d,a,a,e);
													Y(b,d,a,e,d,a);
													Y(b,d,a,e,a,d);
													Y(b,d,a,d,e,a);
													Y(b,d,a,d,a,e);
													Y(b,d,a,a,e,d);
													Y(b,d,a,a,d,e);
													Y(b,a,e,d,d,a);
													Y(b,a,e,d,a,d);
													Y(b,a,e,a,d,d);
													Y(b,a,d,e,d,a);
													Y(b,a,d,e,a,d);
													Y(b,a,d,d,e,a);
													Y(b,a,d,d,a,e);
													Y(b,a,d,a,e,d);
													Y(b,a,d,a,d,e);
													Y(b,a,a,e,d,d);
													Y(b,a,a,d,e,d);
													Y(b,a,a,d,d,e);
													Y(a,e,d,d,b,a);
													Y(a,e,d,d,a,b);
													Y(a,e,d,b,d,a);
													Y(a,e,d,b,a,d);
													Y(a,e,d,a,d,b);
													Y(a,e,d,a,b,d);
													Y(a,e,b,d,d,a);
													Y(a,e,b,d,a,d);
													Y(a,e,b,a,d,d);
													Y(a,e,a,d,d,b);
													Y(a,e,a,d,b,d);
													Y(a,e,a,b,d,d);
													Y(a,d,e,d,b,a);
													Y(a,d,e,d,a,b);
													Y(a,d,e,b,d,a);
													Y(a,d,e,b,a,d);
													Y(a,d,e,a,d,b);
													Y(a,d,e,a,b,d);
													Y(a,d,d,e,b,a);
													Y(a,d,d,e,a,b);
													Y(a,d,d,b,e,a);
													Y(a,d,d,b,a,e);
													Y(a,d,d,a,e,b);
													Y(a,d,d,a,b,e);
													Y(a,d,b,e,d,a);
													Y(a,d,b,e,a,d);
													Y(a,d,b,d,e,a);
													Y(a,d,b,d,a,e);
													Y(a,d,b,a,e,d);
													Y(a,d,b,a,d,e);
													Y(a,d,a,e,d,b);
													Y(a,d,a,e,b,d);
													Y(a,d,a,d,e,b);
													Y(a,d,a,d,b,e);
													Y(a,d,a,b,e,d);
													Y(a,d,a,b,d,e);
													Y(a,b,e,d,d,a);
													Y(a,b,e,d,a,d);
													Y(a,b,e,a,d,d);
													Y(a,b,d,e,d,a);
													Y(a,b,d,e,a,d);
													Y(a,b,d,d,e,a);
													Y(a,b,d,d,a,e);
													Y(a,b,d,a,e,d);
													Y(a,b,d,a,d,e);
													Y(a,b,a,e,d,d);
													Y(a,b,a,d,e,d);
													Y(a,b,a,d,d,e);
													Y(a,a,e,d,d,b);
													Y(a,a,e,d,b,d);
													Y(a,a,e,b,d,d);
													Y(a,a,d,e,d,b);
													Y(a,a,d,e,b,d);
													Y(a,a,d,d,e,b);
													Y(a,a,d,d,b,e);
													Y(a,a,d,b,e,d);
													Y(a,a,d,b,d,e);
													Y(a,a,b,e,d,d);
													Y(a,a,b,d,e,d);
													Y(a,a,b,d,d,e);
												} else {
													Y(d,d,d,b,a,a);
													Y(d,d,d,a,b,a);
													Y(d,d,d,a,a,b);
													Y(d,d,b,d,a,a);
													Y(d,d,b,a,d,a);
													Y(d,d,b,a,a,d);
													Y(d,d,a,d,b,a);
													Y(d,d,a,d,a,b);
													Y(d,d,a,b,d,a);
													Y(d,d,a,b,a,d);
													Y(d,d,a,a,d,b);
													Y(d,d,a,a,b,d);
													Y(d,b,d,d,a,a);
													Y(d,b,d,a,d,a);
													Y(d,b,d,a,a,d);
													Y(d,b,a,d,d,a);
													Y(d,b,a,d,a,d);
													Y(d,b,a,a,d,d);
													Y(d,a,d,d,b,a);
													Y(d,a,d,d,a,b);
													Y(d,a,d,b,d,a);
													Y(d,a,d,b,a,d);
													Y(d,a,d,a,d,b);
													Y(d,a,d,a,b,d);
													Y(d,a,b,d,d,a);
													Y(d,a,b,d,a,d);
													Y(d,a,b,a,d,d);
													Y(d,a,a,d,d,b);
													Y(d,a,a,d,b,d);
													Y(d,a,a,b,d,d);
													Y(b,d,d,d,a,a);
													Y(b,d,d,a,d,a);
													Y(b,d,d,a,a,d);
													Y(b,d,a,d,d,a);
													Y(b,d,a,d,a,d);
													Y(b,d,a,a,d,d);
													Y(b,a,d,d,d,a);
													Y(b,a,d,d,a,d);
													Y(b,a,d,a,d,d);
													Y(b,a,a,d,d,d);
													Y(a,d,d,d,b,a);
													Y(a,d,d,d,a,b);
													Y(a,d,d,b,d,a);
													Y(a,d,d,b,a,d);
													Y(a,d,d,a,d,b);
													Y(a,d,d,a,b,d);
													Y(a,d,b,d,d,a);
													Y(a,d,b,d,a,d);
													Y(a,d,b,a,d,d);
													Y(a,d,a,d,d,b);
													Y(a,d,a,d,b,d);
													Y(a,d,a,b,d,d);
													Y(a,b,d,d,d,a);
													Y(a,b,d,d,a,d);
													Y(a,b,d,a,d,d);
													Y(a,b,a,d,d,d);
													Y(a,a,d,d,d,b);
													Y(a,a,d,d,b,d);
													Y(a,a,d,b,d,d);
													Y(a,a,b,d,d,d);

												}
											}
										} else {
											// c != d
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												auto ep = dp * (e + 2);
												if (d != e) {
													Y(e,d,c,b,a,a);
													Y(e,d,c,a,b,a);
													Y(e,d,c,a,a,b);
													Y(e,d,b,c,a,a);
													Y(e,d,b,a,c,a);
													Y(e,d,b,a,a,c);
													Y(e,d,a,c,b,a);
													Y(e,d,a,c,a,b);
													Y(e,d,a,b,c,a);
													Y(e,d,a,b,a,c);
													Y(e,d,a,a,c,b);
													Y(e,d,a,a,b,c);
													Y(e,c,d,b,a,a);
													Y(e,c,d,a,b,a);
													Y(e,c,d,a,a,b);
													Y(e,c,b,d,a,a);
													Y(e,c,b,a,d,a);
													Y(e,c,b,a,a,d);
													Y(e,c,a,d,b,a);
													Y(e,c,a,d,a,b);
													Y(e,c,a,b,d,a);
													Y(e,c,a,b,a,d);
													Y(e,c,a,a,d,b);
													Y(e,c,a,a,b,d);
													Y(e,b,d,c,a,a);
													Y(e,b,d,a,c,a);
													Y(e,b,d,a,a,c);
													Y(e,b,c,d,a,a);
													Y(e,b,c,a,d,a);
													Y(e,b,c,a,a,d);
													Y(e,b,a,d,c,a);
													Y(e,b,a,d,a,c);
													Y(e,b,a,c,d,a);
													Y(e,b,a,c,a,d);
													Y(e,b,a,a,d,c);
													Y(e,b,a,a,c,d);
													Y(e,a,d,c,b,a);
													Y(e,a,d,c,a,b);
													Y(e,a,d,b,c,a);
													Y(e,a,d,b,a,c);
													Y(e,a,d,a,c,b);
													Y(e,a,d,a,b,c);
													Y(e,a,c,d,b,a);
													Y(e,a,c,d,a,b);
													Y(e,a,c,b,d,a);
													Y(e,a,c,b,a,d);
													Y(e,a,c,a,d,b);
													Y(e,a,c,a,b,d);
													Y(e,a,b,d,c,a);
													Y(e,a,b,d,a,c);
													Y(e,a,b,c,d,a);
													Y(e,a,b,c,a,d);
													Y(e,a,b,a,d,c);
													Y(e,a,b,a,c,d);
													Y(e,a,a,d,c,b);
													Y(e,a,a,d,b,c);
													Y(e,a,a,c,d,b);
													Y(e,a,a,c,b,d);
													Y(e,a,a,b,d,c);
													Y(e,a,a,b,c,d);
													Y(d,e,c,b,a,a);
													Y(d,e,c,a,b,a);
													Y(d,e,c,a,a,b);
													Y(d,e,b,c,a,a);
													Y(d,e,b,a,c,a);
													Y(d,e,b,a,a,c);
													Y(d,e,a,c,b,a);
													Y(d,e,a,c,a,b);
													Y(d,e,a,b,c,a);
													Y(d,e,a,b,a,c);
													Y(d,e,a,a,c,b);
													Y(d,e,a,a,b,c);
													Y(d,c,e,b,a,a);
													Y(d,c,e,a,b,a);
													Y(d,c,e,a,a,b);
													Y(d,c,b,e,a,a);
													Y(d,c,b,a,e,a);
													Y(d,c,b,a,a,e);
													Y(d,c,a,e,b,a);
													Y(d,c,a,e,a,b);
													Y(d,c,a,b,e,a);
													Y(d,c,a,b,a,e);
													Y(d,c,a,a,e,b);
													Y(d,c,a,a,b,e);
													Y(d,b,e,c,a,a);
													Y(d,b,e,a,c,a);
													Y(d,b,e,a,a,c);
													Y(d,b,c,e,a,a);
													Y(d,b,c,a,e,a);
													Y(d,b,c,a,a,e);
													Y(d,b,a,e,c,a);
													Y(d,b,a,e,a,c);
													Y(d,b,a,c,e,a);
													Y(d,b,a,c,a,e);
													Y(d,b,a,a,e,c);
													Y(d,b,a,a,c,e);
													Y(d,a,e,c,b,a);
													Y(d,a,e,c,a,b);
													Y(d,a,e,b,c,a);
													Y(d,a,e,b,a,c);
													Y(d,a,e,a,c,b);
													Y(d,a,e,a,b,c);
													Y(d,a,c,e,b,a);
													Y(d,a,c,e,a,b);
													Y(d,a,c,b,e,a);
													Y(d,a,c,b,a,e);
													Y(d,a,c,a,e,b);
													Y(d,a,c,a,b,e);
													Y(d,a,b,e,c,a);
													Y(d,a,b,e,a,c);
													Y(d,a,b,c,e,a);
													Y(d,a,b,c,a,e);
													Y(d,a,b,a,e,c);
													Y(d,a,b,a,c,e);
													Y(d,a,a,e,c,b);
													Y(d,a,a,e,b,c);
													Y(d,a,a,c,e,b);
													Y(d,a,a,c,b,e);
													Y(d,a,a,b,e,c);
													Y(d,a,a,b,c,e);
													Y(c,e,d,b,a,a);
													Y(c,e,d,a,b,a);
													Y(c,e,d,a,a,b);
													Y(c,e,b,d,a,a);
													Y(c,e,b,a,d,a);
													Y(c,e,b,a,a,d);
													Y(c,e,a,d,b,a);
													Y(c,e,a,d,a,b);
													Y(c,e,a,b,d,a);
													Y(c,e,a,b,a,d);
													Y(c,e,a,a,d,b);
													Y(c,e,a,a,b,d);
													Y(c,d,e,b,a,a);
													Y(c,d,e,a,b,a);
													Y(c,d,e,a,a,b);
													Y(c,d,b,e,a,a);
													Y(c,d,b,a,e,a);
													Y(c,d,b,a,a,e);
													Y(c,d,a,e,b,a);
													Y(c,d,a,e,a,b);
													Y(c,d,a,b,e,a);
													Y(c,d,a,b,a,e);
													Y(c,d,a,a,e,b);
													Y(c,d,a,a,b,e);
													Y(c,b,e,d,a,a);
													Y(c,b,e,a,d,a);
													Y(c,b,e,a,a,d);
													Y(c,b,d,e,a,a);
													Y(c,b,d,a,e,a);
													Y(c,b,d,a,a,e);
													Y(c,b,a,e,d,a);
													Y(c,b,a,e,a,d);
													Y(c,b,a,d,e,a);
													Y(c,b,a,d,a,e);
													Y(c,b,a,a,e,d);
													Y(c,b,a,a,d,e);
													Y(c,a,e,d,b,a);
													Y(c,a,e,d,a,b);
													Y(c,a,e,b,d,a);
													Y(c,a,e,b,a,d);
													Y(c,a,e,a,d,b);
													Y(c,a,e,a,b,d);
													Y(c,a,d,e,b,a);
													Y(c,a,d,e,a,b);
													Y(c,a,d,b,e,a);
													Y(c,a,d,b,a,e);
													Y(c,a,d,a,e,b);
													Y(c,a,d,a,b,e);
													Y(c,a,b,e,d,a);
													Y(c,a,b,e,a,d);
													Y(c,a,b,d,e,a);
													Y(c,a,b,d,a,e);
													Y(c,a,b,a,e,d);
													Y(c,a,b,a,d,e);
													Y(c,a,a,e,d,b);
													Y(c,a,a,e,b,d);
													Y(c,a,a,d,e,b);
													Y(c,a,a,d,b,e);
													Y(c,a,a,b,e,d);
													Y(c,a,a,b,d,e);
													Y(b,e,d,c,a,a);
													Y(b,e,d,a,c,a);
													Y(b,e,d,a,a,c);
													Y(b,e,c,d,a,a);
													Y(b,e,c,a,d,a);
													Y(b,e,c,a,a,d);
													Y(b,e,a,d,c,a);
													Y(b,e,a,d,a,c);
													Y(b,e,a,c,d,a);
													Y(b,e,a,c,a,d);
													Y(b,e,a,a,d,c);
													Y(b,e,a,a,c,d);
													Y(b,d,e,c,a,a);
													Y(b,d,e,a,c,a);
													Y(b,d,e,a,a,c);
													Y(b,d,c,e,a,a);
													Y(b,d,c,a,e,a);
													Y(b,d,c,a,a,e);
													Y(b,d,a,e,c,a);
													Y(b,d,a,e,a,c);
													Y(b,d,a,c,e,a);
													Y(b,d,a,c,a,e);
													Y(b,d,a,a,e,c);
													Y(b,d,a,a,c,e);
													Y(b,c,e,d,a,a);
													Y(b,c,e,a,d,a);
													Y(b,c,e,a,a,d);
													Y(b,c,d,e,a,a);
													Y(b,c,d,a,e,a);
													Y(b,c,d,a,a,e);
													Y(b,c,a,e,d,a);
													Y(b,c,a,e,a,d);
													Y(b,c,a,d,e,a);
													Y(b,c,a,d,a,e);
													Y(b,c,a,a,e,d);
													Y(b,c,a,a,d,e);
													Y(b,a,e,d,c,a);
													Y(b,a,e,d,a,c);
													Y(b,a,e,c,d,a);
													Y(b,a,e,c,a,d);
													Y(b,a,e,a,d,c);
													Y(b,a,e,a,c,d);
													Y(b,a,d,e,c,a);
													Y(b,a,d,e,a,c);
													Y(b,a,d,c,e,a);
													Y(b,a,d,c,a,e);
													Y(b,a,d,a,e,c);
													Y(b,a,d,a,c,e);
													Y(b,a,c,e,d,a);
													Y(b,a,c,e,a,d);
													Y(b,a,c,d,e,a);
													Y(b,a,c,d,a,e);
													Y(b,a,c,a,e,d);
													Y(b,a,c,a,d,e);
													Y(b,a,a,e,d,c);
													Y(b,a,a,e,c,d);
													Y(b,a,a,d,e,c);
													Y(b,a,a,d,c,e);
													Y(b,a,a,c,e,d);
													Y(b,a,a,c,d,e);
													Y(a,e,d,c,b,a);
													Y(a,e,d,c,a,b);
													Y(a,e,d,b,c,a);
													Y(a,e,d,b,a,c);
													Y(a,e,d,a,c,b);
													Y(a,e,d,a,b,c);
													Y(a,e,c,d,b,a);
													Y(a,e,c,d,a,b);
													Y(a,e,c,b,d,a);
													Y(a,e,c,b,a,d);
													Y(a,e,c,a,d,b);
													Y(a,e,c,a,b,d);
													Y(a,e,b,d,c,a);
													Y(a,e,b,d,a,c);
													Y(a,e,b,c,d,a);
													Y(a,e,b,c,a,d);
													Y(a,e,b,a,d,c);
													Y(a,e,b,a,c,d);
													Y(a,e,a,d,c,b);
													Y(a,e,a,d,b,c);
													Y(a,e,a,c,d,b);
													Y(a,e,a,c,b,d);
													Y(a,e,a,b,d,c);
													Y(a,e,a,b,c,d);
													Y(a,d,e,c,b,a);
													Y(a,d,e,c,a,b);
													Y(a,d,e,b,c,a);
													Y(a,d,e,b,a,c);
													Y(a,d,e,a,c,b);
													Y(a,d,e,a,b,c);
													Y(a,d,c,e,b,a);
													Y(a,d,c,e,a,b);
													Y(a,d,c,b,e,a);
													Y(a,d,c,b,a,e);
													Y(a,d,c,a,e,b);
													Y(a,d,c,a,b,e);
													Y(a,d,b,e,c,a);
													Y(a,d,b,e,a,c);
													Y(a,d,b,c,e,a);
													Y(a,d,b,c,a,e);
													Y(a,d,b,a,e,c);
													Y(a,d,b,a,c,e);
													Y(a,d,a,e,c,b);
													Y(a,d,a,e,b,c);
													Y(a,d,a,c,e,b);
													Y(a,d,a,c,b,e);
													Y(a,d,a,b,e,c);
													Y(a,d,a,b,c,e);
													Y(a,c,e,d,b,a);
													Y(a,c,e,d,a,b);
													Y(a,c,e,b,d,a);
													Y(a,c,e,b,a,d);
													Y(a,c,e,a,d,b);
													Y(a,c,e,a,b,d);
													Y(a,c,d,e,b,a);
													Y(a,c,d,e,a,b);
													Y(a,c,d,b,e,a);
													Y(a,c,d,b,a,e);
													Y(a,c,d,a,e,b);
													Y(a,c,d,a,b,e);
													Y(a,c,b,e,d,a);
													Y(a,c,b,e,a,d);
													Y(a,c,b,d,e,a);
													Y(a,c,b,d,a,e);
													Y(a,c,b,a,e,d);
													Y(a,c,b,a,d,e);
													Y(a,c,a,e,d,b);
													Y(a,c,a,e,b,d);
													Y(a,c,a,d,e,b);
													Y(a,c,a,d,b,e);
													Y(a,c,a,b,e,d);
													Y(a,c,a,b,d,e);
													Y(a,b,e,d,c,a);
													Y(a,b,e,d,a,c);
													Y(a,b,e,c,d,a);
													Y(a,b,e,c,a,d);
													Y(a,b,e,a,d,c);
													Y(a,b,e,a,c,d);
													Y(a,b,d,e,c,a);
													Y(a,b,d,e,a,c);
													Y(a,b,d,c,e,a);
													Y(a,b,d,c,a,e);
													Y(a,b,d,a,e,c);
													Y(a,b,d,a,c,e);
													Y(a,b,c,e,d,a);
													Y(a,b,c,e,a,d);
													Y(a,b,c,d,e,a);
													Y(a,b,c,d,a,e);
													Y(a,b,c,a,e,d);
													Y(a,b,c,a,d,e);
													Y(a,b,a,e,d,c);
													Y(a,b,a,e,c,d);
													Y(a,b,a,d,e,c);
													Y(a,b,a,d,c,e);
													Y(a,b,a,c,e,d);
													Y(a,b,a,c,d,e);
													Y(a,a,e,d,c,b);
													Y(a,a,e,d,b,c);
													Y(a,a,e,c,d,b);
													Y(a,a,e,c,b,d);
													Y(a,a,e,b,d,c);
													Y(a,a,e,b,c,d);
													Y(a,a,d,e,c,b);
													Y(a,a,d,e,b,c);
													Y(a,a,d,c,e,b);
													Y(a,a,d,c,b,e);
													Y(a,a,d,b,e,c);
													Y(a,a,d,b,c,e);
													Y(a,a,c,e,d,b);
													Y(a,a,c,e,b,d);
													Y(a,a,c,d,e,b);
													Y(a,a,c,d,b,e);
													Y(a,a,c,b,e,d);
													Y(a,a,c,b,d,e);
													Y(a,a,b,e,d,c);
													Y(a,a,b,e,c,d);
													Y(a,a,b,d,e,c);
													Y(a,a,b,d,c,e);
													Y(a,a,b,c,e,d);
													Y(a,a,b,c,d,e);

												} else {

													Y(d,d,c,b,a,a);
													Y(d,d,c,a,b,a);
													Y(d,d,c,a,a,b);
													Y(d,d,b,c,a,a);
													Y(d,d,b,a,c,a);
													Y(d,d,b,a,a,c);
													Y(d,d,a,c,b,a);
													Y(d,d,a,c,a,b);
													Y(d,d,a,b,c,a);
													Y(d,d,a,b,a,c);
													Y(d,d,a,a,c,b);
													Y(d,d,a,a,b,c);
													Y(d,c,d,b,a,a);
													Y(d,c,d,a,b,a);
													Y(d,c,d,a,a,b);
													Y(d,c,b,d,a,a);
													Y(d,c,b,a,d,a);
													Y(d,c,b,a,a,d);
													Y(d,c,a,d,b,a);
													Y(d,c,a,d,a,b);
													Y(d,c,a,b,d,a);
													Y(d,c,a,b,a,d);
													Y(d,c,a,a,d,b);
													Y(d,c,a,a,b,d);
													Y(d,b,d,c,a,a);
													Y(d,b,d,a,c,a);
													Y(d,b,d,a,a,c);
													Y(d,b,c,d,a,a);
													Y(d,b,c,a,d,a);
													Y(d,b,c,a,a,d);
													Y(d,b,a,d,c,a);
													Y(d,b,a,d,a,c);
													Y(d,b,a,c,d,a);
													Y(d,b,a,c,a,d);
													Y(d,b,a,a,d,c);
													Y(d,b,a,a,c,d);
													Y(d,a,d,c,b,a);
													Y(d,a,d,c,a,b);
													Y(d,a,d,b,c,a);
													Y(d,a,d,b,a,c);
													Y(d,a,d,a,c,b);
													Y(d,a,d,a,b,c);
													Y(d,a,c,d,b,a);
													Y(d,a,c,d,a,b);
													Y(d,a,c,b,d,a);
													Y(d,a,c,b,a,d);
													Y(d,a,c,a,d,b);
													Y(d,a,c,a,b,d);
													Y(d,a,b,d,c,a);
													Y(d,a,b,d,a,c);
													Y(d,a,b,c,d,a);
													Y(d,a,b,c,a,d);
													Y(d,a,b,a,d,c);
													Y(d,a,b,a,c,d);
													Y(d,a,a,d,c,b);
													Y(d,a,a,d,b,c);
													Y(d,a,a,c,d,b);
													Y(d,a,a,c,b,d);
													Y(d,a,a,b,d,c);
													Y(d,a,a,b,c,d);
													Y(c,d,d,b,a,a);
													Y(c,d,d,a,b,a);
													Y(c,d,d,a,a,b);
													Y(c,d,b,d,a,a);
													Y(c,d,b,a,d,a);
													Y(c,d,b,a,a,d);
													Y(c,d,a,d,b,a);
													Y(c,d,a,d,a,b);
													Y(c,d,a,b,d,a);
													Y(c,d,a,b,a,d);
													Y(c,d,a,a,d,b);
													Y(c,d,a,a,b,d);
													Y(c,b,d,d,a,a);
													Y(c,b,d,a,d,a);
													Y(c,b,d,a,a,d);
													Y(c,b,a,d,d,a);
													Y(c,b,a,d,a,d);
													Y(c,b,a,a,d,d);
													Y(c,a,d,d,b,a);
													Y(c,a,d,d,a,b);
													Y(c,a,d,b,d,a);
													Y(c,a,d,b,a,d);
													Y(c,a,d,a,d,b);
													Y(c,a,d,a,b,d);
													Y(c,a,b,d,d,a);
													Y(c,a,b,d,a,d);
													Y(c,a,b,a,d,d);
													Y(c,a,a,d,d,b);
													Y(c,a,a,d,b,d);
													Y(c,a,a,b,d,d);
													Y(b,d,d,c,a,a);
													Y(b,d,d,a,c,a);
													Y(b,d,d,a,a,c);
													Y(b,d,c,d,a,a);
													Y(b,d,c,a,d,a);
													Y(b,d,c,a,a,d);
													Y(b,d,a,d,c,a);
													Y(b,d,a,d,a,c);
													Y(b,d,a,c,d,a);
													Y(b,d,a,c,a,d);
													Y(b,d,a,a,d,c);
													Y(b,d,a,a,c,d);
													Y(b,c,d,d,a,a);
													Y(b,c,d,a,d,a);
													Y(b,c,d,a,a,d);
													Y(b,c,a,d,d,a);
													Y(b,c,a,d,a,d);
													Y(b,c,a,a,d,d);
													Y(b,a,d,d,c,a);
													Y(b,a,d,d,a,c);
													Y(b,a,d,c,d,a);
													Y(b,a,d,c,a,d);
													Y(b,a,d,a,d,c);
													Y(b,a,d,a,c,d);
													Y(b,a,c,d,d,a);
													Y(b,a,c,d,a,d);
													Y(b,a,c,a,d,d);
													Y(b,a,a,d,d,c);
													Y(b,a,a,d,c,d);
													Y(b,a,a,c,d,d);
													Y(a,d,d,c,b,a);
													Y(a,d,d,c,a,b);
													Y(a,d,d,b,c,a);
													Y(a,d,d,b,a,c);
													Y(a,d,d,a,c,b);
													Y(a,d,d,a,b,c);
													Y(a,d,c,d,b,a);
													Y(a,d,c,d,a,b);
													Y(a,d,c,b,d,a);
													Y(a,d,c,b,a,d);
													Y(a,d,c,a,d,b);
													Y(a,d,c,a,b,d);
													Y(a,d,b,d,c,a);
													Y(a,d,b,d,a,c);
													Y(a,d,b,c,d,a);
													Y(a,d,b,c,a,d);
													Y(a,d,b,a,d,c);
													Y(a,d,b,a,c,d);
													Y(a,d,a,d,c,b);
													Y(a,d,a,d,b,c);
													Y(a,d,a,c,d,b);
													Y(a,d,a,c,b,d);
													Y(a,d,a,b,d,c);
													Y(a,d,a,b,c,d);
													Y(a,c,d,d,b,a);
													Y(a,c,d,d,a,b);
													Y(a,c,d,b,d,a);
													Y(a,c,d,b,a,d);
													Y(a,c,d,a,d,b);
													Y(a,c,d,a,b,d);
													Y(a,c,b,d,d,a);
													Y(a,c,b,d,a,d);
													Y(a,c,b,a,d,d);
													Y(a,c,a,d,d,b);
													Y(a,c,a,d,b,d);
													Y(a,c,a,b,d,d);
													Y(a,b,d,d,c,a);
													Y(a,b,d,d,a,c);
													Y(a,b,d,c,d,a);
													Y(a,b,d,c,a,d);
													Y(a,b,d,a,d,c);
													Y(a,b,d,a,c,d);
													Y(a,b,c,d,d,a);
													Y(a,b,c,d,a,d);
													Y(a,b,c,a,d,d);
													Y(a,b,a,d,d,c);
													Y(a,b,a,d,c,d);
													Y(a,b,a,c,d,d);
													Y(a,a,d,d,c,b);
													Y(a,a,d,d,b,c);
													Y(a,a,d,c,d,b);
													Y(a,a,d,c,b,d);
													Y(a,a,d,b,d,c);
													Y(a,a,d,b,c,d);
													Y(a,a,c,d,d,b);
													Y(a,a,c,d,b,d);
													Y(a,a,c,b,d,d);
													Y(a,a,b,d,d,c);
													Y(a,a,b,d,c,d);
													Y(a,a,b,c,d,d);

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
												SUMCHECK;
												auto ep = dp * (e + 2);
												if (d != e) {
													DECLARE_POSITION_VALUES(e);
													Y(e,d,d,d,d,f);
													Y(e,d,d,d,f,d);
													Y(e,d,d,f,d,d);
													Y(e,d,f,d,d,d);
													Y(e,f,d,d,d,d);
													Y(d,e,d,d,d,f);
													Y(d,e,d,d,f,d);
													Y(d,e,d,f,d,d);
													Y(d,e,f,d,d,d);
													Y(d,d,e,d,d,f);
													Y(d,d,e,d,f,d);
													Y(d,d,e,f,d,d);
													Y(d,d,d,e,d,f);
													Y(d,d,d,e,f,d);
													Y(d,d,d,d,e,f);
													Y(d,d,d,d,f,e);
													Y(d,d,d,f,e,d);
													Y(d,d,d,f,d,e);
													Y(d,d,f,e,d,d);
													Y(d,d,f,d,e,d);
													Y(d,d,f,d,d,e);
													Y(d,f,e,d,d,d);
													Y(d,f,d,e,d,d);
													Y(d,f,d,d,e,d);
													Y(d,f,d,d,d,e);
													Y(f,e,d,d,d,d);
													Y(f,d,e,d,d,d);
													Y(f,d,d,e,d,d);
													Y(f,d,d,d,e,d);
													Y(f,d,d,d,d,e);
												} else {
													Y(d,d,d,d,d,f);
													Y(d,d,d,d,f,d);
													Y(d,d,d,f,d,d);
													Y(d,d,f,d,d,d);
													Y(d,f,d,d,d,d);
													Y(f,d,d,d,d,d);
												}
											}
										} else {
											// c != d
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												auto ep = dp * (e + 2);
												if (d != e) {
													Y(e,d,c,c,c,f);
													Y(e,d,c,c,f,c);
													Y(e,d,c,f,c,c);
													Y(e,d,f,c,c,c);
													Y(e,c,d,c,c,f);
													Y(e,c,d,c,f,c);
													Y(e,c,d,f,c,c);
													Y(e,c,c,d,c,f);
													Y(e,c,c,d,f,c);
													Y(e,c,c,c,d,f);
													Y(e,c,c,c,f,d);
													Y(e,c,c,f,d,c);
													Y(e,c,c,f,c,d);
													Y(e,c,f,d,c,c);
													Y(e,c,f,c,d,c);
													Y(e,c,f,c,c,d);
													Y(e,f,d,c,c,c);
													Y(e,f,c,d,c,c);
													Y(e,f,c,c,d,c);
													Y(e,f,c,c,c,d);
													Y(d,e,c,c,c,f);
													Y(d,e,c,c,f,c);
													Y(d,e,c,f,c,c);
													Y(d,e,f,c,c,c);
													Y(d,c,e,c,c,f);
													Y(d,c,e,c,f,c);
													Y(d,c,e,f,c,c);
													Y(d,c,c,e,c,f);
													Y(d,c,c,e,f,c);
													Y(d,c,c,c,e,f);
													Y(d,c,c,c,f,e);
													Y(d,c,c,f,e,c);
													Y(d,c,c,f,c,e);
													Y(d,c,f,e,c,c);
													Y(d,c,f,c,e,c);
													Y(d,c,f,c,c,e);
													Y(d,f,e,c,c,c);
													Y(d,f,c,e,c,c);
													Y(d,f,c,c,e,c);
													Y(d,f,c,c,c,e);
													Y(c,e,d,c,c,f);
													Y(c,e,d,c,f,c);
													Y(c,e,d,f,c,c);
													Y(c,e,c,d,c,f);
													Y(c,e,c,d,f,c);
													Y(c,e,c,c,d,f);
													Y(c,e,c,c,f,d);
													Y(c,e,c,f,d,c);
													Y(c,e,c,f,c,d);
													Y(c,e,f,d,c,c);
													Y(c,e,f,c,d,c);
													Y(c,e,f,c,c,d);
													Y(c,d,e,c,c,f);
													Y(c,d,e,c,f,c);
													Y(c,d,e,f,c,c);
													Y(c,d,c,e,c,f);
													Y(c,d,c,e,f,c);
													Y(c,d,c,c,e,f);
													Y(c,d,c,c,f,e);
													Y(c,d,c,f,e,c);
													Y(c,d,c,f,c,e);
													Y(c,d,f,e,c,c);
													Y(c,d,f,c,e,c);
													Y(c,d,f,c,c,e);
													Y(c,c,e,d,c,f);
													Y(c,c,e,d,f,c);
													Y(c,c,e,c,d,f);
													Y(c,c,e,c,f,d);
													Y(c,c,e,f,d,c);
													Y(c,c,e,f,c,d);
													Y(c,c,d,e,c,f);
													Y(c,c,d,e,f,c);
													Y(c,c,d,c,e,f);
													Y(c,c,d,c,f,e);
													Y(c,c,d,f,e,c);
													Y(c,c,d,f,c,e);
													Y(c,c,c,e,d,f);
													Y(c,c,c,e,f,d);
													Y(c,c,c,d,e,f);
													Y(c,c,c,d,f,e);
													Y(c,c,c,f,e,d);
													Y(c,c,c,f,d,e);
													Y(c,c,f,e,d,c);
													Y(c,c,f,e,c,d);
													Y(c,c,f,d,e,c);
													Y(c,c,f,d,c,e);
													Y(c,c,f,c,e,d);
													Y(c,c,f,c,d,e);
													Y(c,f,e,d,c,c);
													Y(c,f,e,c,d,c);
													Y(c,f,e,c,c,d);
													Y(c,f,d,e,c,c);
													Y(c,f,d,c,e,c);
													Y(c,f,d,c,c,e);
													Y(c,f,c,e,d,c);
													Y(c,f,c,e,c,d);
													Y(c,f,c,d,e,c);
													Y(c,f,c,d,c,e);
													Y(c,f,c,c,e,d);
													Y(c,f,c,c,d,e);
													Y(f,e,d,c,c,c);
													Y(f,e,c,d,c,c);
													Y(f,e,c,c,d,c);
													Y(f,e,c,c,c,d);
													Y(f,d,e,c,c,c);
													Y(f,d,c,e,c,c);
													Y(f,d,c,c,e,c);
													Y(f,d,c,c,c,e);
													Y(f,c,e,d,c,c);
													Y(f,c,e,c,d,c);
													Y(f,c,e,c,c,d);
													Y(f,c,d,e,c,c);
													Y(f,c,d,c,e,c);
													Y(f,c,d,c,c,e);
													Y(f,c,c,e,d,c);
													Y(f,c,c,e,c,d);
													Y(f,c,c,d,e,c);
													Y(f,c,c,d,c,e);
													Y(f,c,c,c,e,d);
													Y(f,c,c,c,d,e);

												} else {
													Y(d,d,c,c,c,f);
													Y(d,d,c,c,f,c);
													Y(d,d,c,f,c,c);
													Y(d,d,f,c,c,c);
													Y(d,c,d,c,c,f);
													Y(d,c,d,c,f,c);
													Y(d,c,d,f,c,c);
													Y(d,c,c,d,c,f);
													Y(d,c,c,d,f,c);
													Y(d,c,c,c,d,f);
													Y(d,c,c,c,f,d);
													Y(d,c,c,f,d,c);
													Y(d,c,c,f,c,d);
													Y(d,c,f,d,c,c);
													Y(d,c,f,c,d,c);
													Y(d,c,f,c,c,d);
													Y(d,f,d,c,c,c);
													Y(d,f,c,d,c,c);
													Y(d,f,c,c,d,c);
													Y(d,f,c,c,c,d);
													Y(c,d,d,c,c,f);
													Y(c,d,d,c,f,c);
													Y(c,d,d,f,c,c);
													Y(c,d,c,d,c,f);
													Y(c,d,c,d,f,c);
													Y(c,d,c,c,d,f);
													Y(c,d,c,c,f,d);
													Y(c,d,c,f,d,c);
													Y(c,d,c,f,c,d);
													Y(c,d,f,d,c,c);
													Y(c,d,f,c,d,c);
													Y(c,d,f,c,c,d);
													Y(c,c,d,d,c,f);
													Y(c,c,d,d,f,c);
													Y(c,c,d,c,d,f);
													Y(c,c,d,c,f,d);
													Y(c,c,d,f,d,c);
													Y(c,c,d,f,c,d);
													Y(c,c,c,d,d,f);
													Y(c,c,c,d,f,d);
													Y(c,c,c,f,d,d);
													Y(c,c,f,d,d,c);
													Y(c,c,f,d,c,d);
													Y(c,c,f,c,d,d);
													Y(c,f,d,d,c,c);
													Y(c,f,d,c,d,c);
													Y(c,f,d,c,c,d);
													Y(c,f,c,d,d,c);
													Y(c,f,c,d,c,d);
													Y(c,f,c,c,d,d);
													Y(f,d,d,c,c,c);
													Y(f,d,c,d,c,c);
													Y(f,d,c,c,d,c);
													Y(f,d,c,c,c,d);
													Y(f,c,d,d,c,c);
													Y(f,c,d,c,d,c);
													Y(f,c,d,c,c,d);
													Y(f,c,c,d,d,c);
													Y(f,c,c,d,c,d);
													Y(f,c,c,c,d,d);
												}
											}
										}
									}
								} else {
									// b != c
									for (auto d = c; d < 8ul; ++d) {
										SKIP5s(d);
										auto ds = cs + d;
										auto dp = cp * (d + 2);
										DECLARE_POSITION_VALUES(d);
										if (c == d) {
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												auto ep = dp * (e + 2);
												if (d != e) {
													DECLARE_POSITION_VALUES(e);
													Y(e,d,d,b,b,f);
													Y(e,d,d,b,f,b);
													Y(e,d,d,f,b,b);
													Y(e,d,b,d,b,f);
													Y(e,d,b,d,f,b);
													Y(e,d,b,b,d,f);
													Y(e,d,b,b,f,d);
													Y(e,d,b,f,d,b);
													Y(e,d,b,f,b,d);
													Y(e,d,f,d,b,b);
													Y(e,d,f,b,d,b);
													Y(e,d,f,b,b,d);
													Y(e,b,d,d,b,f);
													Y(e,b,d,d,f,b);
													Y(e,b,d,b,d,f);
													Y(e,b,d,b,f,d);
													Y(e,b,d,f,d,b);
													Y(e,b,d,f,b,d);
													Y(e,b,b,d,d,f);
													Y(e,b,b,d,f,d);
													Y(e,b,b,f,d,d);
													Y(e,b,f,d,d,b);
													Y(e,b,f,d,b,d);
													Y(e,b,f,b,d,d);
													Y(e,f,d,d,b,b);
													Y(e,f,d,b,d,b);
													Y(e,f,d,b,b,d);
													Y(e,f,b,d,d,b);
													Y(e,f,b,d,b,d);
													Y(e,f,b,b,d,d);
													Y(d,e,d,b,b,f);
													Y(d,e,d,b,f,b);
													Y(d,e,d,f,b,b);
													Y(d,e,b,d,b,f);
													Y(d,e,b,d,f,b);
													Y(d,e,b,b,d,f);
													Y(d,e,b,b,f,d);
													Y(d,e,b,f,d,b);
													Y(d,e,b,f,b,d);
													Y(d,e,f,d,b,b);
													Y(d,e,f,b,d,b);
													Y(d,e,f,b,b,d);
													Y(d,d,e,b,b,f);
													Y(d,d,e,b,f,b);
													Y(d,d,e,f,b,b);
													Y(d,d,b,e,b,f);
													Y(d,d,b,e,f,b);
													Y(d,d,b,b,e,f);
													Y(d,d,b,b,f,e);
													Y(d,d,b,f,e,b);
													Y(d,d,b,f,b,e);
													Y(d,d,f,e,b,b);
													Y(d,d,f,b,e,b);
													Y(d,d,f,b,b,e);
													Y(d,b,e,d,b,f);
													Y(d,b,e,d,f,b);
													Y(d,b,e,b,d,f);
													Y(d,b,e,b,f,d);
													Y(d,b,e,f,d,b);
													Y(d,b,e,f,b,d);
													Y(d,b,d,e,b,f);
													Y(d,b,d,e,f,b);
													Y(d,b,d,b,e,f);
													Y(d,b,d,b,f,e);
													Y(d,b,d,f,e,b);
													Y(d,b,d,f,b,e);
													Y(d,b,b,e,d,f);
													Y(d,b,b,e,f,d);
													Y(d,b,b,d,e,f);
													Y(d,b,b,d,f,e);
													Y(d,b,b,f,e,d);
													Y(d,b,b,f,d,e);
													Y(d,b,f,e,d,b);
													Y(d,b,f,e,b,d);
													Y(d,b,f,d,e,b);
													Y(d,b,f,d,b,e);
													Y(d,b,f,b,e,d);
													Y(d,b,f,b,d,e);
													Y(d,f,e,d,b,b);
													Y(d,f,e,b,d,b);
													Y(d,f,e,b,b,d);
													Y(d,f,d,e,b,b);
													Y(d,f,d,b,e,b);
													Y(d,f,d,b,b,e);
													Y(d,f,b,e,d,b);
													Y(d,f,b,e,b,d);
													Y(d,f,b,d,e,b);
													Y(d,f,b,d,b,e);
													Y(d,f,b,b,e,d);
													Y(d,f,b,b,d,e);
													Y(b,e,d,d,b,f);
													Y(b,e,d,d,f,b);
													Y(b,e,d,b,d,f);
													Y(b,e,d,b,f,d);
													Y(b,e,d,f,d,b);
													Y(b,e,d,f,b,d);
													Y(b,e,b,d,d,f);
													Y(b,e,b,d,f,d);
													Y(b,e,b,f,d,d);
													Y(b,e,f,d,d,b);
													Y(b,e,f,d,b,d);
													Y(b,e,f,b,d,d);
													Y(b,d,e,d,b,f);
													Y(b,d,e,d,f,b);
													Y(b,d,e,b,d,f);
													Y(b,d,e,b,f,d);
													Y(b,d,e,f,d,b);
													Y(b,d,e,f,b,d);
													Y(b,d,d,e,b,f);
													Y(b,d,d,e,f,b);
													Y(b,d,d,b,e,f);
													Y(b,d,d,b,f,e);
													Y(b,d,d,f,e,b);
													Y(b,d,d,f,b,e);
													Y(b,d,b,e,d,f);
													Y(b,d,b,e,f,d);
													Y(b,d,b,d,e,f);
													Y(b,d,b,d,f,e);
													Y(b,d,b,f,e,d);
													Y(b,d,b,f,d,e);
													Y(b,d,f,e,d,b);
													Y(b,d,f,e,b,d);
													Y(b,d,f,d,e,b);
													Y(b,d,f,d,b,e);
													Y(b,d,f,b,e,d);
													Y(b,d,f,b,d,e);
													Y(b,b,e,d,d,f);
													Y(b,b,e,d,f,d);
													Y(b,b,e,f,d,d);
													Y(b,b,d,e,d,f);
													Y(b,b,d,e,f,d);
													Y(b,b,d,d,e,f);
													Y(b,b,d,d,f,e);
													Y(b,b,d,f,e,d);
													Y(b,b,d,f,d,e);
													Y(b,b,f,e,d,d);
													Y(b,b,f,d,e,d);
													Y(b,b,f,d,d,e);
													Y(b,f,e,d,d,b);
													Y(b,f,e,d,b,d);
													Y(b,f,e,b,d,d);
													Y(b,f,d,e,d,b);
													Y(b,f,d,e,b,d);
													Y(b,f,d,d,e,b);
													Y(b,f,d,d,b,e);
													Y(b,f,d,b,e,d);
													Y(b,f,d,b,d,e);
													Y(b,f,b,e,d,d);
													Y(b,f,b,d,e,d);
													Y(b,f,b,d,d,e);
													Y(f,e,d,d,b,b);
													Y(f,e,d,b,d,b);
													Y(f,e,d,b,b,d);
													Y(f,e,b,d,d,b);
													Y(f,e,b,d,b,d);
													Y(f,e,b,b,d,d);
													Y(f,d,e,d,b,b);
													Y(f,d,e,b,d,b);
													Y(f,d,e,b,b,d);
													Y(f,d,d,e,b,b);
													Y(f,d,d,b,e,b);
													Y(f,d,d,b,b,e);
													Y(f,d,b,e,d,b);
													Y(f,d,b,e,b,d);
													Y(f,d,b,d,e,b);
													Y(f,d,b,d,b,e);
													Y(f,d,b,b,e,d);
													Y(f,d,b,b,d,e);
													Y(f,b,e,d,d,b);
													Y(f,b,e,d,b,d);
													Y(f,b,e,b,d,d);
													Y(f,b,d,e,d,b);
													Y(f,b,d,e,b,d);
													Y(f,b,d,d,e,b);
													Y(f,b,d,d,b,e);
													Y(f,b,d,b,e,d);
													Y(f,b,d,b,d,e);
													Y(f,b,b,e,d,d);
													Y(f,b,b,d,e,d);
													Y(f,b,b,d,d,e);

												} else {
													Y(d,d,d,b,b,f);
													Y(d,d,d,b,f,b);
													Y(d,d,d,f,b,b);
													Y(d,d,b,d,b,f);
													Y(d,d,b,d,f,b);
													Y(d,d,b,b,d,f);
													Y(d,d,b,b,f,d);
													Y(d,d,b,f,d,b);
													Y(d,d,b,f,b,d);
													Y(d,d,f,d,b,b);
													Y(d,d,f,b,d,b);
													Y(d,d,f,b,b,d);
													Y(d,b,d,d,b,f);
													Y(d,b,d,d,f,b);
													Y(d,b,d,b,d,f);
													Y(d,b,d,b,f,d);
													Y(d,b,d,f,d,b);
													Y(d,b,d,f,b,d);
													Y(d,b,b,d,d,f);
													Y(d,b,b,d,f,d);
													Y(d,b,b,f,d,d);
													Y(d,b,f,d,d,b);
													Y(d,b,f,d,b,d);
													Y(d,b,f,b,d,d);
													Y(d,f,d,d,b,b);
													Y(d,f,d,b,d,b);
													Y(d,f,d,b,b,d);
													Y(d,f,b,d,d,b);
													Y(d,f,b,d,b,d);
													Y(d,f,b,b,d,d);
													Y(b,d,d,d,b,f);
													Y(b,d,d,d,f,b);
													Y(b,d,d,b,d,f);
													Y(b,d,d,b,f,d);
													Y(b,d,d,f,d,b);
													Y(b,d,d,f,b,d);
													Y(b,d,b,d,d,f);
													Y(b,d,b,d,f,d);
													Y(b,d,b,f,d,d);
													Y(b,d,f,d,d,b);
													Y(b,d,f,d,b,d);
													Y(b,d,f,b,d,d);
													Y(b,b,d,d,d,f);
													Y(b,b,d,d,f,d);
													Y(b,b,d,f,d,d);
													Y(b,b,f,d,d,d);
													Y(b,f,d,d,d,b);
													Y(b,f,d,d,b,d);
													Y(b,f,d,b,d,d);
													Y(b,f,b,d,d,d);
													Y(f,d,d,d,b,b);
													Y(f,d,d,b,d,b);
													Y(f,d,d,b,b,d);
													Y(f,d,b,d,d,b);
													Y(f,d,b,d,b,d);
													Y(f,d,b,b,d,d);
													Y(f,b,d,d,d,b);
													Y(f,b,d,d,b,d);
													Y(f,b,d,b,d,d);
													Y(f,b,b,d,d,d);
												}
											}
										} else {
											// c != d
											for (auto e = d; e < 8ul; ++e) {
												SKIP5s(e);
												auto es = ds + e;
												SUMCHECK;
												auto ep = dp * (e + 2);
												if (d != e) {

Y(e,d,c,b,b,f);
Y(e,d,c,b,f,b);
Y(e,d,c,f,b,b);
Y(e,d,b,c,b,f);
Y(e,d,b,c,f,b);
Y(e,d,b,b,c,f);
Y(e,d,b,b,f,c);
Y(e,d,b,f,c,b);
Y(e,d,b,f,b,c);
Y(e,d,f,c,b,b);
Y(e,d,f,b,c,b);
Y(e,d,f,b,b,c);
Y(e,c,d,b,b,f);
Y(e,c,d,b,f,b);
Y(e,c,d,f,b,b);
Y(e,c,b,d,b,f);
Y(e,c,b,d,f,b);
Y(e,c,b,b,d,f);
Y(e,c,b,b,f,d);
Y(e,c,b,f,d,b);
Y(e,c,b,f,b,d);
Y(e,c,f,d,b,b);
Y(e,c,f,b,d,b);
Y(e,c,f,b,b,d);
Y(e,b,d,c,b,f);
Y(e,b,d,c,f,b);
Y(e,b,d,b,c,f);
Y(e,b,d,b,f,c);
Y(e,b,d,f,c,b);
Y(e,b,d,f,b,c);
Y(e,b,c,d,b,f);
Y(e,b,c,d,f,b);
Y(e,b,c,b,d,f);
Y(e,b,c,b,f,d);
Y(e,b,c,f,d,b);
Y(e,b,c,f,b,d);
Y(e,b,b,d,c,f);
Y(e,b,b,d,f,c);
Y(e,b,b,c,d,f);
Y(e,b,b,c,f,d);
Y(e,b,b,f,d,c);
Y(e,b,b,f,c,d);
Y(e,b,f,d,c,b);
Y(e,b,f,d,b,c);
Y(e,b,f,c,d,b);
Y(e,b,f,c,b,d);
Y(e,b,f,b,d,c);
Y(e,b,f,b,c,d);
Y(e,f,d,c,b,b);
Y(e,f,d,b,c,b);
Y(e,f,d,b,b,c);
Y(e,f,c,d,b,b);
Y(e,f,c,b,d,b);
Y(e,f,c,b,b,d);
Y(e,f,b,d,c,b);
Y(e,f,b,d,b,c);
Y(e,f,b,c,d,b);
Y(e,f,b,c,b,d);
Y(e,f,b,b,d,c);
Y(e,f,b,b,c,d);
Y(d,e,c,b,b,f);
Y(d,e,c,b,f,b);
Y(d,e,c,f,b,b);
Y(d,e,b,c,b,f);
Y(d,e,b,c,f,b);
Y(d,e,b,b,c,f);
Y(d,e,b,b,f,c);
Y(d,e,b,f,c,b);
Y(d,e,b,f,b,c);
Y(d,e,f,c,b,b);
Y(d,e,f,b,c,b);
Y(d,e,f,b,b,c);
Y(d,c,e,b,b,f);
Y(d,c,e,b,f,b);
Y(d,c,e,f,b,b);
Y(d,c,b,e,b,f);
Y(d,c,b,e,f,b);
Y(d,c,b,b,e,f);
Y(d,c,b,b,f,e);
Y(d,c,b,f,e,b);
Y(d,c,b,f,b,e);
Y(d,c,f,e,b,b);
Y(d,c,f,b,e,b);
Y(d,c,f,b,b,e);
Y(d,b,e,c,b,f);
Y(d,b,e,c,f,b);
Y(d,b,e,b,c,f);
Y(d,b,e,b,f,c);
Y(d,b,e,f,c,b);
Y(d,b,e,f,b,c);
Y(d,b,c,e,b,f);
Y(d,b,c,e,f,b);
Y(d,b,c,b,e,f);
Y(d,b,c,b,f,e);
Y(d,b,c,f,e,b);
Y(d,b,c,f,b,e);
Y(d,b,b,e,c,f);
Y(d,b,b,e,f,c);
Y(d,b,b,c,e,f);
Y(d,b,b,c,f,e);
Y(d,b,b,f,e,c);
Y(d,b,b,f,c,e);
Y(d,b,f,e,c,b);
Y(d,b,f,e,b,c);
Y(d,b,f,c,e,b);
Y(d,b,f,c,b,e);
Y(d,b,f,b,e,c);
Y(d,b,f,b,c,e);
Y(d,f,e,c,b,b);
Y(d,f,e,b,c,b);
Y(d,f,e,b,b,c);
Y(d,f,c,e,b,b);
Y(d,f,c,b,e,b);
Y(d,f,c,b,b,e);
Y(d,f,b,e,c,b);
Y(d,f,b,e,b,c);
Y(d,f,b,c,e,b);
Y(d,f,b,c,b,e);
Y(d,f,b,b,e,c);
Y(d,f,b,b,c,e);
Y(c,e,d,b,b,f);
Y(c,e,d,b,f,b);
Y(c,e,d,f,b,b);
Y(c,e,b,d,b,f);
Y(c,e,b,d,f,b);
Y(c,e,b,b,d,f);
Y(c,e,b,b,f,d);
Y(c,e,b,f,d,b);
Y(c,e,b,f,b,d);
Y(c,e,f,d,b,b);
Y(c,e,f,b,d,b);
Y(c,e,f,b,b,d);
Y(c,d,e,b,b,f);
Y(c,d,e,b,f,b);
Y(c,d,e,f,b,b);
Y(c,d,b,e,b,f);
Y(c,d,b,e,f,b);
Y(c,d,b,b,e,f);
Y(c,d,b,b,f,e);
Y(c,d,b,f,e,b);
Y(c,d,b,f,b,e);
Y(c,d,f,e,b,b);
Y(c,d,f,b,e,b);
Y(c,d,f,b,b,e);
Y(c,b,e,d,b,f);
Y(c,b,e,d,f,b);
Y(c,b,e,b,d,f);
Y(c,b,e,b,f,d);
Y(c,b,e,f,d,b);
Y(c,b,e,f,b,d);
Y(c,b,d,e,b,f);
Y(c,b,d,e,f,b);
Y(c,b,d,b,e,f);
Y(c,b,d,b,f,e);
Y(c,b,d,f,e,b);
Y(c,b,d,f,b,e);
Y(c,b,b,e,d,f);
Y(c,b,b,e,f,d);
Y(c,b,b,d,e,f);
Y(c,b,b,d,f,e);
Y(c,b,b,f,e,d);
Y(c,b,b,f,d,e);
Y(c,b,f,e,d,b);
Y(c,b,f,e,b,d);
Y(c,b,f,d,e,b);
Y(c,b,f,d,b,e);
Y(c,b,f,b,e,d);
Y(c,b,f,b,d,e);
Y(c,f,e,d,b,b);
Y(c,f,e,b,d,b);
Y(c,f,e,b,b,d);
Y(c,f,d,e,b,b);
Y(c,f,d,b,e,b);
Y(c,f,d,b,b,e);
Y(c,f,b,e,d,b);
Y(c,f,b,e,b,d);
Y(c,f,b,d,e,b);
Y(c,f,b,d,b,e);
Y(c,f,b,b,e,d);
Y(c,f,b,b,d,e);
Y(b,e,d,c,b,f);
Y(b,e,d,c,f,b);
Y(b,e,d,b,c,f);
Y(b,e,d,b,f,c);
Y(b,e,d,f,c,b);
Y(b,e,d,f,b,c);
Y(b,e,c,d,b,f);
Y(b,e,c,d,f,b);
Y(b,e,c,b,d,f);
Y(b,e,c,b,f,d);
Y(b,e,c,f,d,b);
Y(b,e,c,f,b,d);
Y(b,e,b,d,c,f);
Y(b,e,b,d,f,c);
Y(b,e,b,c,d,f);
Y(b,e,b,c,f,d);
Y(b,e,b,f,d,c);
Y(b,e,b,f,c,d);
Y(b,e,f,d,c,b);
Y(b,e,f,d,b,c);
Y(b,e,f,c,d,b);
Y(b,e,f,c,b,d);
Y(b,e,f,b,d,c);
Y(b,e,f,b,c,d);
Y(b,d,e,c,b,f);
Y(b,d,e,c,f,b);
Y(b,d,e,b,c,f);
Y(b,d,e,b,f,c);
Y(b,d,e,f,c,b);
Y(b,d,e,f,b,c);
Y(b,d,c,e,b,f);
Y(b,d,c,e,f,b);
Y(b,d,c,b,e,f);
Y(b,d,c,b,f,e);
Y(b,d,c,f,e,b);
Y(b,d,c,f,b,e);
Y(b,d,b,e,c,f);
Y(b,d,b,e,f,c);
Y(b,d,b,c,e,f);
Y(b,d,b,c,f,e);
Y(b,d,b,f,e,c);
Y(b,d,b,f,c,e);
Y(b,d,f,e,c,b);
Y(b,d,f,e,b,c);
Y(b,d,f,c,e,b);
Y(b,d,f,c,b,e);
Y(b,d,f,b,e,c);
Y(b,d,f,b,c,e);
Y(b,c,e,d,b,f);
Y(b,c,e,d,f,b);
Y(b,c,e,b,d,f);
Y(b,c,e,b,f,d);
Y(b,c,e,f,d,b);
Y(b,c,e,f,b,d);
Y(b,c,d,e,b,f);
Y(b,c,d,e,f,b);
Y(b,c,d,b,e,f);
Y(b,c,d,b,f,e);
Y(b,c,d,f,e,b);
Y(b,c,d,f,b,e);
Y(b,c,b,e,d,f);
Y(b,c,b,e,f,d);
Y(b,c,b,d,e,f);
Y(b,c,b,d,f,e);
Y(b,c,b,f,e,d);
Y(b,c,b,f,d,e);
Y(b,c,f,e,d,b);
Y(b,c,f,e,b,d);
Y(b,c,f,d,e,b);
Y(b,c,f,d,b,e);
Y(b,c,f,b,e,d);
Y(b,c,f,b,d,e);
Y(b,b,e,d,c,f);
Y(b,b,e,d,f,c);
Y(b,b,e,c,d,f);
Y(b,b,e,c,f,d);
Y(b,b,e,f,d,c);
Y(b,b,e,f,c,d);
Y(b,b,d,e,c,f);
Y(b,b,d,e,f,c);
Y(b,b,d,c,e,f);
Y(b,b,d,c,f,e);
Y(b,b,d,f,e,c);
Y(b,b,d,f,c,e);
Y(b,b,c,e,d,f);
Y(b,b,c,e,f,d);
Y(b,b,c,d,e,f);
Y(b,b,c,d,f,e);
Y(b,b,c,f,e,d);
Y(b,b,c,f,d,e);
Y(b,b,f,e,d,c);
Y(b,b,f,e,c,d);
Y(b,b,f,d,e,c);
Y(b,b,f,d,c,e);
Y(b,b,f,c,e,d);
Y(b,b,f,c,d,e);
Y(b,f,e,d,c,b);
Y(b,f,e,d,b,c);
Y(b,f,e,c,d,b);
Y(b,f,e,c,b,d);
Y(b,f,e,b,d,c);
Y(b,f,e,b,c,d);
Y(b,f,d,e,c,b);
Y(b,f,d,e,b,c);
Y(b,f,d,c,e,b);
Y(b,f,d,c,b,e);
Y(b,f,d,b,e,c);
Y(b,f,d,b,c,e);
Y(b,f,c,e,d,b);
Y(b,f,c,e,b,d);
Y(b,f,c,d,e,b);
Y(b,f,c,d,b,e);
Y(b,f,c,b,e,d);
Y(b,f,c,b,d,e);
Y(b,f,b,e,d,c);
Y(b,f,b,e,c,d);
Y(b,f,b,d,e,c);
Y(b,f,b,d,c,e);
Y(b,f,b,c,e,d);
Y(b,f,b,c,d,e);
Y(f,e,d,c,b,b);
Y(f,e,d,b,c,b);
Y(f,e,d,b,b,c);
Y(f,e,c,d,b,b);
Y(f,e,c,b,d,b);
Y(f,e,c,b,b,d);
Y(f,e,b,d,c,b);
Y(f,e,b,d,b,c);
Y(f,e,b,c,d,b);
Y(f,e,b,c,b,d);
Y(f,e,b,b,d,c);
Y(f,e,b,b,c,d);
Y(f,d,e,c,b,b);
Y(f,d,e,b,c,b);
Y(f,d,e,b,b,c);
Y(f,d,c,e,b,b);
Y(f,d,c,b,e,b);
Y(f,d,c,b,b,e);
Y(f,d,b,e,c,b);
Y(f,d,b,e,b,c);
Y(f,d,b,c,e,b);
Y(f,d,b,c,b,e);
Y(f,d,b,b,e,c);
Y(f,d,b,b,c,e);
Y(f,c,e,d,b,b);
Y(f,c,e,b,d,b);
Y(f,c,e,b,b,d);
Y(f,c,d,e,b,b);
Y(f,c,d,b,e,b);
Y(f,c,d,b,b,e);
Y(f,c,b,e,d,b);
Y(f,c,b,e,b,d);
Y(f,c,b,d,e,b);
Y(f,c,b,d,b,e);
Y(f,c,b,b,e,d);
Y(f,c,b,b,d,e);
Y(f,b,e,d,c,b);
Y(f,b,e,d,b,c);
Y(f,b,e,c,d,b);
Y(f,b,e,c,b,d);
Y(f,b,e,b,d,c);
Y(f,b,e,b,c,d);
Y(f,b,d,e,c,b);
Y(f,b,d,e,b,c);
Y(f,b,d,c,e,b);
Y(f,b,d,c,b,e);
Y(f,b,d,b,e,c);
Y(f,b,d,b,c,e);
Y(f,b,c,e,d,b);
Y(f,b,c,e,b,d);
Y(f,b,c,d,e,b);
Y(f,b,c,d,b,e);
Y(f,b,c,b,e,d);
Y(f,b,c,b,d,e);
Y(f,b,b,e,d,c);
Y(f,b,b,e,c,d);
Y(f,b,b,d,e,c);
Y(f,b,b,d,c,e);
Y(f,b,b,c,e,d);
Y(f,b,b,c,d,e);
												} else {
Y(d,d,c,b,b,f);
Y(d,d,c,b,f,b);
Y(d,d,c,f,b,b);
Y(d,d,b,c,b,f);
Y(d,d,b,c,f,b);
Y(d,d,b,b,c,f);
Y(d,d,b,b,f,c);
Y(d,d,b,f,c,b);
Y(d,d,b,f,b,c);
Y(d,d,f,c,b,b);
Y(d,d,f,b,c,b);
Y(d,d,f,b,b,c);
Y(d,c,d,b,b,f);
Y(d,c,d,b,f,b);
Y(d,c,d,f,b,b);
Y(d,c,b,d,b,f);
Y(d,c,b,d,f,b);
Y(d,c,b,b,d,f);
Y(d,c,b,b,f,d);
Y(d,c,b,f,d,b);
Y(d,c,b,f,b,d);
Y(d,c,f,d,b,b);
Y(d,c,f,b,d,b);
Y(d,c,f,b,b,d);
Y(d,b,d,c,b,f);
Y(d,b,d,c,f,b);
Y(d,b,d,b,c,f);
Y(d,b,d,b,f,c);
Y(d,b,d,f,c,b);
Y(d,b,d,f,b,c);
Y(d,b,c,d,b,f);
Y(d,b,c,d,f,b);
Y(d,b,c,b,d,f);
Y(d,b,c,b,f,d);
Y(d,b,c,f,d,b);
Y(d,b,c,f,b,d);
Y(d,b,b,d,c,f);
Y(d,b,b,d,f,c);
Y(d,b,b,c,d,f);
Y(d,b,b,c,f,d);
Y(d,b,b,f,d,c);
Y(d,b,b,f,c,d);
Y(d,b,f,d,c,b);
Y(d,b,f,d,b,c);
Y(d,b,f,c,d,b);
Y(d,b,f,c,b,d);
Y(d,b,f,b,d,c);
Y(d,b,f,b,c,d);
Y(d,f,d,c,b,b);
Y(d,f,d,b,c,b);
Y(d,f,d,b,b,c);
Y(d,f,c,d,b,b);
Y(d,f,c,b,d,b);
Y(d,f,c,b,b,d);
Y(d,f,b,d,c,b);
Y(d,f,b,d,b,c);
Y(d,f,b,c,d,b);
Y(d,f,b,c,b,d);
Y(d,f,b,b,d,c);
Y(d,f,b,b,c,d);
Y(c,d,d,b,b,f);
Y(c,d,d,b,f,b);
Y(c,d,d,f,b,b);
Y(c,d,b,d,b,f);
Y(c,d,b,d,f,b);
Y(c,d,b,b,d,f);
Y(c,d,b,b,f,d);
Y(c,d,b,f,d,b);
Y(c,d,b,f,b,d);
Y(c,d,f,d,b,b);
Y(c,d,f,b,d,b);
Y(c,d,f,b,b,d);
Y(c,b,d,d,b,f);
Y(c,b,d,d,f,b);
Y(c,b,d,b,d,f);
Y(c,b,d,b,f,d);
Y(c,b,d,f,d,b);
Y(c,b,d,f,b,d);
Y(c,b,b,d,d,f);
Y(c,b,b,d,f,d);
Y(c,b,b,f,d,d);
Y(c,b,f,d,d,b);
Y(c,b,f,d,b,d);
Y(c,b,f,b,d,d);
Y(c,f,d,d,b,b);
Y(c,f,d,b,d,b);
Y(c,f,d,b,b,d);
Y(c,f,b,d,d,b);
Y(c,f,b,d,b,d);
Y(c,f,b,b,d,d);
Y(b,d,d,c,b,f);
Y(b,d,d,c,f,b);
Y(b,d,d,b,c,f);
Y(b,d,d,b,f,c);
Y(b,d,d,f,c,b);
Y(b,d,d,f,b,c);
Y(b,d,c,d,b,f);
Y(b,d,c,d,f,b);
Y(b,d,c,b,d,f);
Y(b,d,c,b,f,d);
Y(b,d,c,f,d,b);
Y(b,d,c,f,b,d);
Y(b,d,b,d,c,f);
Y(b,d,b,d,f,c);
Y(b,d,b,c,d,f);
Y(b,d,b,c,f,d);
Y(b,d,b,f,d,c);
Y(b,d,b,f,c,d);
Y(b,d,f,d,c,b);
Y(b,d,f,d,b,c);
Y(b,d,f,c,d,b);
Y(b,d,f,c,b,d);
Y(b,d,f,b,d,c);
Y(b,d,f,b,c,d);
Y(b,c,d,d,b,f);
Y(b,c,d,d,f,b);
Y(b,c,d,b,d,f);
Y(b,c,d,b,f,d);
Y(b,c,d,f,d,b);
Y(b,c,d,f,b,d);
Y(b,c,b,d,d,f);
Y(b,c,b,d,f,d);
Y(b,c,b,f,d,d);
Y(b,c,f,d,d,b);
Y(b,c,f,d,b,d);
Y(b,c,f,b,d,d);
Y(b,b,d,d,c,f);
Y(b,b,d,d,f,c);
Y(b,b,d,c,d,f);
Y(b,b,d,c,f,d);
Y(b,b,d,f,d,c);
Y(b,b,d,f,c,d);
Y(b,b,c,d,d,f);
Y(b,b,c,d,f,d);
Y(b,b,c,f,d,d);
Y(b,b,f,d,d,c);
Y(b,b,f,d,c,d);
Y(b,b,f,c,d,d);
Y(b,f,d,d,c,b);
Y(b,f,d,d,b,c);
Y(b,f,d,c,d,b);
Y(b,f,d,c,b,d);
Y(b,f,d,b,d,c);
Y(b,f,d,b,c,d);
Y(b,f,c,d,d,b);
Y(b,f,c,d,b,d);
Y(b,f,c,b,d,d);
Y(b,f,b,d,d,c);
Y(b,f,b,d,c,d);
Y(b,f,b,c,d,d);
Y(f,d,d,c,b,b);
Y(f,d,d,b,c,b);
Y(f,d,d,b,b,c);
Y(f,d,c,d,b,b);
Y(f,d,c,b,d,b);
Y(f,d,c,b,b,d);
Y(f,d,b,d,c,b);
Y(f,d,b,d,b,c);
Y(f,d,b,c,d,b);
Y(f,d,b,c,b,d);
Y(f,d,b,b,d,c);
Y(f,d,b,b,c,d);
Y(f,c,d,d,b,b);
Y(f,c,d,b,d,b);
Y(f,c,d,b,b,d);
Y(f,c,b,d,d,b);
Y(f,c,b,d,b,d);
Y(f,c,b,b,d,d);
Y(f,b,d,d,c,b);
Y(f,b,d,d,b,c);
Y(f,b,d,c,d,b);
Y(f,b,d,c,b,d);
Y(f,b,d,b,d,c);
Y(f,b,d,b,c,d);
Y(f,b,c,d,d,b);
Y(f,b,c,d,b,d);
Y(f,b,c,b,d,d);
Y(f,b,b,d,d,c);
Y(f,b,b,d,c,d);
Y(f,b,b,c,d,d);

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
			}
		}
#undef DECLARE_POSITION_VALUES
#undef SUMCHECK
#undef X
#undef Y
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
