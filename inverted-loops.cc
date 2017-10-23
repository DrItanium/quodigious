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
constexpr u64 sums2[numElements<2>] = {
    0x4, 0x5, 0x6, 0x8, 0x9, 0xa, 0xb,
    0x5, 0x6, 0x7, 0x9, 0xa, 0xb, 0xc,
    0x6, 0x7, 0x8, 0xa, 0xb, 0xc, 0xd,
    0x8, 0x9, 0xa, 0xc, 0xd, 0xe, 0xf,
    0x9, 0xa, 0xb, 0xd, 0xe, 0xf, 0x10,
    0xa, 0xb, 0xc, 0xe, 0xf, 0x10, 0x11,
    0xb, 0xc, 0xd, 0xf, 0x10, 0x11, 0x12,
};
constexpr u64 products2[numElements<2>] = {
    0x4, 0x6, 0x8, 0xc, 0xe, 0x10, 0x12,
    0x6, 0x9, 0xc, 0x12, 0x15, 0x18, 0x1b,
    0x8, 0xc, 0x10, 0x18, 0x1c, 0x20, 0x24,
    0xc, 0x12, 0x18, 0x24, 0x2a, 0x30, 0x36,
    0xe, 0x15, 0x1c, 0x2a, 0x31, 0x38, 0x3f,
    0x10, 0x18, 0x20, 0x30, 0x38, 0x40, 0x48,
    0x12, 0x1b, 0x24, 0x36, 0x3f, 0x48, 0x51,
};

