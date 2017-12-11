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

// Perform numeric quodigious checks
#include "qlib.h"
#include "FrequencyAnalyzer.h"
#include <future>
// 64-bit tweakables
/*
 * Perform exact computation (sum and product checks) instead of approximations
 */
constexpr auto exact = true;
/*
 * Don't do sum checks, only product checks
 */
constexpr auto expensiveChecks = false;
/*
 * This will greatly improve speed but omit all sums which are not divisible by
 * three, thus it won't be 100% accurate
 */
constexpr auto oddApprox = true;

/*
 * Enabling this option will force all odd sums (mod3) to be discarded (so the other half of the computation space).
 * This option has lower priority than ODD_APPROX
 */
constexpr auto evenApprox = false;
constexpr bool approximationCheckFailed(u64 sum) noexcept {
	if (expensiveChecks) {
		return false;
	} else if (oddApprox && !evenApprox) {
		return sum % 3 != 0;
	} else if (evenApprox && !oddApprox) {
		return sum % 2 != 0;
	} else {
		return (sum % 3 != 0) && (sum % 2 != 0);
	}
}

template<u64 length>
inline void innerBody(std::ostream& stream, FrequencyTable& table, u64 index, u64 digit) noexcept;
template<u64 length>
inline void body(std::ostream& stream, FrequencyTable& table, u64 index = 0) noexcept {
	static_assert(length <= 19, "Can't have numbers over 19 digits on 64-bit numbers!");
	static_assert(length != 0, "Can't have length of zero!");
	// start at the most significant digit and move inward, that way we don't need
	// to keep multiple template parameters around.
	// a 19 digit number means multiplying by 10^18 etc
	constexpr auto inner = length - 1;
	// use the compiler to compute the 10^inner at compile time to eliminate further
	// multiply operations
	constexpr auto next = fastPow10<inner>;
	// the current product that we get in is what we add to the running product total
	// so if product is 4 on function call then baseProduct will be 4.
	// Then we add the baseProduct to the running total (because this is what multiplication
	// is).
	//
	// So using 4 as our base, 4 * 2 is the same as 4 + 4, 4 *3 (+ 4 4 4), etc
	// Thus we can keep updating the product after each computation.
	//
	// I got the idea from strength reduction in compiler optimization theory
	// we don't include the digits zero or 1 so just skip them by adding two
	// or the equivalent for the corresponding thing
    // we use the stack to keep track of sums, products, and current indexes
    // instead of starting with a whole number and breaking it apart.
    // Walking down the call graph will cause another sum, product, and index
    // part to be added to the provided sum, product, and index.
    //
    // through observational evidence, I found that the least significant
    // digits are all even. Even if it turns out that this isn't the case
    // I can always perform the odd digit checks later on at a significant
    // reduction in speed cost!
    if (length >= 11) {
		// when we are greater than 10 digit numbers, it is a smart idea to
		// perform divide and conquer at each level above 10 digits. The number of
		// threads used for computation is equal to: 2^(width - 10).
		auto fn = [&table, index](auto p0, auto p1, auto p2) noexcept {
			std::ostringstream stream;
			innerBody<inner>(stream, table, index + (p0 * next), p0);
			innerBody<inner>(stream, table, index + (p1 * next), p1);
			innerBody<inner>(stream, table, index + (p2 * next), p2);
			return stream.str();
		};
		auto lowerHalf = std::async(std::launch::async, fn, 3, 4, 6);
		auto upperHalf = std::async(std::launch::async, fn, 7, 8, 9);
		// perform computation on this primary thread because we want to be able
		// to maximize how much work we do and make the amount of work in each
		// thread even. The same number of threads are spawned but the primary
		// thread that spawned the children is reused below.
		innerBody<inner>(stream, table, index + (2 * next), 2); // 2
		stream << lowerHalf.get() << upperHalf.get();
    } else {
        constexpr auto incr = (length == 1) ? 2 : 1;
        // see if we can't just make the compiler handle the incrementation
        // instead of us thus, the code is cleaner too :D
        for (auto i = 2; i < 10; i += incr) {
            if (i != 5) {
                innerBody<inner>(stream, table, index + (i * next), i);
            }
        }
    }
}


template<u64 length>
inline void innerBody(std::ostream& stream, FrequencyTable& table, u64 index, u64 digit) noexcept {
	// this double template instantiation is done to make sure that the compiler
	// does not attempt to instantiate infinitely, if this code was in place
	// of the call to innerbody in body then the compiler would not stop
	// instiantiating. we can then also perform specialization on length zero
    auto copy = table;
    copy.addToTable(digit);
	body<length>(stream, copy, index);
}
template<>
inline void innerBody<0>(std::ostream& stream, FrequencyTable& table, u64 index, u64 digit) noexcept {
	//// specialization
	if (!expensiveChecks) {
        auto sum = table.computeSum() + digit;
		if (approximationCheckFailed(sum)) {
			return;
		}
        auto product = table.computeProduct() * digit;
		if (exact) {
			if (index % product != 0) {
				return;
			}
			if (index % sum == 0) {
				stream << index << '\n';
			}
		} else {
			if (index % product == 0) {
				stream << index << '\n';
			}
		}
	} else {
        auto product = table.computeProduct() * digit;
		if (exact) {
            if (index % product == 0) {
                auto sum = table.computeSum() + digit;
                if (index % sum == 0) {
                    stream << index << '\n';
                }
            }
		} else {
			if (index % product == 0) {
				stream << index << '\n';
			}
		}
	}
}

template<u64 index>
inline void initialBody() noexcept {
	// we don't want main aware of any details of how computation is performed.
	// This allows the decoupling of details from main and the computation body itself
    FrequencyTable table;
    body<index>(std::cout, table);
}

int main() {
	while(std::cin.good()) {
		u64 currentIndex = 0;
		std::cin >> currentIndex;
		if (std::cin.good()) {
			switch(currentIndex) {
				case 10: initialBody<10>(); break;
				case 11: initialBody<11>(); break;
				case 12: initialBody<12>(); break;
				case 13: initialBody<13>(); break;
				case 14: initialBody<14>(); break;
				case 15: initialBody<15>(); break;
				case 16: initialBody<16>(); break;
				case 17: initialBody<17>(); break;
				case 18: initialBody<18>(); break;
				case 19: initialBody<19>(); break;
				default:
						 std::cerr << "Illegal index " << currentIndex << std::endl;
						 return 1;
			}
			std::cout << std::endl;
		}
	}
	return 0;
}
