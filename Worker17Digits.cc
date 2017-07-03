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
#include "PrecomputedRange4.h"

constexpr auto thirdLevelWidth = 5;
Triple topRange[numElements<thirdLevelWidth>];

template<u64 count, u64 addonCount>
inline std::string doIt(int start, int stop) noexcept {
    ArrayView<count, addonCount> tmp;
	auto t8 = getTriples<8>();
	for (int i = start, j = 0; i < stop; ++i) {
		auto curr = t8[i];
		auto sum = curr.getSum();
		auto product = curr.getProduct();
		auto number = curr.getNumber() * fastPow10<4>;
		for (auto const& tmp2 : range4) {
			tmp[j].assume(sum + tmp2.getSum(), product * tmp2.getProduct(), number + tmp2.getNumber());
			++j;
		}
	}
	std::stringstream storage;
    static constexpr auto factor = getPartialSize<thirdLevelWidth, 49>();
    static_assert(isDivisibleBy<thirdLevelWidth, 49>(factor), "Not divisible");
	auto mkBody = [&tmp](auto mult) noexcept {
		auto start = mult * factor;
		auto stop = (mult + 1) * factor;
		auto fn = [&tmp, start, stop]() noexcept {
			std::stringstream storage;
			for (int i = start; i < stop; ++i) {
				auto s = topRange[i].getSum();
				auto p = topRange[i].getProduct();
				auto n = topRange[i].getNumber();
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
	for (auto& r1217 : topRange) {
		r1217.assume(t8->getSum(), t8->getProduct(), t8->getNumber() * fastPow10<12>);
		++t8;
	}
	populateWidth<8>();
    setupPrecomputedWidth4();
	auto fn = [](auto start, auto stop) noexcept {
		return std::async(std::launch::async, doIt<oneSeventhWorkUnit, digits4Width>, start, stop);
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
