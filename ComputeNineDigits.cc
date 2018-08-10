//  Copyright (c) 2018 Joshua Scoggins
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

// Perform numeric quodigious checks
#include "qlib.h"
template<u64 length>
void body(u64 sum = 0, u64 product = 1, u64 index = 0) noexcept {
	static_assert(length >= 10, "Cannot go lower than 10 digits!");
    if constexpr (length == 10) {
        if ((index % product == 0) && (index % sum == 0)) {
            std::cout << index << std::endl;
        }
    } else {
	    constexpr auto inner = length - 1;
	    constexpr auto next = fastPow10<inner>;
	    // unlike the 64-bit version of this code, doing the 32-bit ints for 9 digit
	    // numbers (this code is not used when you request 64-bit numbers!)
	    // does not require as much optimization. We can walk through digit level
	    // by digit level (even if the digit does not contribute too much to the
	    // overall process!).
	    for (auto i = 2; i < 10; ++i) {
            body<inner>(sum + i, product * i, index + (i * next));
	    }
    }
}




int main(int argc, char** argv) {
    if (argc != 2) {
        return 1;
    }
    u64 currentWidth = 0;
    std::string tmp(argv[1]);
    std::istringstream w(tmp);
    w >> currentWidth;
	while(std::cin.good()) {
		u64 currentIndex = 0;
		u64 currentSum = 0;
		u64 currentProduct = 0;
		std::cin >> currentSum;
		std::cin >> currentProduct;
		std::cin >> currentIndex;
		if (std::cin.good()) {
			switch(currentWidth) {
				case 11: body<11>(currentSum, currentProduct, currentIndex); break;
				case 12: body<12>(currentSum, currentProduct, currentIndex); break;
				case 13: body<13>(currentSum, currentProduct, currentIndex); break;
				case 14: body<14>(currentSum, currentProduct, currentIndex); break;
				case 15: body<15>(currentSum, currentProduct, currentIndex); break;
				case 16: body<16>(currentSum, currentProduct, currentIndex); break;
				case 17: body<17>(currentSum, currentProduct, currentIndex); break;
				case 18: body<18>(currentSum, currentProduct, currentIndex); break;
				case 19: body<19>(currentSum, currentProduct, currentIndex); break;
				default:
						 std::cerr << "Illegal index " << currentIndex << std::endl;
						 return 1;
			}
		}
	}
	return 0;
}
