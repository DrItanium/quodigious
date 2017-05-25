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
#include "qlib.h"

using u32 = uint32_t;
inline u32 innerMostBody(u32 sum, u32 product, u32 value) noexcept {
    if (componentQuodigious(value, sum)) {
        if (componentQuodigious(value, product)) {
            return value;
        }
    }
    return 0;
}

inline void merge(u32 value, std::ostream& storage) noexcept {
    if (value != 0) {
        storage << value << std::endl;
    }
}


template<u32 length>
inline void loopBody(std::ostream& storage, u32 sum, u32 product, u32 index) noexcept {
        constexpr auto inner = length - 1;
        constexpr auto next = fastPow10<inner>;
        auto advance = [&sum]() noexcept { ++sum; };
        sum += 2;
        loopBody<inner>(storage, sum, multiply<2>(product), index + (multiply<2>(next)));
        advance();
        loopBody<inner>(storage, sum, multiply<3>(product), index + (multiply<3>(next)));
        advance();
        loopBody<inner>(storage, sum, multiply<4>(product), index + (multiply<4>(next)));
        advance();
        loopBody<inner>(storage, sum, multiply<5>(product), index + multiply<5>(next));
        advance();
        loopBody<inner>(storage, sum, multiply<6>(product), index + multiply<6>(next));
        advance();
        loopBody<inner>(storage, sum, multiply<7>(product), index + multiply<7>(next));
        advance();
        loopBody<inner>(storage, sum, multiply<8>(product), index + multiply<8>(next));
        advance();
        loopBody<inner>(storage, sum, multiply<9>(product), index + multiply<9>(next));
}




template<>
inline void loopBody<1>(std::ostream& storage, u32 sum, u32 product, u32 index) noexcept {
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

template<u32 length>
inline void body(std::ostream& storage) noexcept {
    static_assert(length <= 9, "Can't have numbers over 9 digits on 32-bit numbers!");
    // this is not going to change ever!
    loopBody<length>(storage, 0, 1, 0);
}

int main() {
    std::ostringstream storage;
    while(std::cin.good()) {
        u32 currentIndex = 0;
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
