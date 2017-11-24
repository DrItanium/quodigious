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


// an odd discovery is that the depth variable improves performance even though it is only
// incremented and never directly used!
template<u64 length>
inline void innerBody(std::ostream& stream, u64 sum, u64 product, u64 index, u64 depth) noexcept;
template<u64 length>
inline void body(std::ostream& stream, u64 sum = 0, u64 product = 1, u64 index = 0, u64 depth = 0) noexcept {
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
	sum += 2;
	product <<= 1;
	index += (next << 1);
	if (length >= 11) {
		// when we are greater than 10 digit numbers, it is a smart idea to
		// perform divide and conquer at each level above 10 digits. The number of
		// threads used for computation is equal to: 2^(width - 10).
		auto lowerHalf = std::async(std::launch::async, [baseProduct](auto sum, auto product, auto index, auto depth) noexcept {
				std::ostringstream stream;
				++sum;
				product += baseProduct;
				index += next;
				innerBody<inner>(stream, sum, product, index, depth); // 3
				++sum;
				product += baseProduct;
				index += next;
				innerBody<inner>(stream, sum, product, index, depth); // 4
				sum += 2;
				product += (baseProduct << 1);
				index += (next << 1);
				innerBody<inner>(stream, sum, product, index, depth); // 6
				return stream.str();
				}, sum, product, index, depth);
		auto upperHalf = std::async(std::launch::async, [baseProduct](auto sum, auto product, auto index, auto depth) noexcept {
				std::ostringstream stream;
				sum += 5;
				product += ((baseProduct << 2) + baseProduct);
				index += ((next << 2) + next);
				innerBody<inner>(stream, sum, product, index, depth); // 7
				++sum;
				product += baseProduct;
				index += next;
				innerBody<inner>(stream, sum, product, index, depth); // 8
				++sum;
				product += baseProduct;
				index += next;
				innerBody<inner>(stream, sum, product, index, depth); // 9
				return stream.str();
				}, sum, product, index, depth);
		// perform computation on this primary thread because we want to be able
		// to maximize how much work we do and make the amount of work in each
		// thread even. The same number of threads are spawned but the primary
		// thread that spawned the children is reused below.
		innerBody<inner>(stream, sum, product, index, depth); // 2
		stream << lowerHalf.get() << upperHalf.get();
	} else {
		// hand unrolled loop bodies
		// we use the stack to keep track of sums, products, and current indexes
		// instead of starting with a whole number and breaking it apart.
		// Walking down the call graph will cause another sum, product, and index
		// part to be added to the provided sum, product, and index.
		//
		// The upside of the break apart approach is that it is simple to understand
		// but is harder to improve performance for. When you're operating on the
		// digits of a number instead, the digits are already broken apart and the
		// need to use divides and modulus operations are minimized (although
		// a compiler could technically factor divides and mods into bit shifting)
		if (length == 1) {
			// through observational evidence, I found that the least significant
			// digits are all even. Even if it turns out that this isn't the case
			// I can always perform the odd digit checks later on at a significant
			// reduction in speed cost!
			//
			//
			// The upside is that compilation time is reduced :D
			// it will also eliminate prime numbers :D
			innerBody<inner>(stream, sum, product,index, depth); // 2
			sum += 2;
			product += (baseProduct << 1);
			index += (next << 1);
			innerBody<inner>(stream, sum, product, index, depth); // 4
			sum += 2;
			product += (baseProduct << 1);
			index += (next << 1);
			innerBody<inner>(stream, sum, product,index, depth); // 6
			sum += 2;
			product += (baseProduct << 1);
			index += (next << 1);
			innerBody<inner>(stream, sum, product,index, depth); // 8
		} else {
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
			innerBody<inner>(stream, sum, product, index, depth); // 2
			++sum;
			product += baseProduct;
			index += next;
			innerBody<inner>(stream, sum, product, index, depth); // 3
			++sum;
			product += baseProduct;
			index += next;
			innerBody<inner>(stream, sum, product, index, depth); // 4
			sum += 2;
			product += (baseProduct << 1);
			index += (next << 1);
			innerBody<inner>(stream, sum, product, index, depth); // 6
			++sum;
			product += baseProduct;
			index += next;
			innerBody<inner>(stream, sum, product, index, depth); // 7
			++sum;
			product += baseProduct;
			index += next;
			innerBody<inner>(stream, sum, product, index, depth); // 8
			++sum;
			product += baseProduct;
			index += next;
			innerBody<inner>(stream, sum, product, index, depth); // 9
		}
	}
}


template<u64 length>
inline void innerBody(std::ostream& stream, u64 sum, u64 product, u64 index, u64 depth) noexcept {
	// this double template instantiation is done to make sure that the compiler
	// does not attempt to instantiate infinitely, if this code was in place
	// of the call to innerbody in body then the compiler would not stop
	// instiantiating. we can then also perform specialization on length zero
	body<length>(stream, sum, product, index, depth + 1);
}
template<>
inline void innerBody<0>(std::ostream& stream, u64 sum, u64 product, u64 index, u64 depth) noexcept {
	// specialization
	if (isQuodigious(index, sum, product)) {
#ifndef EXTENDED_RESEARCH
		stream << index << std::endl;
#else /* EXTENDED_RESEARCH */
		stream << "** " << index << " @@ " << product << " (+ (* 3 " << (product / 3) << ") " << (product % 3)  << ") $$ " << sum << " (* 3 " << (sum / 3) << ")" << std::endl;
#endif // end !EXTENDED_RESEARCH
	}
}

