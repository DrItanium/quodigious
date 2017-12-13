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
#ifndef FREQUENCY_ANALYZER_H__
#define FREQUENCY_ANALYZER_H__
#include "qlib.h"

// A class for constructing a unique key from a given number as well as computing product and sum
// This class is meant to heavily reduce recomputation while being very cheap to copy
//

class FrequencyTable {
    public:
        FrequencyTable();
        FrequencyTable(const FrequencyTable&);
        FrequencyTable(FrequencyTable&&) = delete;
        ~FrequencyTable();
        u64 getUniqueId() const noexcept;
        void addToTable(u64 digit) noexcept;
        u64 computeSum() const noexcept { return _sum; }
        u64 computeProduct() const noexcept { return _product; }
    private:
        union {
            byte _numbers[sizeof(uint64_t)];
            uint64_t _value;
        };
        uint64_t _product;
        uint64_t _sum;
};
#endif
