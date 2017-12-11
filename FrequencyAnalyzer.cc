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
#include "FrequencyAnalyzer.h"
FrequencyTable::FrequencyTable() : num2(0), num3(0), num4(0), num6(0), num7(0), num8(0), num9(0) { }
FrequencyTable::FrequencyTable(const FrequencyTable& t) : num2(t.num2), num3(t.num3), num4(t.num4), num6(t.num6), num7(t.num7), num8(t.num8), num9(t.num9) { }
FrequencyTable::~FrequencyTable() { }
u32 FrequencyTable::getUniqueId() const noexcept {
    return (num9 * fastPow10<6>) +
           (num8 * fastPow10<5>) +
           (num7 * fastPow10<4>) +
           (num6 * fastPow10<3>) +
           (num4 * fastPow10<2>) +
           (num3 * fastPow10<1>) +
           num2;
}

void FrequencyTable::addToTable(u64 value) noexcept {
    switch(value) {
        case 2:
            ++num2;
            break;
        case 3:
            ++num3;
            break;
        case 4:
            ++num4;
            break;
        case 6:
            ++num6;
            break;
        case 7:
            ++num7;
            break;
        case 8:
            ++num8;
            break;
        case 9:
            ++num9;
            break;
        default:
            throw "Illegal value provided!"; // this is intentional and should call terminate
    }
}
template<byte width>
inline u64 computeSumPart(byte times) noexcept {
    switch(times) {
        case 0:
            return 0;
        case 1:
            return width;
        case 2:
            return width << 1;
        case 3:
            return (width << 1) + width;
        case 4:
            return width << 2;
        case 5:
            return (width << 2) + width;
        case 6:
            return (width << 2) + (width << 1);
        case 7:
            return (width << 2) + (width << 1) + width;
        case 8:
            return (width << 3);
    }
    auto sum = 0;
    for (byte i = 0; i < times; ++i) {
        sum += width;
    }
    return sum;
}
u64 FrequencyTable::computeSum() const noexcept {
    return computeSumPart<2>(num2) +
           computeSumPart<3>(num3) +
           computeSumPart<4>(num4) +
           computeSumPart<6>(num6) +
           computeSumPart<7>(num7) +
           computeSumPart<8>(num8) +
           computeSumPart<9>(num9);
}

template<byte width>
inline u64 computeProductPart(byte times) noexcept {
    switch(times) {
        case 0:
            return 1;
        case 1:
            return width;
        case 2:
            return width * width;
        case 3:
            return width * width * width;
        case 4:
            return width * width * width * width;
        case 5:
            return width * width * width * width * width;
    }
    auto product = 1;
    for (byte i = 0; i < times; ++i) {
        product *= width;
    }
    return product;
}
u64 FrequencyTable::computeProduct() const noexcept {
    return computeProductPart<2>(num2) *
           computeProductPart<3>(num3) *
           computeProductPart<4>(num4) *
           computeProductPart<6>(num6) *
           computeProductPart<7>(num7) *
           computeProductPart<8>(num8) *
           computeProductPart<9>(num9);
}
