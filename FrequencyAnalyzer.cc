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
FrequencyTable::FrequencyTable() : num2(0), num3(0), num4(0), num5(0), num6(0), num7(0), num8(0), num9(0) { }
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
            throw "Illegal value provided!";
    }
}
