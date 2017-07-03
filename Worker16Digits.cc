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

class Triple {
	private:
		static inline constexpr bool checkValue(u64 sum) noexcept {
			return (sum % 2 == 0) || (sum % 3 == 0);
		}
		static inline constexpr bool innerMostBody(u64 sum, u64 product, u64 value) noexcept {
			return checkValue(sum) && (value % product == 0) && (value % sum == 0);
		}
	public:
		Triple(u64 s, u64 p, u64 n) : _sum(s), _product(p), _number(n) { }
		Triple() : Triple(0, 0, 0) { }
		inline bool assume(u64 sum, u64 product, u64 number) noexcept {
			_sum = sum;
			_product = product;
			_number = number;
		}
		inline bool isQuodigious(u64 sCombine, u64 pCombine, u64 nCombine) const noexcept {
			return innerMostBody(sCombine + _sum, pCombine * _product, nCombine + _number);
		}
		inline u64 buildNumber(u64 offset) const noexcept {
			return _number + offset;
		}
		inline u64 getSum() const noexcept { return _sum; }
		inline u64 getProduct() const noexcept { return _product; }
		inline u64 getNumber() const noexcept { return _number; }
	private:
		u64 _sum;
		u64 _product;
		u64 _number;
};

template<u64 width>
constexpr int numberOfDigitsForGivenWidth() noexcept {
	static_assert(width >= 0, "Negative width doesn't make sense");
	return 7 * numberOfDigitsForGivenWidth<width - 1>();
}
template<> constexpr int numberOfDigitsForGivenWidth<0>() noexcept { return 1; }
template<u64 width>
constexpr auto numElements = numberOfDigitsForGivenWidth<width>();

template<u64 width>
constexpr u64 makeDigitAt(u64 input) noexcept {
	static_assert(width >= 0, "Can't have negative width!");
	return input * fastPow10<width>;
}

template<u64 width>
inline Triple* getTriples() noexcept {
	static_assert(width >= 2 && width < 9, "Illegal width!");
	static Triple elements[numElements<width>];
	return elements;
}

template<u64 width>
inline void populateWidth() noexcept {
	static_assert(width >= 2 && width < 9, "Illegal width!");
	populateWidth<width - 1>();
	auto* triple = getTriples<width>();
	auto* prev = getTriples<width - 1>();
	for (int i = 0; i < numElements<width - 1>; ++i) {
		auto tmp = prev[i];
		auto s = tmp.getSum();
		auto p = tmp.getProduct();
		auto n = makeDigitAt<1>(tmp.getNumber());
		for (int j = 2; j < 10; ++j) {
			if (j != 5) {
				*triple = Triple(s + j, p * j, n + j);
				++triple;

			}
		}
	}
}
//TODO: reduce memory footprint by specializing on 6
template<>
inline void populateWidth<2>() noexcept {
	auto* triple = getTriples<2>();
	for (int i = 2; i < 10; ++i) {
		if (i != 5) {
			auto numberOuter = makeDigitAt<1>(i);
			for (int j = 2; j < 10; ++j) {
				if (j != 5) {
					*triple = Triple(i + j, i * j, numberOuter + j);
					++triple;
				}
			}
		}
	}
}


constexpr auto thirdLevelWidth = 4;
constexpr auto digits4Width = 171;
constexpr auto digits3Width = numElements<2>;
Triple topRange[numElements<thirdLevelWidth>];
Triple range3[digits3Width];
Triple range4[digits4Width];
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

u64 collection4[digits4Width] = {
2224, 2272, 2288, 2336, 2368, 2384, 2432,
2448, 2464, 2496, 2624, 2672, 2688, 2736,
2768, 2784, 2832, 2848, 2864, 2896, 2928,
2944, 2976, 2992, 3232, 3248, 3264, 3296,
3328, 3344, 3376, 3392, 3424, 3472, 3488,
3632, 3648, 3664, 3696, 3728, 3744, 3776,
3792, 3824, 3872, 3888, 3936, 3968, 3984,
4224, 4272, 4288, 4336, 4368, 4384, 4432,
4448, 4464, 4496, 4624, 4672, 4688, 4736,
4768, 4784, 4832, 4848, 4864, 4896, 4928,
4944, 4976, 4992, 6224, 6272, 6288, 6336,
6368, 6384, 6432, 6448, 6464, 6496, 6624,
6672, 6688, 6736, 6768, 6784, 6832, 6848,
6864, 6896, 6928, 6944, 6976, 6992, 7232,
7248, 7264, 7296, 7328, 7344, 7376, 7392,
7424, 7472, 7488, 7632, 7648, 7664, 7696,
7728, 7744, 7776, 7792, 7824, 7872, 7888,
7936, 7968, 7984, 8224, 8272, 8288, 8336,
8368, 8384, 8432, 8448, 8464, 8496, 8624,
8672, 8688, 8736, 8768, 8784, 8832, 8848,
8864, 8896, 8928, 8944, 8976, 8992, 9232,
9248, 9264, 9296, 9328, 9344, 9376, 9392,
9424, 9472, 9488, 9632, 9648, 9664, 9696,
9728, 9744, 9776, 9792, 9824, 9872, 9888,
9936, 9968, 9984,
};

constexpr auto addonWidth = digits4Width;
template<u64 count>
inline std::string doIt(int start, int stop) noexcept {
	std::array<Triple, count * addonWidth> tmp;
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
	static constexpr auto factor = numElements<thirdLevelWidth> / 49;
	static_assert((factor*49) == numElements<thirdLevelWidth>, "Not divisible");
	auto mkBody = [&tmp](auto mult) noexcept {
		auto start = mult * factor;
		auto stop = (mult + 1) * factor;
		auto fn = [&tmp, start, stop]() noexcept {
			std::stringstream storage;
			for (int i = start; i < stop; ++i) {
				auto s = topRange[i].getSum();
				auto p = topRange[i].getProduct();
				auto n = topRange[i].getNumber();
				for (auto const& curr : tmp) {
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
	for (int i = 0; i < digits3Width; ++i) {
		auto number = collection3[i];
		auto digits0 = number % 10;
		auto digits1 = (number / 10) % 10;
		auto digits2 = (number / 100) % 10;
		range3[i].assume(digits0 + digits1 + digits2, digits0 * digits1 * digits2, number);
	}
	for (int i = 0; i < digits4Width; ++i) {
		auto number = collection4[i];
		auto digits0 = number % 10;
		auto digits1 = (number / 10) % 10;
		auto digits2 = (number / 100) % 10;
		auto digits3 = (number / 1000) % 10;
		auto sum = digits0 + digits1 + digits2 + digits3;
		auto product = digits0 * digits1 * digits2 * digits3;
		range4[i].assume(sum, product, number);
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
