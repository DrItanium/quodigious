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
    static constexpr bool activateInnerThreadMode() noexcept {
        // effective placement of the 49 threads we use will lead to effective computation!
        // too high up and each thread does too much work, too low and the threads do too little
        // work. The same thread placement values do not always work either!
        //
        // View each thread as a computation range and it should work better
        switch(max) {
            case 10:
            case 11:
                return pos == 2 || pos == 3;
            case 12:
                return pos == 3 || pos == 4;
            case 13:
                return pos == 2;
            case 14:
            case 15:
            case 16:
            case 17:
                return pos == 2 || pos == 7;
            default:
                return false;
        }
    }
    template<u64 pos, u64 max>
    static constexpr bool activateHalvingMode() noexcept {
        // instead of dividing by seven, do it in half
        switch(max) {
            case 13:
                return pos == 4 || pos == 6 || pos == 7;
            case 14:
            case 15:
            case 16:
            case 17:
                return pos == 8;
            default:
                return false;
        }
    }
    // TODO: add support for disallowing the activation of halving and seven mode at the same time
	template<u64 pos, u64 max>
	static void body(std::ostream& storage, u64 sum, u64 product, u64 index) noexcept {
        static constexpr auto next = fastPow10<pos - 1>;
        static constexpr auto follow = pos + 1;
        static constexpr auto doubleNext = next << 1;
        auto originalProduct = product;
        product <<= 1;
        sum += 2;
        index += doubleNext;
        if (activateInnerThreadMode<pos, max>()) {
            auto fn = [&sum, &product, &index]() { return std::async(std::launch::async, loopBodyString<follow, max>, sum, product, index); };
            auto p0 = fn();
            product += originalProduct;
            ++sum;
            index += next;
            auto p1 = fn();
            product += originalProduct;
            ++sum;
            index += next;
            auto p2 = fn();
            product += (originalProduct << 1);
            sum += 2;
            index += doubleNext;
            auto p3 = fn();
            product += originalProduct;
            ++sum;
            index += next;
            auto p4 = fn();
            product += originalProduct;
            ++sum;
            index += next;
            auto p5 = fn();
            product += originalProduct;
            ++sum;
            index += next;
            auto p6 = fn();

            storage << p0.get() << p1.get() << p2.get() << p3.get() << p4.get() << p5.get() << p6.get();
        } else if (activateHalvingMode<pos, max>()) {
            auto doThree = [originalProduct](auto s, auto p, auto i) {
                std::ostringstream output;
                auto sum = s;
                auto product = p;
                auto index = i;
                loopBody<follow, max>(output, sum, product, index); // 2
                product += originalProduct;
                ++sum;
                index += next;
                loopBody<follow, max>(output, sum, product, index); // 3
                product += originalProduct;
                ++sum;
                index += next;
                loopBody<follow, max>(output, sum, product, index); // 4
                return output.str();
            };
            auto p0 = std::async(std::launch::async, doThree, sum, product, index);
            product += originalProduct;
            ++sum;
            index += next;
            product += originalProduct;
            ++sum;
            index += next;
            product += (originalProduct << 1);
            sum += 2;
            index += doubleNext;
            auto p1 = std::async(std::launch::async, doThree, sum, product, index);
            product += originalProduct;
            ++sum;
            index += next;
            product += originalProduct;
            ++sum;
            index += next;
            product += originalProduct;
            ++sum;
            index += next;
            loopBody<follow, max>(storage,sum,product,index);

            storage << p0.get() << p1.get();
        } else {
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
