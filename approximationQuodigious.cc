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

// Perform numeric quodigious checks, and get an approximate list of entries
#include "qlib.h"
#include <future>


// an odd discovery is that the depth variable improves performance even though it is only
// incremented and never directly used!
template<u64 length>
void innerBody(std::ostream& stream, u64 product, u64 index) noexcept;
template<u64 length>
void body(std::ostream& stream, u64 product = 1, u64 index = 0) noexcept {
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
	auto baseProduct = product;
    // we don't include the digits zero or 1 so just skip them by adding two
    // or the equivalent for the corresponding thing
	product <<= 1;
	index += (next << 1);
	if (length >= 11) {
        // when we are greater than 10 digit numbers, it is a smart idea to
        // perform divide and conquer at each level above 10 digits. The number of
        // threads used for computation is equal to: 2^(width - 10).
		auto lowerHalf = std::async(std::launch::async, [baseProduct](auto product, auto index) noexcept {
				std::ostringstream stream;
				innerBody<inner>(stream, product, index); // 3
				product += baseProduct;
				index += next;
				innerBody<inner>(stream, product, index); // 4
                product += (baseProduct << 1);
                index += (next << 1);
                innerBody<inner>(stream, product, index); // 6
				return stream.str();
				}, (product + baseProduct), (index + next));
		auto upperHalf = std::async(std::launch::async, [baseProduct](auto product, auto index) noexcept {
					std::ostringstream stream;
					innerBody<inner>(stream, product, index); // 7
					product += baseProduct;
					index += next;
					innerBody<inner>(stream, product, index); // 8
					product += baseProduct;
					index += next;
					innerBody<inner>(stream, product, index); // 9
					return stream.str();
				}, (product + ((baseProduct << 2) + baseProduct)), (index + ((next << 2) + next)));
        // perform computation on this primary thread because we want to be able
        // to maximize how much work we do and make the amount of work in each
        // thread even. The same number of threads are spawned but the primary
        // thread that spawned the children is reused below.
        innerBody<inner>(stream, product, index); // 2
		stream << lowerHalf.get() << upperHalf.get();
    } else {
        // hand unrolled loop bodies
        // we use the stack to keep track of products, and current indexes
        // instead of starting with a whole number and breaking it apart.
        // Walking down the call graph will cause another sum, product, and index
        // part to be added to the provided sum, product, and index.
        //
        // The upside of the break apart approach is that it is simple to understand
        // but is harder to improve performance for. When you're operating on the
        // digits of a number instead, the digits are already broken apart and the
        // need to use divides and modulus operations are minimized (although
        // a compiler could technically factor divides and mods into bit shifting)
        // this of this as a for loop from 2 to 10 skipping 5. Each
        // call in this block is as though the current digit is 2,
        // 3, 4, 6, 7, 8, or 9. We use addition only since multiplication
        // adds overhead and isn't really needed.
        // These are the sum, product, and index incrementations that
        // take place inbetween each call.
        //
        // Five is skipped except when length = 1, this will only invoke if
        // you actually pass one to this (which is impossible) since passing
        // 1 into the program will cause the 32-bit integer paths to be used
        // instead.
        innerBody<inner>(stream, product, index); // 2
        product += baseProduct;
        index += next;
        innerBody<inner>(stream, product, index); // 3
        product += baseProduct;
        index += next;
        innerBody<inner>(stream, product, index); // 4
        product += (baseProduct << 1);
        index += (next << 1);
        innerBody<inner>(stream, product, index); // 6
        product += baseProduct;
        index += next;
        innerBody<inner>(stream, product, index); // 7
        product += baseProduct;
        index += next;
        innerBody<inner>(stream, product, index); // 8
        product += baseProduct;
        index += next;
        innerBody<inner>(stream, product, index); // 9
    }
}


template<u64 length>
void innerBody(std::ostream& stream, u64 product, u64 index) noexcept {
    // this double template instantiation is done to make sure that the compiler
    // does not attempt to instantiate infinitely, if this code was in place
    // of the call to innerbody in body then the compiler would not stop
    // instiantiating. we can then also perform specialization on length zero
    body<length>(stream, product, index);
}


template<>
void innerBody<0>(std::ostream& stream, u64 product, u64 index) noexcept {
    if (componentQuodigious(index, product)) {
        stream << index << std::endl;
    }
}

template<>
void innerBody<1>(std::ostream& stream, u64 product, u64 index) noexcept {
    constexpr auto inner = 0;
    constexpr auto next = fastPow10<inner>;
    constexpr auto shiftNext = next << 1;
	auto baseProduct = (product << 1);
    // we don't include the digits zero or 1 so just skip them by adding two
    // or the equivalent for the corresponding thing
	product <<= 1;
	index += (shiftNext);
    // through observational evidence, I found that the least significant
    // digits are all even. Even if it turns out that this isn't the case
    // I can always perform the odd digit checks later on at a significant
    // reduction in speed cost!
    //
    //
    // The upside is that compilation time is reduced :D
    // it will also eliminate prime numbers :D
    innerBody<inner>(stream, product, index); // 2
    //product += baseProduct;
    //index += shiftNext;
    //innerBody<inner>(stream, product, index); // 4
    //product += baseProduct;
    //index += shiftNext;
    //innerBody<inner>(stream, product, index); // 6
    //product += baseProduct;
    //index += shiftNext;
    //innerBody<inner>(stream, product, index); // 8
}

template<u64 index>
inline void initialBody() noexcept {
    // we don't want main aware of any details of how computation is performed.
    // This allows the decoupling of details from main and the computation body itself
    body<index>(std::cout);
}



int main() {
    while(std::cin.good()) {
        u64 currentIndex = 0;
        std::cin >> currentIndex;
        if (std::cin.good()) {
            switch(currentIndex) {
                case 12: initialBody<12>(); break;
                case 13: initialBody<13>(); break;
                case 14: initialBody<14>(); break;
                //case 15: initialBody<15>(); break;
                //case 16: initialBody<16>(); break;
                //case 17: initialBody<17>(); break;
                //case 18: initialBody<18>(); break;
                //case 19: initialBody<19>(); break;
                default:
                        std::cerr << "Illegal index " << currentIndex << std::endl;
                        return 1;
            }
            std::cout << std::endl;
        }
    }
    return 0;
}
