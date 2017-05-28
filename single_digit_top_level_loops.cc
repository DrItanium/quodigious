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

// Perform quodigious checks on numbers using tons of different C++ tricks
#include <iostream>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <functional>
#include <future>
#include "qlib.h"

template<bool experimentalCheck = false>
inline bool checkValue(u64 sum) noexcept {
    if (!experimentalCheck) {
        return true;
    }
	return (sum % 2 == 0) || (sum % 3 == 0);
}
template<bool experimentalCheck = false>
inline u64 innerMostBody(u64 sum, u64 product, u64 value) noexcept {
    if (checkValue<experimentalCheck>(sum)) {
        if (componentQuodigious(value, sum)) {
            if (componentQuodigious(value, product)) {
                return value;
            }
        }
    }
    return 0;
}
inline void merge(u64 value, std::ostream& storage) noexcept {
    if (value != 0) {
        storage << value << std::endl;
    }
}

template<u64 length, bool skipFives>
void loopBody(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept;


template<u64 length, u64 pos>
inline void innerParallelBodyExternalStream(std::ostream& stream, u64 sum, u64 product, u64 index) noexcept {
    constexpr auto inner = (length - 1);
    constexpr auto next = fastPow10<inner>;
    loopBody<inner, true>(stream, sum + pos, multiply<pos>(product), index + (multiply<pos>(next)));
}
template<u64 length, u64 pos>
inline std::string innerParallelBody(u64 sum, u64 product, u64 index) noexcept {
    std::ostringstream storage;
    innerParallelBodyExternalStream<length, pos>(storage, sum, product, index);
    std::string out(storage.str());
    return out;
}

template<u64 length, u64 digitA, u64 digitB>
std::string dualParallelBody(u64 sum, u64 product, u64 index) noexcept {
    static_assert(digitA != digitB, "the first digit and the second should not be equal!");
    std::ostringstream storage;
    innerParallelBodyExternalStream<length, digitA>(storage, sum, product, index);
    innerParallelBodyExternalStream<length, digitB>(storage, sum, product, index);
    std::string out(storage.str());
    return out;
}

template<u64 length, u64 digitA, u64 digitB, u64 digitC>
std::string tripleParallelBody(u64 sum, u64 product, u64 index) noexcept {
    static_assert(digitA != digitB, "the first digit and the second should not be equal!");
	static_assert(digitB != digitC, "the second digit and the third digit should not be equal!");
	static_assert(digitC != digitA, "the third digit and first digit should not be equal!");
    std::ostringstream storage;
    innerParallelBodyExternalStream<length, digitA>(storage, sum, product, index);
    innerParallelBodyExternalStream<length, digitB>(storage, sum, product, index);
	innerParallelBodyExternalStream<length, digitC>(storage, sum, product, index);
    std::string out(storage.str());
    return out;
}

template<u64 length>
inline void tripleSplit(std::ostream& stream, u64 sum, u64 product, u64 index) noexcept {
	auto doAsync = [&sum, &product, &index](auto fn) {
		return std::async(std::launch::async, fn, sum, product, index);
	};
    if (length >= 12 && length <= 14) {
        auto b2 = doAsync(innerParallelBody<length, 2>);
        auto b3 = doAsync(innerParallelBody<length, 3>);
        auto b4 = doAsync(innerParallelBody<length, 4>);
        auto b6 = doAsync(innerParallelBody<length, 6>);
        auto b7 = doAsync(innerParallelBody<length, 7>);
        auto b8 = doAsync(innerParallelBody<length, 8>);
        auto b9 = doAsync(innerParallelBody<length, 9>);
		stream << b2.get();
        stream << b3.get() << b4.get() << b6.get() << b7.get() << b8.get() << b9.get();
    } else if (length > 14) {
    	auto b1 = doAsync(innerParallelBody<length, 2>);
    	auto b2 = doAsync(innerParallelBody<length, 3>);
	stream << b1.get() << b2.get();
	auto b3 = doAsync(innerParallelBody<length, 4>);
	auto b4 = doAsync(innerParallelBody<length, 6>);
	stream << b3.get() << b4.get();
	auto b5 = doAsync(innerParallelBody<length, 7>);
	auto b6 = doAsync(innerParallelBody<length, 8>);
	stream << b5.get() << b6.get();
	auto b7 = doAsync(innerParallelBody<length, 9>);
	stream << b7.get();
    } else {
        // cut the computation in thirds...mostly
	auto b2 = doAsync(innerParallelBody<length, 2>);
        auto b3 = doAsync(dualParallelBody<length, 3, 4>);
        auto b4 = doAsync(dualParallelBody<length, 6, 7>);
        auto b5 = doAsync(dualParallelBody<length, 8, 9>);
        stream << b2.get() << b3.get() << b4.get() << b5.get();
    }
}

template<u64 length>
struct SkipFives : std::integral_constant<bool, (length > 4)> { };

template<u64 length>
struct EnableParallelism : std::integral_constant<bool, false> { };
#define EnableParallelismAtLevel(q) template <> struct EnableParallelism< q > : std::integral_constant<bool, true> { }
EnableParallelismAtLevel(10);
EnableParallelismAtLevel(12);
EnableParallelismAtLevel(13);
EnableParallelismAtLevel(14);
EnableParallelismAtLevel(16);
EnableParallelismAtLevel(18);
#undef EnableParallelismAtLevel

template<u64 length, bool skipFives>
inline void loopBody(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept {
    if (EnableParallelism<length>::value) {
        tripleSplit<length>(storage, sum, product, index);
    } else {
        constexpr auto inner = length - 1;
        constexpr auto next = fastPow10<inner>;

        auto advance = [&sum]() noexcept { ++sum; };
        auto doubleAdvance = [&sum]() noexcept { sum += 2; };

        doubleAdvance();
        loopBody<inner, skipFives>(storage, sum, multiply<2>(product), index + (multiply<2>(next)));
        advance();
        loopBody<inner, skipFives>(storage, sum, multiply<3>(product), index + (multiply<3>(next)));
        advance();
        loopBody<inner, skipFives>(storage, sum, multiply<4>(product), index + (multiply<4>(next)));
        if (!skipFives) {
            advance();
            loopBody<inner, skipFives>(storage, sum, multiply<5>(product), index + multiply<5>(next));
            advance();
        } else {
            doubleAdvance();
        }
        loopBody<inner, skipFives>(storage, sum, multiply<6>(product), index + multiply<6>(next));
        advance();
        loopBody<inner, skipFives>(storage, sum, multiply<7>(product), index + multiply<7>(next));
        advance();
        loopBody<inner, skipFives>(storage, sum, multiply<8>(product), index + multiply<8>(next));
        advance();
        loopBody<inner, skipFives>(storage, sum, multiply<9>(product), index + multiply<9>(next));
    }
}




template<>
inline void loopBody<1, false>(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept {
    auto advance = [&sum, &index]() noexcept { ++sum; ++index; };
    sum += 2;
    index += 2;
    merge(innerMostBody(sum, multiply<2>(product), index), storage);
    advance();
    merge(innerMostBody(sum, multiply<3>(product), index), storage);
    advance();
    merge(innerMostBody(sum, multiply<4>(product), index), storage);
    advance();
    merge(innerMostBody(sum, multiply<5>(product), index), storage);
    advance();
    merge(innerMostBody(sum, multiply<6>(product), index), storage);
    advance();
    merge(innerMostBody(sum, multiply<7>(product), index), storage);
    advance();
    merge(innerMostBody(sum, multiply<8>(product), index), storage);
    advance();
    merge(innerMostBody(sum, multiply<9>(product), index), storage);
}

template<>
inline void loopBody<1, true>(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept {
    auto advance = [&sum, &index]() noexcept { sum += 2; index += 2; };
    advance();
    merge(innerMostBody<true>(sum, multiply<2>(product), index), storage);
    advance();
    merge(innerMostBody<true>(sum, multiply<4>(product), index), storage);
    advance();
    merge(innerMostBody<true>(sum, multiply<6>(product), index), storage);
    advance();
    merge(innerMostBody<true>(sum, multiply<8>(product), index), storage);
}

template<u64 length, bool skipFives, u64 pos>
inline std::string parallelBody() noexcept {
    std::string out;
    if (pos == 5 && skipFives) {
        return out;
    }
    constexpr auto next = (length - 1);
    std::ostringstream storage;
    loopBody<next, skipFives>(storage, pos, pos, multiply<pos>(fastPow10<next>));
    out = storage.str();
    return out;
}

template<u64 length, bool skipFives>
inline void loopBody(std::ostream& storage) noexcept {
	loopBody<length, skipFives>(storage, 0, 1, 0);
}

template<u64 length, u64 pos>
inline bool body(std::ostream& storage) noexcept {
    static_assert(length <= 19, "Can't have numbers over 19 digits at this time!");
    // this is not going to change ever!
	innerParallelBodyExternalStream<length, pos>(storage, 0, 1, 0);
	return true;
}

template<u64 index>
bool posDispatch(std::ostream& storage, u64 pos) noexcept {
	switch(pos) {
		case 2: return body<index, 2>(storage);
		case 3: return body<index, 3>(storage);
		case 4: return body<index, 4>(storage);
		case 5: return body<index, 5>(storage);
		case 6: return body<index, 6>(storage);
		case 7: return body<index, 7>(storage);
		case 8: return body<index, 8>(storage);
		case 9: return body<index, 9>(storage);
		default:
			std::cerr << "Illegal position provided" << pos << std::endl;
			return false;
	}
}

bool bodyDispatch(std::ostream& storage, u64 index, u64 pos) noexcept {
	switch(index) {
		// one is not supported in this case
		case 2: return posDispatch<2>(storage, pos); 
		case 3: return posDispatch<3>(storage, pos); 
		case 4: return posDispatch<4>(storage, pos); 
		case 5: return posDispatch<5>(storage, pos); 
		case 6: return posDispatch<6>(storage, pos); 
		case 7: return posDispatch<7>(storage, pos); 
		case 8: return posDispatch<8>(storage, pos); 
		case 9: return posDispatch<9>(storage, pos); 
		case 10: return posDispatch<10>(storage, pos); 
		case 11: return posDispatch<11>(storage, pos); 
		case 12: return posDispatch<12>(storage, pos); 
		case 13: return posDispatch<13>(storage, pos); 
		case 14: return posDispatch<14>(storage, pos); 
		case 15: return posDispatch<15>(storage, pos); 
		case 16: return posDispatch<16>(storage, pos); 
		case 17: return posDispatch<17>(storage, pos); 
		case 18: return posDispatch<18>(storage, pos); 
		case 19: return posDispatch<19>(storage, pos); 
		default:
				 std::cerr << "Illegal index " << index << std::endl;
				 return false;
	}
}

int main() {
    std::ostringstream storage;
    while(std::cin.good()) {
        u64 currentIndex = 0;
		u64 currentPosition = 0;
        std::cin >> currentIndex;
		std::cin >> currentPosition;
        if (std::cin.good()) {
			if (!bodyDispatch(storage, currentIndex, currentPosition)) {
				return 1;
			} else {
				std::cout << storage.str() << std::endl;
				storage.str("");
			}
        }
    }
    return 0;
}