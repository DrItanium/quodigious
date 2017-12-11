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

template<u64 width, u64 length>
constexpr u64 quickSum() noexcept {
    return width * length;
}
template<byte width>
inline u64 computeSumPart(byte times) noexcept {
    static constexpr u64 sums[] = {
        quickSum<width, 0>(),
        quickSum<width, 1>(),
        quickSum<width, 2>(),
        quickSum<width, 3>(),
        quickSum<width, 4>(),
        quickSum<width, 5>(),
        quickSum<width, 6>(),
        quickSum<width, 7>(),
        quickSum<width, 8>(),
        quickSum<width, 9>(),
        quickSum<width, 10>(),
        quickSum<width, 11>(),
        quickSum<width, 12>(),
        quickSum<width, 13>(),
        quickSum<width, 14>(),
        quickSum<width, 15>(),
        quickSum<width, 16>(),
        quickSum<width, 17>(),
        quickSum<width, 18>(),
        quickSum<width, 19>(),
    };
    return sums[times];
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
template<u64 value, u64 length>
struct QuickPow {
    static constexpr u64 op() noexcept {
        return QuickPow<value, length - 1>::op() * value;
    }
    QuickPow() = delete;
    ~QuickPow() = delete;
    QuickPow(const QuickPow&) = delete;
    QuickPow(QuickPow&&) = delete;
};

template<u64 value>
struct QuickPow<value, 0> {
    static constexpr u64 op() noexcept {
        return 1;
    }
    QuickPow() = delete;
    ~QuickPow() = delete;
    QuickPow(const QuickPow&) = delete;
    QuickPow(QuickPow&&) = delete;
};

template<u64 value, u64 length>
constexpr u64 quickPow() noexcept {
    return QuickPow<value, length>::op();
}

template<byte width>
inline u64 computeProductPart(byte times) noexcept {
    static constexpr u64 products[] = {
        quickPow<width, 0>(),
        quickPow<width, 1>(),
        quickPow<width, 2>(),
        quickPow<width, 3>(),
        quickPow<width, 4>(),
        quickPow<width, 5>(),
        quickPow<width, 6>(),
        quickPow<width, 7>(),
        quickPow<width, 8>(),
        quickPow<width, 9>(),
        quickPow<width, 10>(),
        quickPow<width, 11>(),
        quickPow<width, 12>(),
        quickPow<width, 13>(),
        quickPow<width, 14>(),
        quickPow<width, 15>(),
        quickPow<width, 16>(),
        quickPow<width, 17>(),
        quickPow<width, 18>(),
        quickPow<width, 19>(),
    };
    return products[times];
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

void FrequencyTable::removeFromTable(u64 value) noexcept {
    switch(value) {
        case 2:
            --num2;
            break;
        case 3:
            --num3;
            break;
        case 4:
            --num4;
            break;
        case 6:
            --num6;
            break;
        case 7:
            --num7;
            break;
        case 8:
            --num8;
            break;
        case 9:
            --num9;
            break;
        default:
            throw "Illegal value provided!"; // this is intentional and should call terminate
    }
}