constexpr u64 values2To4[] = {
    0xdc, 0xe6, 0xf0, 0x104, 0x10e, 0x118, 0x122,
    0x140, 0x14a, 0x154, 0x168, 0x172, 0x17c, 0x186,
    0x1a4, 0x1ae, 0x1b8, 0x1cc, 0x1d6, 0x1e0, 0x1ea,
    0x26c, 0x276, 0x280, 0x294, 0x29e, 0x2a8, 0x2b2,
    0x2d0, 0x2da, 0x2e4, 0x2f8, 0x302, 0x30c, 0x316,
    0x334, 0x33e, 0x348, 0x35c, 0x366, 0x370, 0x37a,
    0x398, 0x3a2, 0x3ac, 0x3c0, 0x3ca, 0x3d4, 0x3de,
};
template<u64 length>
inline void body(std::ostream& storage) noexcept {
    static_assert(length <= 19, "Can't have numbers over 19 digits at this time!");
    static constexpr auto threadCount = 49;
    decltype(std::async(std::launch::async, loopBodyString<4, length>, 0, 1, 0)) watcher[threadCount] = {
    std::async(std::launch::async, loopBodyString<4, length>, 4, 4, 220),
    std::async(std::launch::async, loopBodyString<4, length>, 5, 6, 230),
    std::async(std::launch::async, loopBodyString<4, length>, 6, 8, 240),
    std::async(std::launch::async, loopBodyString<4, length>, 8, 12, 260),
    std::async(std::launch::async, loopBodyString<4, length>, 9, 14, 270),
    std::async(std::launch::async, loopBodyString<4, length>, 10, 16, 280),
    std::async(std::launch::async, loopBodyString<4, length>, 11, 18, 290),
    std::async(std::launch::async, loopBodyString<4, length>, 5, 6, 320),
    std::async(std::launch::async, loopBodyString<4, length>, 6, 9, 330),
    std::async(std::launch::async, loopBodyString<4, length>, 7, 12, 340),
    std::async(std::launch::async, loopBodyString<4, length>, 9, 18, 360),
    std::async(std::launch::async, loopBodyString<4, length>, 10, 21, 370),
    std::async(std::launch::async, loopBodyString<4, length>, 11, 24, 380),
    std::async(std::launch::async, loopBodyString<4, length>, 12, 27, 390),
    std::async(std::launch::async, loopBodyString<4, length>, 6, 8, 420),
    std::async(std::launch::async, loopBodyString<4, length>, 7, 12, 430),
    std::async(std::launch::async, loopBodyString<4, length>, 8, 16, 440),
    std::async(std::launch::async, loopBodyString<4, length>, 10, 24, 460),
    std::async(std::launch::async, loopBodyString<4, length>, 11, 28, 470),
    std::async(std::launch::async, loopBodyString<4, length>, 12, 32, 480),
    std::async(std::launch::async, loopBodyString<4, length>, 13, 36, 490),
    std::async(std::launch::async, loopBodyString<4, length>, 8, 12, 620),
    std::async(std::launch::async, loopBodyString<4, length>, 9, 18, 630),
    std::async(std::launch::async, loopBodyString<4, length>, 10, 24, 640),
    std::async(std::launch::async, loopBodyString<4, length>, 12, 36, 660),
    std::async(std::launch::async, loopBodyString<4, length>, 13, 42, 670),
    std::async(std::launch::async, loopBodyString<4, length>, 14, 48, 680),
    std::async(std::launch::async, loopBodyString<4, length>, 15, 54, 690),
    std::async(std::launch::async, loopBodyString<4, length>, 9, 14, 720),
    std::async(std::launch::async, loopBodyString<4, length>, 10, 21, 730),
    std::async(std::launch::async, loopBodyString<4, length>, 11, 28, 740),
    std::async(std::launch::async, loopBodyString<4, length>, 13, 42, 760),
    std::async(std::launch::async, loopBodyString<4, length>, 14, 49, 770),
    std::async(std::launch::async, loopBodyString<4, length>, 15, 56, 780),
    std::async(std::launch::async, loopBodyString<4, length>, 16, 63, 790),
    std::async(std::launch::async, loopBodyString<4, length>, 10, 16, 820),
    std::async(std::launch::async, loopBodyString<4, length>, 11, 24, 830),
    std::async(std::launch::async, loopBodyString<4, length>, 12, 32, 840),
    std::async(std::launch::async, loopBodyString<4, length>, 14, 48, 860),
    std::async(std::launch::async, loopBodyString<4, length>, 15, 56, 870),
    std::async(std::launch::async, loopBodyString<4, length>, 16, 64, 880),
    std::async(std::launch::async, loopBodyString<4, length>, 17, 72, 890),
    std::async(std::launch::async, loopBodyString<4, length>, 11, 18, 920),
    std::async(std::launch::async, loopBodyString<4, length>, 12, 27, 930),
    std::async(std::launch::async, loopBodyString<4, length>, 13, 36, 940),
    std::async(std::launch::async, loopBodyString<4, length>, 15, 54, 960),
    std::async(std::launch::async, loopBodyString<4, length>, 16, 63, 970),
    std::async(std::launch::async, loopBodyString<4, length>, 17, 72, 980),
    std::async(std::launch::async, loopBodyString<4, length>, 18, 81, 990),

    };
    /*
    watcher[0] = std::async(std::launch::async, loopBodyString<4, length>, 4, 4, 220);
    watcher[1] = std::async(std::launch::async, loopBodyString<4, length>, 5, 6, 230);
    watcher[2] = std::async(std::launch::async, loopBodyString<4, length>, 6, 8, 240);
    watcher[3] = std::async(std::launch::async, loopBodyString<4, length>, 8, 12, 260);
    watcher[4] = std::async(std::launch::async, loopBodyString<4, length>, 9, 14, 270);
    watcher[5] = std::async(std::launch::async, loopBodyString<4, length>, 10, 16, 280);
    watcher[6] = std::async(std::launch::async, loopBodyString<4, length>, 11, 18, 290);
    watcher[7] = std::async(std::launch::async, loopBodyString<4, length>, 5, 6, 320);
    watcher[8] = std::async(std::launch::async, loopBodyString<4, length>, 6, 9, 330);
    watcher[9] = std::async(std::launch::async, loopBodyString<4, length>, 7, 12, 340);
    watcher[10] = std::async(std::launch::async, loopBodyString<4, length>, 9, 18, 360);
    watcher[11] = std::async(std::launch::async, loopBodyString<4, length>, 10, 21, 370);
    watcher[12] = std::async(std::launch::async, loopBodyString<4, length>, 11, 24, 380);
    watcher[13] = std::async(std::launch::async, loopBodyString<4, length>, 12, 27, 390);
    watcher[14] = std::async(std::launch::async, loopBodyString<4, length>, 6, 8, 420);
    watcher[15] = std::async(std::launch::async, loopBodyString<4, length>, 7, 12, 430);
    watcher[16] = std::async(std::launch::async, loopBodyString<4, length>, 8, 16, 440);
    watcher[17] = std::async(std::launch::async, loopBodyString<4, length>, 10, 24, 460);
    watcher[18] = std::async(std::launch::async, loopBodyString<4, length>, 11, 28, 470);
    watcher[19] = std::async(std::launch::async, loopBodyString<4, length>, 12, 32, 480);
    watcher[20] = std::async(std::launch::async, loopBodyString<4, length>, 13, 36, 490);
    watcher[21] = std::async(std::launch::async, loopBodyString<4, length>, 8, 12, 620);
    watcher[22] = std::async(std::launch::async, loopBodyString<4, length>, 9, 18, 630);
    watcher[23] = std::async(std::launch::async, loopBodyString<4, length>, 10, 24, 640);
    watcher[24] = std::async(std::launch::async, loopBodyString<4, length>, 12, 36, 660);
    watcher[25] = std::async(std::launch::async, loopBodyString<4, length>, 13, 42, 670);
    watcher[26] = std::async(std::launch::async, loopBodyString<4, length>, 14, 48, 680);
    watcher[27] = std::async(std::launch::async, loopBodyString<4, length>, 15, 54, 690);
    watcher[28] = std::async(std::launch::async, loopBodyString<4, length>, 9, 14, 720);
    watcher[29] = std::async(std::launch::async, loopBodyString<4, length>, 10, 21, 730);
    watcher[30] = std::async(std::launch::async, loopBodyString<4, length>, 11, 28, 740);
    watcher[31] = std::async(std::launch::async, loopBodyString<4, length>, 13, 42, 760);
    watcher[32] = std::async(std::launch::async, loopBodyString<4, length>, 14, 49, 770);
    watcher[33] = std::async(std::launch::async, loopBodyString<4, length>, 15, 56, 780);
    watcher[34] = std::async(std::launch::async, loopBodyString<4, length>, 16, 63, 790);
    watcher[35] = std::async(std::launch::async, loopBodyString<4, length>, 10, 16, 820);
    watcher[36] = std::async(std::launch::async, loopBodyString<4, length>, 11, 24, 830);
    watcher[37] = std::async(std::launch::async, loopBodyString<4, length>, 12, 32, 840);
    watcher[38] = std::async(std::launch::async, loopBodyString<4, length>, 14, 48, 860);
    watcher[39] = std::async(std::launch::async, loopBodyString<4, length>, 15, 56, 870);
    watcher[40] = std::async(std::launch::async, loopBodyString<4, length>, 16, 64, 880);
    watcher[41] = std::async(std::launch::async, loopBodyString<4, length>, 17, 72, 890);
    watcher[42] = std::async(std::launch::async, loopBodyString<4, length>, 11, 18, 920);
    watcher[43] = std::async(std::launch::async, loopBodyString<4, length>, 12, 27, 930);
    watcher[44] = std::async(std::launch::async, loopBodyString<4, length>, 13, 36, 940);
    watcher[45] = std::async(std::launch::async, loopBodyString<4, length>, 15, 54, 960);
    watcher[46] = std::async(std::launch::async, loopBodyString<4, length>, 16, 63, 970);
    watcher[47] = std::async(std::launch::async, loopBodyString<4, length>, 17, 72, 980);
    watcher[48] = std::async(std::launch::async, loopBodyString<4, length>, 18, 81, 990);
    */
    // above code is an unroll of the the following code
    //for (int i = 0; i < threadCount; ++i) {
    //    watcher[i] = std::async(std::launch::async, loopBodyString<4, length>, sums2[i], products2[i], values2To4[i]);
    //}
    for (int i = 0; i < threadCount; ++i) {
        storage << watcher[i].get();
    }
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
