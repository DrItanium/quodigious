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
constexpr u64 getShiftedValue(u64 value) noexcept {
    return value << shiftAmount<position>;
}
template<u64 position>
constexpr u64 convertNumber(u64 value) noexcept {
    static_assert(position > 0, "Can't access position 0!");
    if constexpr (position == 1) {
        return ((value & 0b111) + 2);
    } else {
        constexpr auto nextPos = position - 1;
        constexpr auto mask = getShiftedValue<nextPos>(0b111ul);
        auto significand = (value & mask) >> shiftAmount<nextPos>;
        return [significand]() -> u64 {
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
template<u64 len>
constexpr auto shouldSkip5Digit(u64 x) noexcept {
    if constexpr (len > 4) {
        return x == 3ul;
    } else {
        return false;
    }
}


#define SKIP5s(x) \
    if (shouldSkip5Digit<length>(x)) { \
        ++x; \
    }
void tryInsertIntoList(u64 value, std::list<u64>& l) noexcept {
    if constexpr (debugEnabled()) {
        if (auto it = std::find(l.begin(), l.end(), value); it != l.end()) {
            std::cout << "Duplicate value: " << value << std::endl;
        } else {
            l.emplace_back(value);
        }
    } else {
        l.emplace_back(value);
    }
}
constexpr bool isNotDivisibleByThree(u64 value) noexcept {
    return (value % 3) != 0;
}
constexpr bool isDivisibleByThree(u64 value) noexcept {
    return (value % 3) == 0;
}
constexpr u64 computePartialProduct(u64 a, u64 b) noexcept {
    return a * (b + 2);
}
constexpr bool divisibleByProductAndSum(u64 value, u64 product, u64 sum) noexcept {
    return (value % product == 0) && (value % sum == 0);
}

constexpr bool disableUnpackingOptimization() noexcept {
#ifdef DISABLE_UNPACKED5
    return true;
#else
    return false;
#endif
}

using DataTriple = std::tuple<u64, u64, u64>;
template<u64 position, u64 length>
void body(MatchList& list, const DataTriple& contents) noexcept;
template<u64 position, u64 length>
void body(MatchList& list, u64 sum = 0, u64 product = 1, u64 index = 0) noexcept {
    static_assert(length <= 19, "Can't have numbers over 19 digits on 64-bit numbers!");
    static_assert(length > 0, "Can't have length of zero!");
    static_assert(length >= position, "Position is out of bounds!");
    static constexpr auto indexIncr = getShiftedValue<position>(1ul);
    static constexpr auto lenGreaterAndPos = [](u64 len, u64 pos) noexcept {
        return (length > len) && (position == pos);
    };
    auto fn = [&list](auto n, auto ep, auto es) noexcept {
        if (divisibleByProductAndSum(n, ep, es)) {
            list.emplace_back(n); 
        }
    };
    if constexpr (position == length) {
        if constexpr (length > 10) {
            // if the number is not divisible by three then skip it
            if (isNotDivisibleByThree(sum)) {
                return;
            }
        }
        fn(convertNumber<length>(index), product, sum);
    } else if constexpr (lenGreaterAndPos(10, 2) || 
            lenGreaterAndPos(11, 3) || 
            lenGreaterAndPos(12, 4) || 
            lenGreaterAndPos(13, 5) || 
            lenGreaterAndPos(14, 6)) {
            //lenGreaterAndPos<14, 6, length, position>) {
        // setup a series of operations to execute in parallel on two separate threads
        // of execution
        std::list<DataTriple> lower, upper;
        auto dprod = product << 1;
        lower.emplace_back(sum + 0, dprod + (0 * product), index + (0 * indexIncr));
        lower.emplace_back(sum + 1, dprod + (1 * product), index + (1 * indexIncr));
        lower.emplace_back(sum + 2, dprod + (2 * product), index + (2 * indexIncr));
        upper.emplace_back(sum + 4, dprod + (4 * product), index + (4 * indexIncr));
        upper.emplace_back(sum + 5, dprod + (5 * product), index + (5 * indexIncr));
        upper.emplace_back(sum + 6, dprod + (6 * product), index + (6 * indexIncr));
        upper.emplace_back(sum + 7, dprod + (7 * product), index + (7 * indexIncr));
        auto halveIt = [](const std::list<DataTriple> & collection) {
            MatchList l;
            for(const auto& a : collection) {
                body<position + 1, length>(l, a);
            }
            return l;
        };
        auto t0 = std::async(std::launch::async, halveIt, std::cref(lower)),
             t1 = std::async(std::launch::async, halveIt, std::cref(upper));
        auto l0 = t0.get(),
             l1 = t1.get();
        list.splice(list.cbegin(), l0);
        list.splice(list.cbegin(), l1);
    } else if constexpr (length > 10 && ((length - position) == 5) && !disableUnpackingOptimization()) {
        static constexpr auto buildTuple = [](u64 val) {
            return std::make_tuple<u64, u64, u64, u64, u64>(val * fastPow10<position>, 
                                                            val * fastPow10<position+1>,
                                                            val * fastPow10<position+2>,
                                                            val * fastPow10<position+3>,
                                                            val * fastPow10<position+4>);
        };
        static constexpr std::tuple<u64, u64, u64, u64, u64> p10s[] = {
            buildTuple(0),
            buildTuple(1),
            buildTuple(2),
            buildTuple(3),
            buildTuple(4),
            buildTuple(5),
            buildTuple(6),
            buildTuple(7),
        };

        // this will generate a partial number but reduce the number of conversions
        // required greatly!
        // The last two digits are handled in a base 10 fashion without the +2 added
        // This will make the partial converison correct (remember that a 0 becomes a 2
        // in this model).
        //
        // Thus we implicitly add the offsets for each position to this base10 2 value :D

        auto outerConverted = convertNumber<length>(index);
        static constexpr auto computePositionValues = [](u64 var) noexcept { return p10s[var]; };
        static constexpr auto computeSumProduct = [](auto var, auto sum, auto product) noexcept {
            return std::make_tuple(var + sum, computePartialProduct(product, var)); 
        };

#define X(x,y,z,w,h) fn(outerConverted + x ## 1 + y ## 2 + z ## 3 + w ## 4 + h ## 5, ep, es)
#define DECLARE_POSITION_VALUES(var) \
        auto [var ## 1, var ## 2, var ## 3, var ## 4, var ## 5] = computePositionValues(var)
        for (auto a = 0ul; a < 8ul; ++a) {
            SKIP5s(a);
            DECLARE_POSITION_VALUES(a);
            auto [as, ap] = computeSumProduct(a, sum, product);
            for (auto b = a; b < 8ul; ++b) {
                SKIP5s(b);
                auto [bs, bp] = computeSumProduct(b, as, ap);
                // use transitivity to reduce the amount of recomputation. 
                // if a == b then it means that a and b can be used interchangably
                // in the final computation so there is no need to actually perform
                // separate computation with a and b. We can just use b (or a) in
                // all cases where a and b need to be used. This allows us to 
                // eliminate redundant cases. Thus speeding computation up quite
                // a bit.
                if (DECLARE_POSITION_VALUES(b); a == b) {
                    for (auto c = b; c < 8ul; ++c) {
                        SKIP5s(c);
                        auto [cs, cp] = computeSumProduct(c, bs, bp);
                        if (DECLARE_POSITION_VALUES(c); b == c) {
                            // a == b and b == c, => a == c. Thus a, b, and c 
                            // can be used interchangeably. Thus the number of 
                            // unique computations required is reduced even further
                            // down this path
                            for (auto d = c; d < 8ul; ++d) {
                                SKIP5s(d);
                                auto [ds, dp] = computeSumProduct(d, cs, cp);
                                if (DECLARE_POSITION_VALUES(d); c == d) {
                                    // a == b and b == c and c == d => a == c 
                                    // and a == d and b == d. Further reducing the
                                    // number of required computations
                                    //
                                    // NOTE: This is the edge case where the number is
                                    // like 4444444443, 999999999, 9999999998, etc.
                                    for (auto e = d; e < 8ul; ++e) {
                                        SKIP5s(e);
                                        if (auto es = ds + e; isDivisibleByThree(es)) {
                                            auto ep = computePartialProduct(dp, e);
                                            DECLARE_POSITION_VALUES(e);
                                            // in all cases we must check this computation
                                            X(d,d,d,d,e);
                                            if (d != e) {
                                                // if d != e then e is unique compared to
                                                // every other value, thus we should perform
                                                // computation with e in each position.
                                                X(e,d,d,d,d); X(d,e,d,d,d); X(d,d,e,d,d); 
                                                X(d,d,d,e,d); 
                                            }

                                        }
                                    }
                                } else {
                                    // a == b and b == c and c != d => a == c and a != d and b != d
                                    for (auto e = d; e < 8ul; ++e) {
                                        SKIP5s(e);
                                        if (auto es = ds + e; isDivisibleByThree(es)) {
                                            auto ep = computePartialProduct(dp, e);
                                            DECLARE_POSITION_VALUES(e);
                                            X(e,d,c,c,c); X(e,c,d,c,c); X(e,c,c,d,c);
                                            X(e,c,c,c,d); 
                                            X(a,b,c,d,e); X(c,e,d,c,c); X(c,e,c,d,c); 
                                            X(c,e,c,c,d); X(c,c,e,d,c); X(c,c,e,c,d); 
                                            if (d != e) {
                                                X(d,e,c,c,c); X(d,c,e,c,c); X(d,c,c,e,c); 
                                                X(d,c,c,c,e); X(c,d,e,c,c); X(c,d,c,e,c); 
                                                X(c,d,c,c,e); X(c,c,d,e,c); X(c,c,d,c,e); 
                                                X(c,c,c,e,d); 
                                            }
                                        }
                                    }
                                }
                            }
                        } else {
                            // a == b and b != c => a != c and c > b and c > a
                            // read on for more information about the use of strict inequalities
                            for (auto d = c; d < 8ul; ++d) {
                                SKIP5s(d);
                                auto [ds, dp] = computeSumProduct(d, cs, cp);
                                if (DECLARE_POSITION_VALUES(d); c == d) {
                                    // a == b and b != c and c == d and a != c => a != d and b != d
                                    for (auto e = d; e < 8ul; ++e) {
                                        SKIP5s(e);
                                        if (auto es = ds + e; isDivisibleByThree(es)) {
                                            auto ep = computePartialProduct(dp, e);
                                            DECLARE_POSITION_VALUES(e);
                                            X(e,d,c,b,b); X(e,d,b,c,b); X(e,d,b,b,c); 
                                            X(e,b,b,d,c); X(e,b,d,c,b); X(e,b,d,b,c); 
                                            X(b,e,b,d,c); X(b,e,d,c,b); X(b,e,c,b,d); 
                                            X(a,b,c,d,e); 
                                            if (d != e) {
                                                X(d,e,c,b,b); X(d,e,b,c,b); X(d,e,b,b,c);
                                                X(d,c,e,b,b); X(d,c,b,e,b); X(d,c,b,b,e);
                                                X(d,b,e,c,b); X(d,b,e,b,c); X(d,b,c,e,b);
                                                X(d,b,c,b,e); X(d,b,b,e,c); X(d,b,b,c,e);
                                                X(b,d,e,c,b); X(b,d,e,b,c); X(b,d,c,e,b);
                                                X(b,d,c,b,e); X(b,d,b,e,c); X(b,d,b,c,e);
                                                X(b,b,e,d,d); X(b,b,d,e,d); 
                                            }
                                        }
                                    }
                                } else {
                                    // a == b and b != c and c != d and a != c could
                                    // cause problems because a could equal d and thus so
                                    // could b. That is impossible in this case though
                                    // as d would only ever be >= to c. In this case since
                                    // c != d it means that d is greater than c. Thus
                                    // it means that d != a and thus d != b. This property
                                    // applies to all cases as we enter into further nested
                                    // loops. Thus we get to the logic of:
                                    //
                                    // a == b and b != c and c != d and a != c and d > c => a != d and b != d 
   
                                    for (auto e = d; e < 8ul; ++e) {
                                        SKIP5s(e);
                                        if (auto es = ds + e; isDivisibleByThree(es)) {
                                            auto ep = computePartialProduct(dp, e);
                                            DECLARE_POSITION_VALUES(e);
                                            X(e,d,c,b,b); X(e,d,b,c,b); X(e,d,b,b,c); 
                                            X(e,b,b,d,c); X(e,b,d,c,b); X(e,b,d,b,c); 
                                            X(e,c,d,b,b); X(e,c,b,d,b); X(e,c,b,b,d); 
                                            X(e,b,c,d,b); X(e,b,c,b,d); X(e,b,b,c,d);
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
                    }
                } else {
                    for (auto c = b; c < 8ul; ++c) {
                        SKIP5s(c);
                        auto [cs, cp] = computeSumProduct(c, bs, bp);
                        if (DECLARE_POSITION_VALUES(c); b == c) {
                            for (auto d = c; d < 8ul; ++d) {
                                SKIP5s(d);
                                auto [ds, dp] = computeSumProduct(d, cs, cp);
                                if (DECLARE_POSITION_VALUES(d); c == d) {
                                    for (auto e = d; e < 8ul; ++e) {
                                        SKIP5s(e);
                                        if (auto es = ds + e; isDivisibleByThree(es)) {
                                            auto ep = computePartialProduct(dp, e);
                                            DECLARE_POSITION_VALUES(e);
                                            X(e,d,c,c,a); X(e,d,c,a,c); X(e,d,a,c,c); 
                                            X(e,a,d,c,c); 
                                            X(a,b,c,d,e); 
                                            if (d != e) {
                                                X(d,e,c,c,a); X(d,e,c,a,c); X(d,e,a,c,c);
                                                X(d,c,e,c,a); X(d,c,e,a,c); X(d,c,c,e,a);
                                                X(d,c,c,a,e); X(d,c,a,e,c); X(d,c,a,c,e);
                                                X(a,c,c,e,d); X(c,a,d,e,c); X(c,a,d,c,e); 
                                                X(d,a,e,d,d); X(a,e,d,d,d); X(a,d,e,d,d); 
                                            }
                                        }
                                    }
                                } else {
                                    for (auto e = d; e < 8ul; ++e) {
                                        SKIP5s(e);
                                        if (auto es = ds + e; isDivisibleByThree(es)) {
                                            auto ep = computePartialProduct(dp, e);
                                            DECLARE_POSITION_VALUES(e);
                                            X(e,a,d,c,c); X(e,d,c,c,a); X(e,d,c,a,c); 
                                            X(e,d,a,c,c); X(e,c,d,c,a); X(e,c,d,a,c); 
                                            X(e,c,c,d,a); X(e,c,c,a,d); X(e,c,a,d,c); 
                                            X(e,c,a,c,d); X(e,a,c,d,c); X(e,a,c,c,d); 
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
                            }
                        } else {
                            for (auto d = c; d < 8ul; ++d) {
                                SKIP5s(d);
                                auto [ds, dp] = computeSumProduct(d, cs, cp);
                                if (DECLARE_POSITION_VALUES(d); c == d) {
                                    for (auto e = d; e < 8ul; ++e) {
                                        SKIP5s(e);
                                        if (auto es = ds + e; isDivisibleByThree(es)) {
                                            auto ep = computePartialProduct(dp, e);
                                            DECLARE_POSITION_VALUES(e);
                                            X(e,d,d,b,a); X(e,d,d,a,b); X(e,d,b,d,a);
                                            X(e,d,b,a,d); X(e,d,a,d,b); X(e,d,a,b,d);
                                            X(e,b,d,d,a); X(e,b,d,a,d); X(e,b,a,d,d);
                                            X(e,a,d,d,b); X(e,a,d,b,d); X(e,a,b,d,d);
                                            X(a,b,c,d,e); X(a,e,d,d,b); X(a,e,d,b,d); 
                                            X(a,e,b,d,d); X(b,e,d,d,a); X(b,e,d,a,d); 
                                            X(b,e,a,d,d); X(b,a,d,d,e);

                                            if (d != e) {
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
                                    }
                                } else {
                                    for (auto e = d; e < 8ul; ++e) {
                                        SKIP5s(e);
                                        if (auto es = ds + e; isDivisibleByThree(es)) {
                                            auto ep = computePartialProduct(dp, e);
                                            DECLARE_POSITION_VALUES(e);
                                            X(e,a,b,d,c); X(e,a,c,b,d); X(e,a,c,d,b);
                                            X(e,d,c,b,a); X(e,d,c,a,b); X(e,d,b,c,a);
                                            X(e,d,a,b,c); X(e,d,a,c,b); X(e,d,b,a,c); 
                                            X(e,c,b,d,a); X(e,c,d,a,b); X(e,c,d,b,a); 
                                            X(e,c,a,b,d); X(e,c,a,d,b); X(e,c,b,a,d);
                                            X(e,a,b,c,d); X(e,a,d,b,c); X(e,a,d,c,b); 
                                            X(e,b,a,c,d); X(e,b,a,d,c); X(e,b,c,a,d); 
                                            X(e,b,c,d,a); X(e,b,d,a,c); X(e,b,d,c,a); 
                                            X(a,e,c,d,b); X(a,e,d,b,c); X(a,e,d,c,b); 
                                            X(a,e,b,c,d); X(a,e,b,d,c); X(a,e,c,b,d); 
                                            X(b,e,c,d,a); X(b,e,d,a,c); X(b,e,d,c,a); 
                                            X(b,e,a,c,d); X(b,e,a,d,c); X(b,e,c,a,d); 
                                            X(c,e,b,d,a); X(c,e,d,a,b); X(c,e,d,b,a); 
                                            X(c,e,a,b,d); X(c,e,a,d,b); X(c,e,b,a,d); 

                                            X(a,c,b,e,d); X(a,c,d,e,b); X(a,b,d,e,c);
                                            X(c,b,d,e,a); X(b,c,d,e,a); X(b,a,c,e,d); 
                                            X(b,a,d,e,c); X(c,a,d,e,b); 


                                            X(a,b,c,d,e); X(a,b,d,c,e); X(a,c,d,b,e); 
                                            X(c,b,d,a,e); X(c,b,a,d,e); X(b,c,d,a,e); 
                                            X(b,c,a,d,e); X(c,a,d,b,e); X(c,a,b,d,e); 
                                            X(b,a,d,c,e); 


                                            if (d != e) {
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
#undef DECLARE_POSITION_VALUES
#undef X
    } else {
        auto dprod = product << 1;
        body<position + 1, length>(list, sum + 0, dprod + (0 * product), index + (0 * indexIncr));
        body<position + 1, length>(list, sum + 1, dprod + (1 * product), index + (1 * indexIncr));
        body<position + 1, length>(list, sum + 2, dprod + (2 * product), index + (2 * indexIncr));
        body<position + 1, length>(list, sum + 4, dprod + (4 * product), index + (4 * indexIncr));
        body<position + 1, length>(list, sum + 5, dprod + (5 * product), index + (5 * indexIncr));
        body<position + 1, length>(list, sum + 6, dprod + (6 * product), index + (6 * indexIncr));
        body<position + 1, length>(list, sum + 7, dprod + (7 * product), index + (7 * indexIncr));
    }
}
#undef SKIP5s
template<u64 position, u64 length>
void body(MatchList& list, const DataTriple& contents) noexcept {
    auto [sum, prod, ind] = contents;
    body<position, length>(list, sum, prod, ind);
}

template<auto width>
MatchList parallelBody(u64 base) noexcept {
    MatchList list;
    auto start = (base - 2ul);
    auto index = start << 3;
    static constexpr auto addon = width << 1;
    auto startPlusAddon = start + addon;
    // using the frequency analysis I did before for loops64.cc I found
    // that on even digits that 4 and 8 are used while odd digits use 2
    // and 6. This is a frequency analysis job only :D
    for (auto i = ((base % 2ul == 0) ? 4ul : 2ul); i < 10ul; i += 4ul) {
        auto j = i - 2ul;
        body<2, width>(list, startPlusAddon + j, base * i, index + j);
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
