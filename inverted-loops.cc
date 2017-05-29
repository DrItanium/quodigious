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
#include "qlib.h"

using byte = uint8_t;
inline constexpr bool checkValue(u64 sum) noexcept {
	return (sum % 2 == 0) || (sum % 3 == 0);
}
inline constexpr u64 innerMostBody(u64 sum, u64 product, u64 value) noexcept {
    if (checkValue(sum)) {
        if (componentQuodigious(value, sum)) {
            if (componentQuodigious(value, product)) {
                return value;
            }
        }
    }
    return 0;
}
inline void merge(u64 value, std::ostream& storage) noexcept {
    if (value != 0) {
        storage << value << std::endl;
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
		product <<= 1;
		sum += 2;
		index += multiply<2>(next);
		auto advance = [&originalProduct, &product, &sum, &index]() noexcept { product += originalProduct; ++sum; index += next; };
		if (pos == 2 || pos == 3 || pos == 4) {
			auto b0 = std::async(std::launch::async, loopBodyString<follow, max>, sum, product, index);
			advance();
			auto b1 = std::async(std::launch::async, loopBodyString<follow, max>, sum, product, index);
			advance();
			auto b2 = std::async(std::launch::async, loopBodyString<follow, max>, sum, product, index);
			advance();
			advance();
			auto b3 = std::async(std::launch::async, loopBodyString<follow, max>, sum, product, index);
			advance();
			auto b4 = std::async(std::launch::async, loopBodyString<follow, max>, sum, product, index);
			advance();
			auto b5 = std::async(std::launch::async, loopBodyString<follow, max>, sum, product, index);
			advance();
			auto b6 = std::async(std::launch::async, loopBodyString<follow, max>, sum, product, index);
			storage << b0.get() << b1.get() << b2.get() << b3.get() << b4.get() << b5.get() << b6.get();
		} else {
			loopBody<follow, max>(storage, sum, product, index); // 2 
			advance();
			loopBody<follow, max>(storage, sum, product, index); // 3
			advance();
			loopBody<follow, max>(storage, sum, product, index); // 4
			advance();
			advance();
			loopBody<follow, max>(storage, sum, product, index); // 6
			advance();
			loopBody<follow, max>(storage, sum, product, index); // 7
			advance();
			loopBody<follow, max>(storage, sum, product, index); // 8
			advance();
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
		constexpr auto next = fastPow10<pos - 1>;
		auto originalProduct = product;
		product <<= 1;
		sum += 2;
		index += multiply<2>(next);
		auto advance = [&originalProduct, &product, &sum, &index]() { product += originalProduct; ++sum; index += next; };
		merge(innerMostBody(sum, product, index), storage); // 2
		advance();
		merge(innerMostBody(sum, product, index), storage); // 3
		advance();
		merge(innerMostBody(sum, product, index), storage); // 4
		advance();
		advance();
		merge(innerMostBody(sum, product, index), storage); // 6
		advance();
		merge(innerMostBody(sum, product, index), storage); // 7
		advance();
		merge(innerMostBody(sum, product, index), storage); // 8
		advance();
		merge(innerMostBody(sum, product, index), storage); // 9
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

template<u64 pos, u64 length>
inline std::string startBody() noexcept {
	return loopBodyString<2, length>(pos, pos, pos);
}

template<u64 length>
inline void body(std::ostream& storage) noexcept {
    static_assert(length <= 19, "Can't have numbers over 19 digits at this time!");
    // this is not going to change ever!
	//auto b0 = std::async(std::launch::async, startBody<2, length>);
	//auto b1 = std::async(std::launch::async, startBody<4, length>);
	//auto b2 = std::async(std::launch::async, startBody<6, length>);
	//auto b3 = std::async(std::launch::async, startBody<8, length>);
	//storage << b0.get() << b1.get() << b2.get() << b3.get();
	storage << startBody<2, length>() << startBody<4, length>() << startBody<6, length>() << startBody<8, length>();
}

int main() {
    std::ostringstream storage;
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