template<>
inline void innerBody<2>(std::ostream& stream, u64 sum, u64 product, u64 index, u64 depth) noexcept {
	// try out the different combinations

	++depth;
	// did some research and found that all quodigious numbers only match up
	// with a subset of the total search space at this level thus we can
	// disable quite a few, the ones commented out are the ones which do not
	// contribute for finding quodigious numbers between 11 and 17 digits
	//innerBody<0>(stream, sum + 2 + 2, product << 2, index + 22, depth);
	innerBody<0>(stream, sum + 2 + 4, product << 3, index + 24, depth);
	//innerBody<0>(stream, sum + 2 + 6, product * 2 * 6, index + 26, depth);
	innerBody<0>(stream, sum + 2 + 8, product << 4, index + 28, depth);
	innerBody<0>(stream, sum + 3 + 2, product * 3 * 2, index + 32, depth);
	//innerBody<0>(stream, sum + 3 + 4, product * 3 * 4, index + 34, depth);
	innerBody<0>(stream, sum + 3 + 6, product * 3 * 6, index + 36, depth);
	//innerBody<0>(stream, sum + 3 + 8, product * 3 * 8, index + 38, depth);
	//innerBody<0>(stream, sum + 4 + 2, product << 3, index + 42, depth);
	innerBody<0>(stream, sum + 4 + 4, product << 4, index + 44, depth);
	//innerBody<0>(stream, sum + 4 + 6, product * 4 * 6, index + 46, depth);
	innerBody<0>(stream, sum + 4 + 8, product << 5, index + 48, depth);
	//innerBody<0>(stream, sum + 6 + 2, product * 6 * 2, index + 62, depth);
	innerBody<0>(stream, sum + 6 + 4, product * 6 * 4, index + 64, depth);
	//innerBody<0>(stream, sum + 6 + 6, product * 6 * 6, index + 66, depth);
	innerBody<0>(stream, sum + 6 + 8, product * 6 * 8, index + 68, depth);
	innerBody<0>(stream, sum + 7 + 2, product * 7 * 2, index + 72, depth);
	//innerBody<0>(stream, sum + 7 + 4, product * 7 * 4, index + 74, depth);
	innerBody<0>(stream, sum + 7 + 6, product * 7 * 6, index + 76, depth);
	//innerBody<0>(stream, sum + 7 + 8, product * 7 * 8, index + 78, depth);
	//innerBody<0>(stream, sum + 8 + 2, product << 4, index + 82, depth);
	innerBody<0>(stream, sum + 8 + 4, product << 5, index + 84, depth);
	//innerBody<0>(stream, sum + 8 + 6, product * 8 * 6, index + 86, depth);
	innerBody<0>(stream, sum + 8 + 8, product << 6, index + 88, depth);
	innerBody<0>(stream, sum + 9 + 2, product * 9 * 2, index + 92, depth);
	//innerBody<0>(stream, sum + 9 + 4, product * 9 * 4, index + 94, depth);
	innerBody<0>(stream, sum + 9 + 6, product * 9 * 6, index + 96, depth);
	//innerBody<0>(stream, sum + 9 + 8, product * 9 * 8, index + 98, depth);
}

