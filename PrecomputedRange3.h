//  Copyright (c) 2017 Joshua Scoggins
//
//  This software is provided 'as-is', without any express or implied
//  warranty. In no event will the authors be held liable for any damages
//  arising from the use of this software.  //
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

#ifndef PRECOMPUTED_RANGE3_H__
#define PRECOMPUTED_RANGE3_H__
#include "qlib.h"
#include "Triple.h"

constexpr auto digits3Width = numElements<2>;
Triple range3[digits3Width];
// these were the three least significant digits for all numbers 13 digits and
// above! So we can do 49 numbers instead of 196!
u64 collection3[digits3Width] = {
	224, 232, 248, 264, 272, 288, 296,
	328, 336, 344, 368, 376, 384, 392,
	424, 432, 448, 464, 472, 488, 496,
	624, 632, 648, 664, 672, 688, 696,
	728, 736, 744, 768, 776, 784, 792,
	824, 832, 848, 864, 872, 888, 896,
	928, 936, 944, 968, 976, 984, 992,
};

inline void setupPrecomputedWidth3() noexcept {
    for (int i = 0; i < digits3Width; ++i) {
        auto number = collection3[i];
        auto digits0 = number % 10;
        auto digits1 = (number / 10) % 10;
        auto digits2 = (number / 100) % 10;
        range3[i].assume(digits0 + digits1 + digits2, digits0 * digits1 * digits2, number);
    }
}
#endif
