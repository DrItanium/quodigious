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
#include <future>
#include <cstdint>
#include <vector>
#include <functional>
#include "qlib.h"

template<u64 length>
void body(u64 number) noexcept {
	auto tup = digitSumAndProduct<length>(number);
	std::cout << number << ": " << std::get<0>(tup) << ", " << std::get<1>(tup) << std::endl;
}

int main() {
	while(std::cin.good()) {
		u64 currentValue = 0u;
		std::cin >> currentValue;
		// compute the length using log10 + 1
		auto almostLength = static_cast<u64>(std::floor(std::log10(currentValue)));
		switch(almostLength + 1) {
				case 1: body<1>(currentValue); break;
				case 2: body<2>(currentValue); break;
				case 3: body<3>(currentValue); break;
				case 4: body<4>(currentValue); break;
				case 5: body<5>(currentValue); break;
				case 6: body<6>(currentValue); break;
				case 7: body<7>(currentValue); break;
				case 8: body<8>(currentValue); break;
				case 9: body<9>(currentValue); break;
				case 10: body<10>(currentValue); break;
				case 11: body<11>(currentValue); break;
				case 12: body<12>(currentValue); break;
				case 13: body<13>(currentValue); break;
				case 14: body<14>(currentValue); break;
				case 15: body<15>(currentValue); break;
				case 16: body<16>(currentValue); break;
				case 17: body<17>(currentValue); break;
				case 18: body<18>(currentValue); break;
				case 19: body<19>(currentValue); break;
				default:
						 std::cerr << "Illegal value " << currentValue << " with length " << almostLength + 1 << std::endl;
						 return 1;
		}
	}
}
