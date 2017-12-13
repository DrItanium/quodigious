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
FrequencyTable::FrequencyTable() : _value(0), _product(1), _sum(0) { }
FrequencyTable::FrequencyTable(const FrequencyTable& t) : _value(t._value), _product(t._product), _sum(t._sum) { }
FrequencyTable::~FrequencyTable() { }
u64 FrequencyTable::getUniqueId() const noexcept {
    return _value;
}

void FrequencyTable::addToTable(u64 value) noexcept {
    ++_numbers[value - 2];
    _product *= value;
    _sum += value;
}
