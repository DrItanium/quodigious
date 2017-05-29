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
#include <cstdint>
#include <sstream>
#include <functional>
#include <future>
#include <vector>
#include "qlib.h"

inline constexpr bool checkValue(u64 sum) noexcept {
	return (sum % 2 == 0) || (sum % 3 == 0);
}
inline constexpr u64 innerMostBody(u64 sum, u64 product, u64 value) noexcept {
	if (checkValue(sum) && isQuodigious(value, sum, product)) {
		return value;
	}
	return 0;
}

using PackagedData = std::tuple<u64, u64, u64>;
using PrecomputedData = std::vector<PackagedData>;

PrecomputedData* precomputesPos6= nullptr;
PrecomputedData* precomputesPos8= nullptr;
PrecomputedData* precomputesPos10= nullptr;
void setup() noexcept {
	precomputesPos6 = new PrecomputedData;
	precomputesPos8 = new PrecomputedData;
	precomputesPos10 = new PrecomputedData;
	for (int i = 2; i < 10; ++i) {
		if (i != 5) {
			auto iIndex = (i * fastPow10<5>);
			auto iIndex8 = (i * fastPow10<7>);
			auto iIndex10 = (i * fastPow10<9>);
			auto iMul = i;
			auto iSum = i;
			for (int j = 2; j < 10; ++j) {
				if (j != 5) {
					precomputesPos6->emplace_back(PackagedData(iSum + j, iMul * j, iIndex + (j * fastPow10<6>)));
					precomputesPos8->emplace_back(PackagedData(iSum + j, iMul * j, iIndex8 + (j * fastPow10<8>)));
					precomputesPos10->emplace_back(PackagedData(iSum + j, iMul * j, iIndex10 + (j * fastPow10<10>)));
				}
			}
		}
	}
}


template<u64 pos, u64 max>
void loopBody(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept;

template<u64 pos, u64 max>
inline std::string loopBodyString(u64 sum, u64 product, u64 index) noexcept;

template<bool topLevel>
struct ActualLoopBody {
	ActualLoopBody() = delete;
	~ActualLoopBody() = delete;
	ActualLoopBody(ActualLoopBody&&) = delete;
	ActualLoopBody(const ActualLoopBody&) = delete;

	template<u64 pos, u64 max>
	static void body(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept {
		constexpr auto next = fastPow10<pos - 1>;
		constexpr auto follow = pos + 1;
		auto originalProduct = product;
		auto initialIncrement = [&product, &sum, &index]() { product <<= 1; sum += 2; index += multiply<2>(next); };
		auto advance = [&originalProduct, &product, &sum, &index]() noexcept { product += originalProduct; ++sum; index += next; };
		// this really, 5 and 6 only really runs well on massive numbers of cores
		if (pos == 2 || pos == 3 || pos == 4 || pos == 5) {
			auto mkComputation = [&sum, &product, &index]() noexcept { return std::async(std::launch::async, loopBodyString<follow, max>, sum, product, index); };
			initialIncrement();
			auto b0 = mkComputation();
			advance();
			auto b1 = mkComputation();
			advance();
			auto b2 = mkComputation();
			advance();
			advance();
			auto b3 = mkComputation();
			advance();
			auto b4 = mkComputation();
			advance();
			auto b5 = mkComputation();
			advance();
			auto b6 = mkComputation();
			storage << b0.get() << b1.get() << b2.get() << b3.get() << b4.get() << b5.get() << b6.get();
		} else if (pos == 6) {
			// skip ahead to 8
			for (const auto& a : *precomputesPos6) {
				loopBody<8, max>(storage, sum + std::get<0>(a), product * std::get<1>(a), index + std::get<2>(a));
			}
		} else if (pos == 8) {
			// skip ahead to 10
			for (const auto& a : *precomputesPos8) {
				loopBody<10, max>(storage, sum + std::get<0>(a), product * std::get<1>(a), index + std::get<2>(a));
			}
		} else if (pos == 10) {
			for (const auto& a : *precomputesPos10) {
				loopBody<12, max>(storage, sum + std::get<0>(a), product * std::get<1>(a), index + std::get<2>(a));
			}
		} else {
			initialIncrement();
			for (int i = 2; i < 10; ++i) {
				if (i != 5) {
					loopBody<follow, max>(storage, sum, product, index);
				}
				advance();
			}
		}
	}
};

template<>
struct ActualLoopBody<true> {
	ActualLoopBody() = delete;
	~ActualLoopBody() = delete;
	ActualLoopBody(ActualLoopBody&&) = delete;
	ActualLoopBody(const ActualLoopBody&) = delete;
	template<u64 pos, u64 max>
	static void body(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept {
		static_assert(max == pos, "Can't have a top level if the position and max don't match!");
		constexpr auto next = fastPow10<pos - 1>;
		auto originalProduct = product;
		product <<= 1;
		sum += 2;
		index += multiply<2>(next);
		auto advance = [&originalProduct, &product, &sum, &index]() noexcept { product += originalProduct; ++sum; index += next; };
		auto merge = [&storage](auto value) noexcept { if (value != 0) { storage << value << std::endl; } };
		merge(innerMostBody(sum, product, index)); // 2
		advance();
		merge(innerMostBody(sum, product, index)); // 3
		advance();
		merge(innerMostBody(sum, product, index)); // 4
		advance();
		advance();
		merge(innerMostBody(sum, product, index)); // 6
		advance();
		merge(innerMostBody(sum, product, index)); // 7
		advance();
		merge(innerMostBody(sum, product, index)); // 8
		advance();
		merge(innerMostBody(sum, product, index)); // 9
	}
};

template<u64 pos, u64 max>
inline void loopBody(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept {
	static_assert (pos <= max, "Position can't be larger than maximum!");
	// walk through two separate set of actions
	ActualLoopBody<pos == max>::template body< pos, max > (storage, sum, product, index);
}

template<u64 pos, u64 max>
inline std::string loopBodyString(u64 sum, u64 product, u64 index) noexcept {
	std::ostringstream storage;
	loopBody<pos, max> (storage, sum, product, index);
	return storage.str();
}

template<u64 length>
inline void body(std::ostream& storage) noexcept {
    static_assert(length <= 19, "Can't have numbers over 19 digits at this time!");
	auto p0 = std::async(std::launch::async, loopBodyString<2, length>, 2, 2, 2);
	auto p1 = std::async(std::launch::async, loopBodyString<2, length>, 4, 4, 4);
	auto p2 = std::async(std::launch::async, loopBodyString<2, length>, 6, 6, 6);
	auto p3 = std::async(std::launch::async, loopBodyString<2, length>, 8, 8, 8);
	storage << p0.get() << p1.get() << p2.get() << p3.get();
}

int main() {
    std::ostringstream storage;
	setup();
    while(std::cin.good()) {
        u64 currentIndex = 0;
        std::cin >> currentIndex;
        if (std::cin.good()) {
            switch(currentIndex) {
                case 12: body<12>(storage); break;
                case 13: body<13>(storage); break;
                case 14: body<14>(storage); break;
                case 15: body<15>(storage); break;
                case 16: body<16>(storage); break;
                case 17: body<17>(storage); break;
                case 18: body<18>(storage); break;
                case 19: body<19>(storage); break;
                default:
                         std::cerr << "Illegal index " << currentIndex << std::endl;
                         return 1;
            }
            std::cout << storage.str() << std::endl;
            storage.str("");
        }
    }
    return 0;
}
