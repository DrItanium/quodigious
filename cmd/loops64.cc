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
#include <future>

template<u32 length>
void body32(u32 sum = 0, u32 product = 1, u32 index = 0) noexcept {
	static_assert(length <= 9, "Can't have numbers over 9 digits on 32-bit numbers!");
    if constexpr (length == 0) {
        if ((index % product == 0) && (index % sum == 0)) {
            std::cout << index << '\n';
        }
    } else {
	    constexpr auto inner = length - 1;
	    constexpr auto next = fastPow10<inner>;
	    // unlike the 64-bit version of this code, doing the 32-bit ints for 9 digit
	    // numbers (this code is not used when you request 64-bit numbers!)
	    // does not require as much optimization. We can walk through digit level
	    // by digit level (even if the digit does not contribute too much to the
	    // overall process!).
	    for (auto i = 2; i < 10; ++i) {
            body32<inner>(sum + i, product * i, index + (i * next));
	    }
    }
}

// 64-bit tweakables
/*
 * Should enable the use of futures
 */
constexpr auto useFutures = true;
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

/*
 * The depth at which to perform a hack to speed up computation using observed
 * behavior
 */
constexpr auto observedSpecializationDepth = 0; // don't use it at all

template<u64 length>
inline void body(std::ostream& stream, u64 sum = 0, u64 product = 1, u64 index = 0) noexcept {
	static_assert(length <= 19, "Can't have numbers over 19 digits on 64-bit numbers!");
    if constexpr (length == 0) {
        // specialization
        bool outputToStream = false;
        if constexpr (!expensiveChecks) {
            if constexpr (oddApprox && !evenApprox) {
                if ((sum % 3) != 0) {
                    return;
                }
            } else if constexpr (evenApprox && !oddApprox) {
                if ((sum & 1) != 0) {
                    return;
                }
            } else {
                if (((sum % 3) != 0) && ((sum & 1) != 0)) {
                    return;
                }
            }
        }
        if constexpr (exact) {
            outputToStream = ((index % product == 0) && (index % sum == 0));
        } else {
            outputToStream = (index % product == 0);
        }
        if (outputToStream) {
            stream << index << '\n';
        }
    } else {
	    //static_assert(length != 0, "Can't have length of zero!");
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
	    if constexpr (useFutures && length >= 11) {
	    	// when we are greater than 10 digit numbers, it is a smart idea to
	    	// perform divide and conquer at each level above 10 digits. The number of
	    	// threads used for computation is equal to: 2^(width - 10).
            auto fn = [](auto sum, auto product, auto index, auto p0, auto p1, auto p2) {
                std::ostringstream stream;
                body<inner>(stream, sum + p0, product * p0, index + (p0 * next));
                body<inner>(stream, sum + p1, product * p1, index + (p1 * next));
                body<inner>(stream, sum + p2, product * p2, index + (p2 * next));
                return stream.str();
            };
	    	auto lowerHalf = std::async(std::launch::async, fn, sum, product, index, 3, 4, 6);
	    	auto upperHalf = std::async(std::launch::async, fn, sum, product, index, 7, 8, 9);
	    	// perform computation on this primary thread because we want to be able
	    	// to maximize how much work we do and make the amount of work in each
	    	// thread even. The same number of threads are spawned but the primary
	    	// thread that spawned the children is reused below.
	    	body<inner>(stream, sum + 2, product * 2, index + (2 * next)); // 2
	    	stream << lowerHalf.get() << upperHalf.get();
        } else if constexpr (observedSpecializationDepth == length && length == 2) {
#include "Specialization2Digits.cc"
        } else if constexpr (observedSpecializationDepth == length && length == 3) {
#include "Specialization3Digits.cc"
        } else if constexpr (observedSpecializationDepth == length && length == 4) {
#include "Specialization4Digits.cc"
        } else if constexpr (observedSpecializationDepth == length && length == 5) {
#include "Specialization5Digits.cc"
        } else if constexpr (observedSpecializationDepth == length && length == 6) {
#include "Specialization6Digits.cc"
        } else if constexpr (observedSpecializationDepth == length && length == 7) {
#include "Specialization7Digits.cc"
        } else if constexpr (observedSpecializationDepth == length && length == 8) {
#include "Specialization8Digits.cc"
        } else if constexpr (observedSpecializationDepth == length && length == 9) {
#include "Specialization9Digits.cc"
        } else if constexpr (observedSpecializationDepth == length && length == 10) {
#include "Specialization10Digits.cc"
	    } else {
	    	// we use the stack to keep track of sums, products, and current indexes
	    	// instead of starting with a whole number and breaking it apart.
	    	// Walking down the call graph will cause another sum, product, and index
	    	// part to be added to the provided sum, product, and index.
	    	//
	    	// through observational evidence, I found that the least significant
	    	// digits are all even. Even if it turns out that this isn't the case
	    	// I can always perform the odd digit checks later on at a significant
	    	// reduction in speed cost!
            static constexpr auto incr = (length == 1) ? 2 : 1 ;
            for (auto i = 2; i < 10; i += incr) {
                if ( i != 5) {
                    body<inner>(stream, sum + i, product * i, index + (i * next));
                }
            }
	    }
    }
}



int main() {
	while(std::cin.good()) {
		u64 currentIndex = 0;
		std::cin >> currentIndex;
		if (std::cin.good()) {
			switch(currentIndex) {
				case 1: body32<1>(); break;
				case 2: body32<2>(); break;
				case 3: body32<3>(); break;
				case 4: body32<4>(); break;
				case 5: body32<5>(); break;
				case 6: body32<6>(); break;
				case 7: body32<7>(); break;
				case 8: body32<8>(); break;
				case 9: body32<9>(); break;
                case 10: body<10>(std::cout); break;
				case 11: body<11>(std::cout); break;
				case 12: body<12>(std::cout); break;
				case 13: body<13>(std::cout); break;
				case 14: body<14>(std::cout); break;
				case 15: body<15>(std::cout); break;
				case 16: body<16>(std::cout); break;
				case 17: body<17>(std::cout); break;
				case 18: body<18>(std::cout); break;
				case 19: body<19>(std::cout); break;
				default:
						 std::cerr << "Illegal index " << currentIndex << std::endl;
						 return 1;
			}
			std::cout << std::endl;
		}
	}
	return 0;
}