template<>
inline void innerBody<3>(std::ostream& stream, u64 sum, u64 product, u64 index, u64 depth) noexcept {
	++depth;
	// did some research and found that all quodigious numbers only match up
	// with a subset of the total search space at this level thus we can
	// disable quite a few, the ones commented out are the ones which do not
	// contribute for finding quodigious numbers between 11 and 17 digits
	//
	// Generated with the unique3.clp expert system

	innerBody<0>(stream, sum + 2 + 2 + 4, product * 2 * 2 * 4, index + 224, depth);
	innerBody<0>(stream, sum + 2 + 3 + 2, product * 2 * 3 * 2, index + 232, depth);
	innerBody<0>(stream, sum + 2 + 4 + 8, product * 2 * 4 * 8, index + 248, depth);
	innerBody<0>(stream, sum + 2 + 6 + 4, product * 2 * 6 * 4, index + 264, depth);
	innerBody<0>(stream, sum + 2 + 7 + 2, product * 2 * 7 * 2, index + 272, depth);
	innerBody<0>(stream, sum + 2 + 8 + 8, product * 2 * 8 * 8, index + 288, depth);
	innerBody<0>(stream, sum + 2 + 9 + 6, product * 2 * 9 * 6, index + 296, depth);
	innerBody<0>(stream, sum + 3 + 2 + 8, product * 3 * 2 * 8, index + 328, depth);
	innerBody<0>(stream, sum + 3 + 3 + 6, product * 3 * 3 * 6, index + 336, depth);
	innerBody<0>(stream, sum + 3 + 4 + 4, product * 3 * 4 * 4, index + 344, depth);
	innerBody<0>(stream, sum + 3 + 6 + 8, product * 3 * 6 * 8, index + 368, depth);
	innerBody<0>(stream, sum + 3 + 7 + 6, product * 3 * 7 * 6, index + 376, depth);
	innerBody<0>(stream, sum + 3 + 8 + 4, product * 3 * 8 * 4, index + 384, depth);
	innerBody<0>(stream, sum + 3 + 9 + 2, product * 3 * 9 * 2, index + 392, depth);
	innerBody<0>(stream, sum + 4 + 2 + 4, product * 4 * 2 * 4, index + 424, depth);
	innerBody<0>(stream, sum + 4 + 3 + 2, product * 4 * 3 * 2, index + 432, depth);
	innerBody<0>(stream, sum + 4 + 4 + 8, product * 4 * 4 * 8, index + 448, depth);
	innerBody<0>(stream, sum + 4 + 6 + 4, product * 4 * 6 * 4, index + 464, depth);
	innerBody<0>(stream, sum + 4 + 7 + 2, product * 4 * 7 * 2, index + 472, depth);
	innerBody<0>(stream, sum + 4 + 8 + 8, product * 4 * 8 * 8, index + 488, depth);
	innerBody<0>(stream, sum + 4 + 9 + 6, product * 4 * 9 * 6, index + 496, depth);
	innerBody<0>(stream, sum + 6 + 2 + 4, product * 6 * 2 * 4, index + 624, depth);
	innerBody<0>(stream, sum + 6 + 3 + 2, product * 6 * 3 * 2, index + 632, depth);
	innerBody<0>(stream, sum + 6 + 4 + 8, product * 6 * 4 * 8, index + 648, depth);
	innerBody<0>(stream, sum + 6 + 6 + 4, product * 6 * 6 * 4, index + 664, depth);
	innerBody<0>(stream, sum + 6 + 7 + 2, product * 6 * 7 * 2, index + 672, depth);
	innerBody<0>(stream, sum + 6 + 8 + 8, product * 6 * 8 * 8, index + 688, depth);
	innerBody<0>(stream, sum + 6 + 9 + 6, product * 6 * 9 * 6, index + 696, depth);
	innerBody<0>(stream, sum + 7 + 2 + 8, product * 7 * 2 * 8, index + 728, depth);
	innerBody<0>(stream, sum + 7 + 3 + 6, product * 7 * 3 * 6, index + 736, depth);
	innerBody<0>(stream, sum + 7 + 4 + 4, product * 7 * 4 * 4, index + 744, depth);
	innerBody<0>(stream, sum + 7 + 6 + 8, product * 7 * 6 * 8, index + 768, depth);
	innerBody<0>(stream, sum + 7 + 7 + 6, product * 7 * 7 * 6, index + 776, depth);
	innerBody<0>(stream, sum + 7 + 8 + 4, product * 7 * 8 * 4, index + 784, depth);
	innerBody<0>(stream, sum + 7 + 9 + 2, product * 7 * 9 * 2, index + 792, depth);
	innerBody<0>(stream, sum + 8 + 2 + 4, product * 8 * 2 * 4, index + 824, depth);
	innerBody<0>(stream, sum + 8 + 3 + 2, product * 8 * 3 * 2, index + 832, depth);
	innerBody<0>(stream, sum + 8 + 4 + 8, product * 8 * 4 * 8, index + 848, depth);
	innerBody<0>(stream, sum + 8 + 6 + 4, product * 8 * 6 * 4, index + 864, depth);
	innerBody<0>(stream, sum + 8 + 7 + 2, product * 8 * 7 * 2, index + 872, depth);
	innerBody<0>(stream, sum + 8 + 8 + 8, product * 8 * 8 * 8, index + 888, depth);
	innerBody<0>(stream, sum + 8 + 9 + 6, product * 8 * 9 * 6, index + 896, depth);
	innerBody<0>(stream, sum + 9 + 2 + 8, product * 9 * 2 * 8, index + 928, depth);
	innerBody<0>(stream, sum + 9 + 3 + 6, product * 9 * 3 * 6, index + 936, depth);
	innerBody<0>(stream, sum + 9 + 4 + 4, product * 9 * 4 * 4, index + 944, depth);
	innerBody<0>(stream, sum + 9 + 6 + 8, product * 9 * 6 * 8, index + 968, depth);
	innerBody<0>(stream, sum + 9 + 7 + 6, product * 9 * 7 * 6, index + 976, depth);
	innerBody<0>(stream, sum + 9 + 8 + 4, product * 9 * 8 * 4, index + 984, depth);
	innerBody<0>(stream, sum + 9 + 9 + 2, product * 9 * 9 * 2, index + 992, depth);
}
template<>
inline void innerBody<4>(std::ostream& stream, u64 sum, u64 product, u64 index, u64 depth) noexcept {
	++depth;
	// did some research and found that all quodigious numbers only match up
	// with a subset of the total search space at this level thus we can
	// disable quite a few, the ones commented out are the ones which do not
	// contribute for finding quodigious numbers between 11 and 17 digits
	//
	// Generated with the unique4.clp expert system
	innerBody<0>(stream, sum + 2 + 2 + 2 + 4, product * 2 * 2 * 2 * 4, index + 2224, depth);
	innerBody<0>(stream, sum + 2 + 2 + 7 + 2, product * 2 * 2 * 7 * 2, index + 2272, depth);
	innerBody<0>(stream, sum + 2 + 2 + 8 + 8, product * 2 * 2 * 8 * 8, index + 2288, depth);
	innerBody<0>(stream, sum + 2 + 3 + 3 + 6, product * 2 * 3 * 3 * 6, index + 2336, depth);
	innerBody<0>(stream, sum + 2 + 3 + 6 + 8, product * 2 * 3 * 6 * 8, index + 2368, depth);
	innerBody<0>(stream, sum + 2 + 3 + 8 + 4, product * 2 * 3 * 8 * 4, index + 2384, depth);
	innerBody<0>(stream, sum + 2 + 4 + 3 + 2, product * 2 * 4 * 3 * 2, index + 2432, depth);
	innerBody<0>(stream, sum + 2 + 4 + 4 + 8, product * 2 * 4 * 4 * 8, index + 2448, depth);
	innerBody<0>(stream, sum + 2 + 4 + 6 + 4, product * 2 * 4 * 6 * 4, index + 2464, depth);
	innerBody<0>(stream, sum + 2 + 4 + 9 + 6, product * 2 * 4 * 9 * 6, index + 2496, depth);
	innerBody<0>(stream, sum + 2 + 6 + 2 + 4, product * 2 * 6 * 2 * 4, index + 2624, depth);
	innerBody<0>(stream, sum + 2 + 6 + 7 + 2, product * 2 * 6 * 7 * 2, index + 2672, depth);
	innerBody<0>(stream, sum + 2 + 6 + 8 + 8, product * 2 * 6 * 8 * 8, index + 2688, depth);
	innerBody<0>(stream, sum + 2 + 7 + 3 + 6, product * 2 * 7 * 3 * 6, index + 2736, depth);
	innerBody<0>(stream, sum + 2 + 7 + 6 + 8, product * 2 * 7 * 6 * 8, index + 2768, depth);
	innerBody<0>(stream, sum + 2 + 7 + 8 + 4, product * 2 * 7 * 8 * 4, index + 2784, depth);
	innerBody<0>(stream, sum + 2 + 8 + 3 + 2, product * 2 * 8 * 3 * 2, index + 2832, depth);
	innerBody<0>(stream, sum + 2 + 8 + 4 + 8, product * 2 * 8 * 4 * 8, index + 2848, depth);
	innerBody<0>(stream, sum + 2 + 8 + 6 + 4, product * 2 * 8 * 6 * 4, index + 2864, depth);
	innerBody<0>(stream, sum + 2 + 8 + 9 + 6, product * 2 * 8 * 9 * 6, index + 2896, depth);
	innerBody<0>(stream, sum + 2 + 9 + 2 + 8, product * 2 * 9 * 2 * 8, index + 2928, depth);
	innerBody<0>(stream, sum + 2 + 9 + 4 + 4, product * 2 * 9 * 4 * 4, index + 2944, depth);
	innerBody<0>(stream, sum + 2 + 9 + 7 + 6, product * 2 * 9 * 7 * 6, index + 2976, depth);
	innerBody<0>(stream, sum + 2 + 9 + 9 + 2, product * 2 * 9 * 9 * 2, index + 2992, depth);
	innerBody<0>(stream, sum + 3 + 2 + 3 + 2, product * 3 * 2 * 3 * 2, index + 3232, depth);
	innerBody<0>(stream, sum + 3 + 2 + 4 + 8, product * 3 * 2 * 4 * 8, index + 3248, depth);
	innerBody<0>(stream, sum + 3 + 2 + 6 + 4, product * 3 * 2 * 6 * 4, index + 3264, depth);
	innerBody<0>(stream, sum + 3 + 2 + 9 + 6, product * 3 * 2 * 9 * 6, index + 3296, depth);
	innerBody<0>(stream, sum + 3 + 3 + 2 + 8, product * 3 * 3 * 2 * 8, index + 3328, depth);
	innerBody<0>(stream, sum + 3 + 3 + 4 + 4, product * 3 * 3 * 4 * 4, index + 3344, depth);
	innerBody<0>(stream, sum + 3 + 3 + 7 + 6, product * 3 * 3 * 7 * 6, index + 3376, depth);
	innerBody<0>(stream, sum + 3 + 3 + 9 + 2, product * 3 * 3 * 9 * 2, index + 3392, depth);
	innerBody<0>(stream, sum + 3 + 4 + 2 + 4, product * 3 * 4 * 2 * 4, index + 3424, depth);
	innerBody<0>(stream, sum + 3 + 4 + 7 + 2, product * 3 * 4 * 7 * 2, index + 3472, depth);
	innerBody<0>(stream, sum + 3 + 4 + 8 + 8, product * 3 * 4 * 8 * 8, index + 3488, depth);
	innerBody<0>(stream, sum + 3 + 6 + 3 + 2, product * 3 * 6 * 3 * 2, index + 3632, depth);
	innerBody<0>(stream, sum + 3 + 6 + 4 + 8, product * 3 * 6 * 4 * 8, index + 3648, depth);
	innerBody<0>(stream, sum + 3 + 6 + 6 + 4, product * 3 * 6 * 6 * 4, index + 3664, depth);
	innerBody<0>(stream, sum + 3 + 6 + 9 + 6, product * 3 * 6 * 9 * 6, index + 3696, depth);
	innerBody<0>(stream, sum + 3 + 7 + 2 + 8, product * 3 * 7 * 2 * 8, index + 3728, depth);
	innerBody<0>(stream, sum + 3 + 7 + 4 + 4, product * 3 * 7 * 4 * 4, index + 3744, depth);
	innerBody<0>(stream, sum + 3 + 7 + 7 + 6, product * 3 * 7 * 7 * 6, index + 3776, depth);
	innerBody<0>(stream, sum + 3 + 7 + 9 + 2, product * 3 * 7 * 9 * 2, index + 3792, depth);
	innerBody<0>(stream, sum + 3 + 8 + 2 + 4, product * 3 * 8 * 2 * 4, index + 3824, depth);
	innerBody<0>(stream, sum + 3 + 8 + 7 + 2, product * 3 * 8 * 7 * 2, index + 3872, depth);
	innerBody<0>(stream, sum + 3 + 8 + 8 + 8, product * 3 * 8 * 8 * 8, index + 3888, depth);
	innerBody<0>(stream, sum + 3 + 9 + 3 + 6, product * 3 * 9 * 3 * 6, index + 3936, depth);
	innerBody<0>(stream, sum + 3 + 9 + 6 + 8, product * 3 * 9 * 6 * 8, index + 3968, depth);
	innerBody<0>(stream, sum + 3 + 9 + 8 + 4, product * 3 * 9 * 8 * 4, index + 3984, depth);
	innerBody<0>(stream, sum + 4 + 2 + 2 + 4, product * 4 * 2 * 2 * 4, index + 4224, depth);
	innerBody<0>(stream, sum + 4 + 2 + 7 + 2, product * 4 * 2 * 7 * 2, index + 4272, depth);
	innerBody<0>(stream, sum + 4 + 2 + 8 + 8, product * 4 * 2 * 8 * 8, index + 4288, depth);
	innerBody<0>(stream, sum + 4 + 3 + 3 + 6, product * 4 * 3 * 3 * 6, index + 4336, depth);
	innerBody<0>(stream, sum + 4 + 3 + 6 + 8, product * 4 * 3 * 6 * 8, index + 4368, depth);
	innerBody<0>(stream, sum + 4 + 3 + 8 + 4, product * 4 * 3 * 8 * 4, index + 4384, depth);
	innerBody<0>(stream, sum + 4 + 4 + 3 + 2, product * 4 * 4 * 3 * 2, index + 4432, depth);
	innerBody<0>(stream, sum + 4 + 4 + 4 + 8, product * 4 * 4 * 4 * 8, index + 4448, depth);
	innerBody<0>(stream, sum + 4 + 4 + 6 + 4, product * 4 * 4 * 6 * 4, index + 4464, depth);
	innerBody<0>(stream, sum + 4 + 4 + 9 + 6, product * 4 * 4 * 9 * 6, index + 4496, depth);
	innerBody<0>(stream, sum + 4 + 6 + 2 + 4, product * 4 * 6 * 2 * 4, index + 4624, depth);
	innerBody<0>(stream, sum + 4 + 6 + 7 + 2, product * 4 * 6 * 7 * 2, index + 4672, depth);
	innerBody<0>(stream, sum + 4 + 6 + 8 + 8, product * 4 * 6 * 8 * 8, index + 4688, depth);
	innerBody<0>(stream, sum + 4 + 7 + 3 + 6, product * 4 * 7 * 3 * 6, index + 4736, depth);
	innerBody<0>(stream, sum + 4 + 7 + 6 + 8, product * 4 * 7 * 6 * 8, index + 4768, depth);
	innerBody<0>(stream, sum + 4 + 7 + 8 + 4, product * 4 * 7 * 8 * 4, index + 4784, depth);
	innerBody<0>(stream, sum + 4 + 8 + 3 + 2, product * 4 * 8 * 3 * 2, index + 4832, depth);
	innerBody<0>(stream, sum + 4 + 8 + 4 + 8, product * 4 * 8 * 4 * 8, index + 4848, depth);
	innerBody<0>(stream, sum + 4 + 8 + 6 + 4, product * 4 * 8 * 6 * 4, index + 4864, depth);
	innerBody<0>(stream, sum + 4 + 8 + 9 + 6, product * 4 * 8 * 9 * 6, index + 4896, depth);
	innerBody<0>(stream, sum + 4 + 9 + 2 + 8, product * 4 * 9 * 2 * 8, index + 4928, depth);
	innerBody<0>(stream, sum + 4 + 9 + 4 + 4, product * 4 * 9 * 4 * 4, index + 4944, depth);
	innerBody<0>(stream, sum + 4 + 9 + 7 + 6, product * 4 * 9 * 7 * 6, index + 4976, depth);
	innerBody<0>(stream, sum + 4 + 9 + 9 + 2, product * 4 * 9 * 9 * 2, index + 4992, depth);
	innerBody<0>(stream, sum + 6 + 2 + 2 + 4, product * 6 * 2 * 2 * 4, index + 6224, depth);
	innerBody<0>(stream, sum + 6 + 2 + 7 + 2, product * 6 * 2 * 7 * 2, index + 6272, depth);
	innerBody<0>(stream, sum + 6 + 2 + 8 + 8, product * 6 * 2 * 8 * 8, index + 6288, depth);
	innerBody<0>(stream, sum + 6 + 3 + 3 + 6, product * 6 * 3 * 3 * 6, index + 6336, depth);
	innerBody<0>(stream, sum + 6 + 3 + 6 + 8, product * 6 * 3 * 6 * 8, index + 6368, depth);
	innerBody<0>(stream, sum + 6 + 3 + 8 + 4, product * 6 * 3 * 8 * 4, index + 6384, depth);
	innerBody<0>(stream, sum + 6 + 4 + 3 + 2, product * 6 * 4 * 3 * 2, index + 6432, depth);
	innerBody<0>(stream, sum + 6 + 4 + 4 + 8, product * 6 * 4 * 4 * 8, index + 6448, depth);
	innerBody<0>(stream, sum + 6 + 4 + 6 + 4, product * 6 * 4 * 6 * 4, index + 6464, depth);
	innerBody<0>(stream, sum + 6 + 4 + 9 + 6, product * 6 * 4 * 9 * 6, index + 6496, depth);
	innerBody<0>(stream, sum + 6 + 6 + 2 + 4, product * 6 * 6 * 2 * 4, index + 6624, depth);
	innerBody<0>(stream, sum + 6 + 6 + 7 + 2, product * 6 * 6 * 7 * 2, index + 6672, depth);
	innerBody<0>(stream, sum + 6 + 6 + 8 + 8, product * 6 * 6 * 8 * 8, index + 6688, depth);
	innerBody<0>(stream, sum + 6 + 7 + 3 + 6, product * 6 * 7 * 3 * 6, index + 6736, depth);
	innerBody<0>(stream, sum + 6 + 7 + 6 + 8, product * 6 * 7 * 6 * 8, index + 6768, depth);
	innerBody<0>(stream, sum + 6 + 7 + 8 + 4, product * 6 * 7 * 8 * 4, index + 6784, depth);
	innerBody<0>(stream, sum + 6 + 8 + 3 + 2, product * 6 * 8 * 3 * 2, index + 6832, depth);
	innerBody<0>(stream, sum + 6 + 8 + 4 + 8, product * 6 * 8 * 4 * 8, index + 6848, depth);
	innerBody<0>(stream, sum + 6 + 8 + 6 + 4, product * 6 * 8 * 6 * 4, index + 6864, depth);
	innerBody<0>(stream, sum + 6 + 8 + 9 + 6, product * 6 * 8 * 9 * 6, index + 6896, depth);
	innerBody<0>(stream, sum + 6 + 9 + 2 + 8, product * 6 * 9 * 2 * 8, index + 6928, depth);
	innerBody<0>(stream, sum + 6 + 9 + 4 + 4, product * 6 * 9 * 4 * 4, index + 6944, depth);
	innerBody<0>(stream, sum + 6 + 9 + 7 + 6, product * 6 * 9 * 7 * 6, index + 6976, depth);
	innerBody<0>(stream, sum + 6 + 9 + 9 + 2, product * 6 * 9 * 9 * 2, index + 6992, depth);
	innerBody<0>(stream, sum + 7 + 2 + 3 + 2, product * 7 * 2 * 3 * 2, index + 7232, depth);
	innerBody<0>(stream, sum + 7 + 2 + 4 + 8, product * 7 * 2 * 4 * 8, index + 7248, depth);
	innerBody<0>(stream, sum + 7 + 2 + 6 + 4, product * 7 * 2 * 6 * 4, index + 7264, depth);
	innerBody<0>(stream, sum + 7 + 2 + 9 + 6, product * 7 * 2 * 9 * 6, index + 7296, depth);
	innerBody<0>(stream, sum + 7 + 3 + 2 + 8, product * 7 * 3 * 2 * 8, index + 7328, depth);
	innerBody<0>(stream, sum + 7 + 3 + 4 + 4, product * 7 * 3 * 4 * 4, index + 7344, depth);
	innerBody<0>(stream, sum + 7 + 3 + 7 + 6, product * 7 * 3 * 7 * 6, index + 7376, depth);
	innerBody<0>(stream, sum + 7 + 3 + 9 + 2, product * 7 * 3 * 9 * 2, index + 7392, depth);
	innerBody<0>(stream, sum + 7 + 4 + 2 + 4, product * 7 * 4 * 2 * 4, index + 7424, depth);
	innerBody<0>(stream, sum + 7 + 4 + 7 + 2, product * 7 * 4 * 7 * 2, index + 7472, depth);
	innerBody<0>(stream, sum + 7 + 4 + 8 + 8, product * 7 * 4 * 8 * 8, index + 7488, depth);
	innerBody<0>(stream, sum + 7 + 6 + 3 + 2, product * 7 * 6 * 3 * 2, index + 7632, depth);
	innerBody<0>(stream, sum + 7 + 6 + 4 + 8, product * 7 * 6 * 4 * 8, index + 7648, depth);
	innerBody<0>(stream, sum + 7 + 6 + 6 + 4, product * 7 * 6 * 6 * 4, index + 7664, depth);
	innerBody<0>(stream, sum + 7 + 6 + 9 + 6, product * 7 * 6 * 9 * 6, index + 7696, depth);
	innerBody<0>(stream, sum + 7 + 7 + 2 + 8, product * 7 * 7 * 2 * 8, index + 7728, depth);
	innerBody<0>(stream, sum + 7 + 7 + 4 + 4, product * 7 * 7 * 4 * 4, index + 7744, depth);
	innerBody<0>(stream, sum + 7 + 7 + 7 + 6, product * 7 * 7 * 7 * 6, index + 7776, depth);
	innerBody<0>(stream, sum + 7 + 7 + 9 + 2, product * 7 * 7 * 9 * 2, index + 7792, depth);
	innerBody<0>(stream, sum + 7 + 8 + 2 + 4, product * 7 * 8 * 2 * 4, index + 7824, depth);
	innerBody<0>(stream, sum + 7 + 8 + 7 + 2, product * 7 * 8 * 7 * 2, index + 7872, depth);
	innerBody<0>(stream, sum + 7 + 8 + 8 + 8, product * 7 * 8 * 8 * 8, index + 7888, depth);
	innerBody<0>(stream, sum + 7 + 9 + 3 + 6, product * 7 * 9 * 3 * 6, index + 7936, depth);
	innerBody<0>(stream, sum + 7 + 9 + 6 + 8, product * 7 * 9 * 6 * 8, index + 7968, depth);
	innerBody<0>(stream, sum + 7 + 9 + 8 + 4, product * 7 * 9 * 8 * 4, index + 7984, depth);
	innerBody<0>(stream, sum + 8 + 2 + 2 + 4, product * 8 * 2 * 2 * 4, index + 8224, depth);
	innerBody<0>(stream, sum + 8 + 2 + 7 + 2, product * 8 * 2 * 7 * 2, index + 8272, depth);
	innerBody<0>(stream, sum + 8 + 2 + 8 + 8, product * 8 * 2 * 8 * 8, index + 8288, depth);
	innerBody<0>(stream, sum + 8 + 3 + 3 + 6, product * 8 * 3 * 3 * 6, index + 8336, depth);
	innerBody<0>(stream, sum + 8 + 3 + 6 + 8, product * 8 * 3 * 6 * 8, index + 8368, depth);
	innerBody<0>(stream, sum + 8 + 3 + 8 + 4, product * 8 * 3 * 8 * 4, index + 8384, depth);
	innerBody<0>(stream, sum + 8 + 4 + 3 + 2, product * 8 * 4 * 3 * 2, index + 8432, depth);
	innerBody<0>(stream, sum + 8 + 4 + 4 + 8, product * 8 * 4 * 4 * 8, index + 8448, depth);
	innerBody<0>(stream, sum + 8 + 4 + 6 + 4, product * 8 * 4 * 6 * 4, index + 8464, depth);
	innerBody<0>(stream, sum + 8 + 4 + 9 + 6, product * 8 * 4 * 9 * 6, index + 8496, depth);
	innerBody<0>(stream, sum + 8 + 6 + 2 + 4, product * 8 * 6 * 2 * 4, index + 8624, depth);
	innerBody<0>(stream, sum + 8 + 6 + 7 + 2, product * 8 * 6 * 7 * 2, index + 8672, depth);
	innerBody<0>(stream, sum + 8 + 6 + 8 + 8, product * 8 * 6 * 8 * 8, index + 8688, depth);
	innerBody<0>(stream, sum + 8 + 7 + 3 + 6, product * 8 * 7 * 3 * 6, index + 8736, depth);
	innerBody<0>(stream, sum + 8 + 7 + 6 + 8, product * 8 * 7 * 6 * 8, index + 8768, depth);
	innerBody<0>(stream, sum + 8 + 7 + 8 + 4, product * 8 * 7 * 8 * 4, index + 8784, depth);
	innerBody<0>(stream, sum + 8 + 8 + 3 + 2, product * 8 * 8 * 3 * 2, index + 8832, depth);
	innerBody<0>(stream, sum + 8 + 8 + 4 + 8, product * 8 * 8 * 4 * 8, index + 8848, depth);
	innerBody<0>(stream, sum + 8 + 8 + 6 + 4, product * 8 * 8 * 6 * 4, index + 8864, depth);
	innerBody<0>(stream, sum + 8 + 8 + 9 + 6, product * 8 * 8 * 9 * 6, index + 8896, depth);
	innerBody<0>(stream, sum + 8 + 9 + 2 + 8, product * 8 * 9 * 2 * 8, index + 8928, depth);
	innerBody<0>(stream, sum + 8 + 9 + 4 + 4, product * 8 * 9 * 4 * 4, index + 8944, depth);
	innerBody<0>(stream, sum + 8 + 9 + 7 + 6, product * 8 * 9 * 7 * 6, index + 8976, depth);
	innerBody<0>(stream, sum + 8 + 9 + 9 + 2, product * 8 * 9 * 9 * 2, index + 8992, depth);
	innerBody<0>(stream, sum + 9 + 2 + 3 + 2, product * 9 * 2 * 3 * 2, index + 9232, depth);
	innerBody<0>(stream, sum + 9 + 2 + 4 + 8, product * 9 * 2 * 4 * 8, index + 9248, depth);
	innerBody<0>(stream, sum + 9 + 2 + 6 + 4, product * 9 * 2 * 6 * 4, index + 9264, depth);
	innerBody<0>(stream, sum + 9 + 2 + 9 + 6, product * 9 * 2 * 9 * 6, index + 9296, depth);
	innerBody<0>(stream, sum + 9 + 3 + 2 + 8, product * 9 * 3 * 2 * 8, index + 9328, depth);
	innerBody<0>(stream, sum + 9 + 3 + 4 + 4, product * 9 * 3 * 4 * 4, index + 9344, depth);
	innerBody<0>(stream, sum + 9 + 3 + 7 + 6, product * 9 * 3 * 7 * 6, index + 9376, depth);
	innerBody<0>(stream, sum + 9 + 3 + 9 + 2, product * 9 * 3 * 9 * 2, index + 9392, depth);
	innerBody<0>(stream, sum + 9 + 4 + 2 + 4, product * 9 * 4 * 2 * 4, index + 9424, depth);
	innerBody<0>(stream, sum + 9 + 4 + 7 + 2, product * 9 * 4 * 7 * 2, index + 9472, depth);
	innerBody<0>(stream, sum + 9 + 4 + 8 + 8, product * 9 * 4 * 8 * 8, index + 9488, depth);
	innerBody<0>(stream, sum + 9 + 6 + 3 + 2, product * 9 * 6 * 3 * 2, index + 9632, depth);
	innerBody<0>(stream, sum + 9 + 6 + 4 + 8, product * 9 * 6 * 4 * 8, index + 9648, depth);
	innerBody<0>(stream, sum + 9 + 6 + 6 + 4, product * 9 * 6 * 6 * 4, index + 9664, depth);
	innerBody<0>(stream, sum + 9 + 6 + 9 + 6, product * 9 * 6 * 9 * 6, index + 9696, depth);
	innerBody<0>(stream, sum + 9 + 7 + 2 + 8, product * 9 * 7 * 2 * 8, index + 9728, depth);
	innerBody<0>(stream, sum + 9 + 7 + 4 + 4, product * 9 * 7 * 4 * 4, index + 9744, depth);
	innerBody<0>(stream, sum + 9 + 7 + 7 + 6, product * 9 * 7 * 7 * 6, index + 9776, depth);
	innerBody<0>(stream, sum + 9 + 7 + 9 + 2, product * 9 * 7 * 9 * 2, index + 9792, depth);
	innerBody<0>(stream, sum + 9 + 8 + 2 + 4, product * 9 * 8 * 2 * 4, index + 9824, depth);
	innerBody<0>(stream, sum + 9 + 8 + 7 + 2, product * 9 * 8 * 7 * 2, index + 9872, depth);
	innerBody<0>(stream, sum + 9 + 8 + 8 + 8, product * 9 * 8 * 8 * 8, index + 9888, depth);
	innerBody<0>(stream, sum + 9 + 9 + 3 + 6, product * 9 * 9 * 3 * 6, index + 9936, depth);
	innerBody<0>(stream, sum + 9 + 9 + 6 + 8, product * 9 * 9 * 6 * 8, index + 9968, depth);
	innerBody<0>(stream, sum + 9 + 9 + 8 + 4, product * 9 * 9 * 8 * 4, index + 9984, depth);
}

