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
#include <string>
#include "qlib.h"

template<typename T>
void field(std::ostream& out, const std::string& title, T value, bool newline = true) {
	out << "(" << title << " " << value << ")";
	if (newline) {
		out << '\n';
	} else {
		out << " ";
	}
}
template<u64 length>
void printoutInformation(u64 number, std::tuple<u64, u64> tup) noexcept {
	std::cout << "(qnum ";
	field(std::cout, "length", length, false);
	field(std::cout, "number", number, false);
	field(std::cout, "sum", std::get<0>(tup), false);
	field(std::cout, "product", std::get<1>(tup), false);
	std::cout << ")\n";
}
template<u64 length>
void body(u64 number) noexcept {
	auto tup = digitSumAndProduct<length>(number);
	printoutInformation<length>(number, tup);
}
template<>
void body<0>(u64 number) noexcept {
	std::cout << "0: " << number << ": " << number << ", " << number << std::endl;
	printoutInformation<0>(number, std::tuple<u64, u64>(number, number));
}

u64 numberLength(u64 input) noexcept {
	u64 count = 0u;
	do {
		++count;
		input /= 10u;
	} while(input != 0);
	return count;
}
int main() {
	while(std::cin.good()) {
		u64 currentValue = 0u;
		std::cin >> currentValue;
		// compute the length using log10 + 1
		auto almostLength = numberLength(currentValue);
		switch(almostLength) {
				case 0: body<0>(currentValue); break;
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
						 std::cerr << "Illegal value " << currentValue << " with length " << almostLength << std::endl;
						 return 1;
		}
	}
}
