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

// Perform 32-bit numeric quodigious checks
#include "qlib.h"

template<u64 length>
inline void innerBody(std::ostream& stream, u64 sum, u64 product, u64 index, u64 depth) noexcept;

template<u64 length>
inline void body(std::ostream& stream, u64 sum = 0, u64 product = 1, u64 index = 0, u64 depth = 0) noexcept {
    static_assert(length <= 19, "Can't have numbers over 19 digits on 64-bit numbers!");
    static_assert(length != 0, "Can't have length of zero!");
    constexpr auto inner = length - 1;
    constexpr auto next = fastPow10<inner>;
    auto baseProduct = product;
    sum += 2;
    product <<= 1;
    index += multiply<2>(next);
    if (length == 1 && (depth >= 10)) {
        innerBody<inner>(stream, sum, product,index, depth); // 2
        sum += 2;
        product += (baseProduct << 1);
        index += (next << 1);
        innerBody<inner>(stream, sum, product,index, depth); // 4
        sum += 2;
        product += (baseProduct << 1);
        index += (next << 1);
        innerBody<inner>(stream, sum, product,index, depth); // 6
        sum += 2;
        product += (baseProduct << 1);
        index += (next << 1);
        innerBody<inner>(stream, sum, product,index, depth); // 8
    } else {
        innerBody<inner>(stream, sum, product, index, depth); // 2
        ++sum;
        product += baseProduct;
        index += next;
        innerBody<inner>(stream, sum, product, index, depth); // 3
        ++sum;
        product += baseProduct;
        index += next;
        innerBody<inner>(stream, sum, product, index, depth); // 4
        ++sum;
        product += baseProduct;
        index += next;
        if (length == 1) {
            innerBody<inner>(stream, sum, product, index, depth); // 5
        }
        ++sum;
        product += baseProduct;
        index += next;
        innerBody<inner>(stream, sum, product, index, depth); // 6
        ++sum;
        product += baseProduct;
        index += next;
        innerBody<inner>(stream, sum, product, index, depth); // 7
        ++sum;
        product += baseProduct;
        index += next;
        innerBody<inner>(stream, sum, product, index, depth); // 8
        ++sum;
        product += baseProduct;
        index += next;
        innerBody<inner>(stream, sum, product, index, depth); // 9
        ++sum;
        product += baseProduct;
        index += next;
    }
}

template<u64 length>
inline void innerBody(std::ostream& stream, u64 sum, u64 product, u64 index, u64 depth) noexcept {
    body<length>(stream, sum, product, index, depth + 1);
}
template<>
inline void innerBody<0>(std::ostream& stream, u64 sum, u64 product, u64 index, u64 depth) noexcept {
    if (isQuodigious(index, sum, product)) {
        stream << index << std::endl;
    }
}

template<u64 index>
inline void initialBody() noexcept {
    body<index>(std::cout);
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