template<u64 index>
inline void initialBody() noexcept {
	// we don't want main aware of any details of how computation is performed.
	// This allows the decoupling of details from main and the computation body itself
	body<index>(std::cout);
}

template<u32 length>
inline void innerBody32(u32 sum, u32 product, u32 index) noexcept;

template<u32 length>
void body32(u32 sum = 0, u32 product = 1, u32 index = 0) noexcept {
	static_assert(length <= 9, "Can't have numbers over 9 digits on 32-bit numbers!");
	static_assert(length != 0, "Can't have length of zero!");
	constexpr auto inner = length - 1;
	constexpr auto next = fastPow10<inner>;
	auto baseProduct = product;
	sum += 2;
	product <<= 1;
	index += (next << 1);
	// unlike the 64-bit version of this code, doing the 32-bit ints for 9 digit
	// numbers (this code is not used when you request 64-bit numbers!)
	// does not require as much optimization. We can walk through digit level
	// by digit level (even if the digit does not contribute too much to the
	// overall process!).
	//for (int i = 2; i < 10; ++i) {
	//    innerBody32<inner>(sum, product, index);
	//    ++sum;
	//    product += baseProduct;
	//    index += next;
	//}
	// force an unroll here
	innerBody32<inner>(sum, product, index); // 2
	++sum;
	product += baseProduct;
	index += next;
	innerBody32<inner>(sum, product, index); // 3
	++sum;
	product += baseProduct;
	index += next;
	innerBody32<inner>(sum, product, index); // 4
	++sum;
	product += baseProduct;
	index += next;
	innerBody32<inner>(sum, product, index); // 5
	++sum;
	product += baseProduct;
	index += next;
	innerBody32<inner>(sum, product, index); // 6
	++sum;
	product += baseProduct;
	index += next;
	innerBody32<inner>(sum, product, index); // 7
	++sum;
	product += baseProduct;
	index += next;
	innerBody32<inner>(sum, product, index); // 8
	++sum;
	product += baseProduct;
	index += next;
	innerBody32<inner>(sum, product, index); // 9
}

template<u32 length>
void innerBody32(u32 sum, u32 product, u32 index) noexcept {
	// this double template instantiation is done to make sure that the compiler
	// does not attempt to instantiate infinitely, if this code was in place
	// of the call to innerbody32 in body32 then the compiler would not stop
	// instiantiating. we can then also perform specialization on length zero
	body32<length>(sum, product, index);
}

template<>
void innerBody32<0>(u32 sum, u32 product, u32 index) noexcept {
	// perform the check in the case that length == 0
	if (isQuodigious(index, sum, product)) {
		std::cout << index << std::endl;
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
