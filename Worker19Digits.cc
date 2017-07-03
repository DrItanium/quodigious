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

// Perform quodigious checks on numbers using tons of different C++ tricks
#include <iostream>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <functional>
#include <future>
#include <map>
#include "qlib.h"
#include "Triple.h"
#include "PrecomputedRange3.h"

constexpr auto thirdLevelWidth = 8;
Triple range12To17[numElements<thirdLevelWidth>];

template<u64 count>
inline std::string doIt(int start, int stop) noexcept {
	std::array<Triple, count * numElements<2>> tmp;
	auto t8 = getTriples<8>();
	for (int i = start, j = 0; i < stop; ++i) {
		auto curr = t8[i];
		auto sum = curr.getSum();
		auto product = curr.getProduct();
		auto number = curr.getNumber() * fastPow10<3>;
		for (auto const& tmp2 : range3) {
			tmp[j].assume(sum + tmp2.getSum(), product * tmp2.getProduct(), number + tmp2.getNumber());
			++j;
		}
	}
	std::stringstream storage;
	static constexpr auto factor = numElements<thirdLevelWidth> / 49;
	static_assert((factor*49) == numElements<thirdLevelWidth>, "Not divisible");
	auto mkBody = [&tmp](auto mult) noexcept {
		auto start = mult * factor;
		auto stop = (mult + 1) * factor;
		auto fn = [&tmp, start, stop]() noexcept {
			std::stringstream storage;
			for (int i = start; i < stop; ++i) {
				auto s = range12To17[i].getSum();
				auto p = range12To17[i].getProduct();
				auto n = range12To17[i].getNumber();
				for (auto const & curr : tmp) {
					if (curr.isQuodigious(s, p, n)) {
						storage << curr.buildNumber(n) << std::endl;
					}
				}
			}
			return storage.str();
		};
		return std::async(std::launch::async, fn);
	};
	auto b0 = mkBody(0);
	decltype(b0) rest[48];
	for (int i = 0, j = 1; i < 48; ++i, ++j) {
		rest[i] = mkBody(j);
	}
	storage << b0.get();
	for (int i = 0; i < 48; ++i) {
		storage << rest[i].get();
	}
	return storage.str();
}

int main() {
	auto errorCode = 0;
	constexpr auto workUnitWidth = 4;
	constexpr auto fallOver = 8 - workUnitWidth;
	constexpr auto workUnitCount = numElements<workUnitWidth>;
	constexpr auto oneSeventhWorkUnit = workUnitCount / 7;
	std::stringstream collection0;
	// setup the triples
	populateWidth<thirdLevelWidth>();
	auto t8 = getTriples<thirdLevelWidth>();
	for (auto& r1217 : range12To17) {
		r1217.assume(t8->getSum(), t8->getProduct(), t8->getNumber() * fastPow10<11>);
		++t8;
	}
	populateWidth<8>();
	for (int i = 0; i < numElements<2>; ++i) {
		auto number = collection3[i];
		auto digits0 = number % 10;
		auto digits1 = (number / 10) % 10;
		auto digits2 = (number / 100) % 10;
		range3[i].assume(digits0 + digits1 + digits2, digits0 * digits1 * digits2, number);
	}
	auto fn = [](auto start, auto stop) noexcept {
		return std::async(std::launch::async, doIt<oneSeventhWorkUnit>, start, stop);
	};


	while(std::cin.good()) {
		int innerThreadId = 0;
		std::cin >> innerThreadId;
		if (innerThreadId < 0 || innerThreadId >= numElements<fallOver>) {
			std::cerr << "Illegal inner thread id, must be in the range [0," << numElements<fallOver> - 1 << "]" << std::endl;
			errorCode = 1;
			break;
		}
		if (!std::cin.good()) {
			break;
		}
		// divide the code up into seven parts
		auto start = workUnitCount * innerThreadId;
		auto stop0 = oneSeventhWorkUnit + start;
		auto stop1 = oneSeventhWorkUnit + stop0;
		auto stop2 = oneSeventhWorkUnit + stop1;
		auto stop3 = oneSeventhWorkUnit + stop2;
		auto stop4 = oneSeventhWorkUnit + stop3;
		auto stop5 = oneSeventhWorkUnit + stop4;
		auto stop6 = oneSeventhWorkUnit + stop5;
		auto stop = workUnitCount * (innerThreadId+ 1);
		if (stop != stop6) {
			errorCode = 1;
			std::cerr << "size mismatch!" << std::endl;
			break;
		}
		auto b0 = fn(start, stop0);
		auto b1 = fn(stop0, stop1);
		auto b2 = fn(stop1, stop2);
		auto b3 = fn(stop2, stop3);
		auto b4 = fn(stop3, stop4);
		auto b5 = fn(stop4, stop5);
		auto b6 = fn(stop5, stop6);
		collection0 << b0.get() << b1.get() << b2.get() << b3.get() << b4.get() << b5.get() << b6.get();
	}
	std::cout << collection0.str() << std::endl;
	return errorCode;
}
