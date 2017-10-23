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
#include <map>
#include "qlib.h"

constexpr bool checkValue(u64 sum) noexcept {
	return (isEven(sum)) || (sum % 3 == 0);
}
constexpr u64 performCheck(u64 sum, u64 product, u64 value) noexcept {
	if (checkValue(sum) && isQuodigious(value, sum, product)) {
		return value;
	}
	return 0;
}
void innerMostBody(std::ostream& stream, u64 sum, u64 product, u64 value) noexcept {
	// inject the least significant digits 2,4,6,8
	merge(performCheck(sum + 2, product << 1, value + 2), stream); // 2
	merge(performCheck(sum + 4, product << 2, value + 4), stream); // 4
	merge(performCheck(sum + 6, (product << 1) + (product << 2) , value + 6), stream); // 6
	merge(performCheck(sum + 8, product << 3, value + 8), stream); // 8
}


u64 sums2[] = {
0x4, 0x5, 0x6, 0x8, 0x9, 0xa, 0xb,
0x5, 0x6, 0x7, 0x9, 0xa, 0xb, 0xc,
0x6, 0x7, 0x8, 0xa, 0xb, 0xc, 0xd,
0x8, 0x9, 0xa, 0xc, 0xd, 0xe, 0xf,
0x9, 0xa, 0xb, 0xd, 0xe, 0xf, 0x10,
0xa, 0xb, 0xc, 0xe, 0xf, 0x10, 0x11,
0xb, 0xc, 0xd, 0xf, 0x10, 0x11, 0x12,
};
u64 products2[] = {
0x4, 0x6, 0x8, 0xc, 0xe, 0x10, 0x12,
0x6, 0x9, 0xc, 0x12, 0x15, 0x18, 0x1b,
0x8, 0xc, 0x10, 0x18, 0x1c, 0x20, 0x24,
0xc, 0x12, 0x18, 0x24, 0x2a, 0x30, 0x36,
0xe, 0x15, 0x1c, 0x2a, 0x31, 0x38, 0x3f,
0x10, 0x18, 0x20, 0x30, 0x38, 0x40, 0x48,
0x12, 0x1b, 0x24, 0x36, 0x3f, 0x48, 0x51,
};
u64 numbers2[] = {
0x16, 0x17, 0x18, 0x1a, 0x1b, 0x1c, 0x1d,
0x20, 0x21, 0x22, 0x24, 0x25, 0x26, 0x27,
0x2a, 0x2b, 0x2c, 0x2e, 0x2f, 0x30, 0x31,
0x3e, 0x3f, 0x40, 0x42, 0x43, 0x44, 0x45,
0x48, 0x49, 0x4a, 0x4c, 0x4d, 0x4e, 0x4f,
0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59,
0x5c, 0x5d, 0x5e, 0x60, 0x61, 0x62, 0x63,
};

u64 values2To4[] = {
0xdc, 0xe6, 0xf0, 0x104, 0x10e, 0x118, 0x122,
0x140, 0x14a, 0x154, 0x168, 0x172, 0x17c, 0x186,
0x1a4, 0x1ae, 0x1b8, 0x1cc, 0x1d6, 0x1e0, 0x1ea,
0x26c, 0x276, 0x280, 0x294, 0x29e, 0x2a8, 0x2b2,
0x2d0, 0x2da, 0x2e4, 0x2f8, 0x302, 0x30c, 0x316,
0x334, 0x33e, 0x348, 0x35c, 0x366, 0x370, 0x37a,
0x398, 0x3a2, 0x3ac, 0x3c0, 0x3ca, 0x3d4, 0x3de,
};

template<u64 pos, u64 max>
inline void loopBody(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept;


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
		if (pos == 2) {
			static constexpr auto threadCount = 49;
			auto mkComputation = [sum, product, index](auto uS, auto uP, auto uInd) noexcept { return std::async(std::launch::async, loopBodyString<4, max>, sum + uS, product * uP, index + uInd); };
			auto* ptrSum = sums2;
			auto* ptrProd = products2;
			auto* ptrVals = values2To4;
			decltype(mkComputation(*ptrSum, *ptrProd, *ptrVals)) watcher[threadCount];
			for (int i = 0; i < threadCount; ++i, ++ptrSum, ++ptrProd, ++ptrVals) {
				watcher[i] = mkComputation(*ptrSum, *ptrProd, *ptrVals);
			}
			for (int i = 0; i < threadCount; ++i) {
				storage << watcher[i].get();
			}
		} else {
            static constexpr auto next = fastPow10<pos - 1>;
            static constexpr auto follow = pos + 1;
			static constexpr auto doubleNext = next << 1;
            auto originalProduct = product;
			product <<= 1;
			sum += 2;
			index += doubleNext;
			loopBody<follow, max>(storage, sum, product, index); // 2
			product += originalProduct;
			++sum;
			index += next;
			loopBody<follow, max>(storage, sum, product, index); // 3
			product += originalProduct;
			++sum;
			index += next;
			loopBody<follow, max>(storage, sum, product, index); // 4
			product += (originalProduct << 1);
			sum += 2;
			index += doubleNext;
			loopBody<follow, max>(storage, sum, product, index); // 6
			product += originalProduct;
			++sum;
			index += next;
			loopBody<follow, max>(storage, sum, product, index); // 7
			product += originalProduct;
			++sum;
			index += next;
			loopBody<follow, max>(storage, sum, product, index); // 8
			product += originalProduct;
			++sum;
			index += next;
			loopBody<follow, max>(storage, sum, product, index); // 9
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
        static constexpr auto next = fastPow10<pos - 1>;
		static constexpr auto doubleNext = next << 1;
        auto originalProduct = product;
        product <<= 1;
        sum += 2;
        index += doubleNext;
        (innerMostBody(storage, sum, product, index)); // 2
		product += originalProduct;
		++sum;
		index += next;
        (innerMostBody(storage, sum, product, index)); // 3
		product += originalProduct;
		++sum;
		index += next;
        (innerMostBody(storage, sum, product, index)); // 4
		product += (originalProduct << 1);
		sum += 2;
		index += doubleNext;
        (innerMostBody(storage, sum, product, index)); // 6
		product += originalProduct;
		++sum;
		index += next;
        (innerMostBody(storage, sum, product, index)); // 7
		product += originalProduct;
		++sum;
		index += next;
        (innerMostBody(storage, sum, product, index)); // 8
		product += originalProduct;
		++sum;
		index += next;
        (innerMostBody(storage, sum, product, index)); // 9


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
	loopBody<2, length>(storage, 0, 1, 0);
}

int main() {
    std::ostringstream storage;
    while(std::cin.good()) {
        u64 currentIndex = 0;
        std::cin >> currentIndex;
        if (std::cin.good()) {
            switch(currentIndex) {
                case 10: body<10>(storage); break;
                case 11: body<11>(storage); break;
                case 12: body<12>(storage); break;
                case 13: body<13>(storage); break;
                case 14: body<14>(storage); break;
                case 15: body<15>(storage); break;
                case 16: body<16>(storage); break;
                case 17: body<17>(storage); break;
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
