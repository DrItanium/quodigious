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
FrequencyTable::FrequencyTable() : _value(0) { }
FrequencyTable::FrequencyTable(const FrequencyTable& t) : _value(t._value) { }
FrequencyTable::~FrequencyTable() { }
u64 FrequencyTable::getUniqueId() const noexcept {
    return _value;
}

void FrequencyTable::addToTable(u64 value) noexcept {
    ++_numbers[value - 2];
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
    return computeSumPart<2>(_numbers[0]) +
           computeSumPart<3>(_numbers[1]) +
           computeSumPart<4>(_numbers[2]) +
           computeSumPart<6>(_numbers[4]) +
           computeSumPart<7>(_numbers[5]) +
           computeSumPart<8>(_numbers[6]) +
           computeSumPart<9>(_numbers[7]);
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
    return computeProductPart<2>(_numbers[0]) *
           computeProductPart<3>(_numbers[1]) *
           computeProductPart<4>(_numbers[2]) *
           computeProductPart<6>(_numbers[4]) *
           computeProductPart<7>(_numbers[5]) *
           computeProductPart<8>(_numbers[6]) *
           computeProductPart<9>(_numbers[7]);
}
