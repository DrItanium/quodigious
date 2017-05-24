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
    return isEven(sum) || (sum % 3 == 0);
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
inline void loopBody(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept {
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

template<u64 inner, u64 next, u64 pos>
inline std::string innerParallelBody(u64 sum, u64 product, u64 index) noexcept {
    std::ostringstream storage;
    loopBody<inner, true>(storage, sum + pos, multiply<pos>(product), index + (multiply<pos>(next)));
    std::string out(storage.str());
    return out;
}
template<u64 length>
inline void tripleSplit(std::ostream& stream, u64 sum, u64 product, u64 index) noexcept {
    // cut the computation in half...mostly
    auto b2 = std::async(std::launch::async, innerParallelBody<(length - 1), fastPow10<(length - 1)>, 2>, sum, product, index);
    auto b3 = std::async(std::launch::async, [](auto sum, auto product, auto index) {
            std::ostringstream storage;
            storage << innerParallelBody<(length - 1), fastPow10<(length - 1)>, 3>(sum, product, index);
            storage << innerParallelBody<(length - 1), fastPow10<(length - 1)>, 4>(sum, product, index);
            std::string out(storage.str());
            return out;
            }, sum, product, index);
    auto b4 = std::async(std::launch::async, [](auto sum, auto product, auto index) {
            std::ostringstream storage;
            storage << innerParallelBody<(length - 1), fastPow10<(length - 1)>, 6>(sum, product, index);
            storage << innerParallelBody<(length - 1), fastPow10<(length - 1)>, 7>(sum, product, index);
            std::string out(storage.str());
            return out;
            }, sum, product, index);
    auto b5 = std::async(std::launch::async, [](auto sum, auto product, auto index) {
            std::ostringstream storage;
            storage << innerParallelBody<(length - 1), fastPow10<(length - 1)>, 8>(sum, product, index);
            storage << innerParallelBody<(length - 1), fastPow10<(length - 1)>, 9>(sum, product, index);
            std::string out(storage.str());
            return out;
            }, sum, product, index);

    stream << b2.get() << b3.get() << b4.get() << b5.get();
}
#define performThreadSplitAtLevel(q) \
    template <> \
    inline void loopBody < q , true > (std::ostream& stream, u64 sum, u64 product, u64 index) noexcept { \
        tripleSplit< q > (stream, sum, product, index); \
    }
performThreadSplitAtLevel(10);
performThreadSplitAtLevel(11);
performThreadSplitAtLevel(12);
performThreadSplitAtLevel(13);
performThreadSplitAtLevel(14);
performThreadSplitAtLevel(16);
#undef performThreadSplitAtLevel

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
    std::ostringstream storage;
    loopBody<length - 1, skipFives>(storage, pos, pos, multiply<pos>(fastPow10<length - 1>));
    out = storage.str();
    return out;
}

template<u64 length, bool skipFives>
inline void loopBody(std::ostream& storage) noexcept {
    if (length > 7) {
        //auto b2 = std::async(std::launch::async, parallelBody<length, skipFives, 2>);
        auto b3 = std::async(std::launch::async, parallelBody<length, skipFives, 3>);
        auto b4 = std::async(std::launch::async, parallelBody<length, skipFives, 4>);
        auto b5 = std::async(std::launch::async, parallelBody<length, skipFives, 5>);
        auto b6 = std::async(std::launch::async, parallelBody<length, skipFives, 6>);
        auto b7 = std::async(std::launch::async, parallelBody<length, skipFives, 7>);
        auto b8 = std::async(std::launch::async, parallelBody<length, skipFives, 8>);
        auto b9 = std::async(std::launch::async, parallelBody<length, skipFives, 9>);
        //storage << b2.get();
        storage << parallelBody<length, skipFives, 2>();
        storage << b3.get() << b4.get() << b5.get() << b6.get();
        storage << b7.get() << b8.get() << b9.get();
    } else {
        loopBody<length, skipFives>(storage, 0, 1, 0);
    }
}

template<> inline void loopBody<1, false>(std::ostream& storage) noexcept { loopBody<1, false>(storage, 0, 1, 0); }

template<u64 length>
inline void body(std::ostream& storage) noexcept {
    static_assert(length <= 19, "Can't have numbers over 19 digits at this time!");
    // this is not going to change ever!
    loopBody<length, (length > 4)>(storage);
}

int main() {
    std::ostringstream storage;
    while(std::cin.good()) {
        u64 currentIndex = 0;
        std::cin >> currentIndex;
        if (std::cin.good()) {
            switch(currentIndex) {
                case 1: body<1>(storage); break;
                case 2: body<2>(storage); break;
                case 3: body<3>(storage); break;
                case 4: body<4>(storage); break;
                case 5: body<5>(storage); break;
                case 6: body<6>(storage); break;
                case 7: body<7>(storage); break;
                case 8: body<8>(storage); break;
                case 9: body<9>(storage); break;
                case 10: body<10>(storage); break;
                case 11: body<11>(storage); break;
                case 12: body<12>(storage); break;
                case 13: body<13>(storage); break;
                case 14: body<14>(storage); break;
                case 15: body<15>(storage); break;
                case 16: body<16>(storage); break;
                case 17: body<17>(storage); break;
                case 18: body<18>(storage); break;
                case 19: body<19>(storage); break;
                default:
                         std::cerr << "Illegal index " << currentIndex << std::endl;
                         return 1;
            }
            std::cout << storage.str() << std::endl;
            storage.str("");
        }
    }
    return 0;
}
