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
		auto lowerHalf = std::async(std::launch::async, [baseProduct, depth](auto sum, auto product, auto index) noexcept {
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
				}, sum, product, index);
		auto upperHalf = std::async(std::launch::async, [baseProduct, depth](auto sum, auto product, auto index) noexcept {
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
				}, sum, product, index);
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
			innerBody<inner>(stream, sum, product, index, depth); // 2
			sum += 2;
			product += (baseProduct << 1);
			index += (next << 1);
			innerBody<inner>(stream, sum, product, index, depth); // 4
			sum += 2;
			product += (baseProduct << 1);
			index += (next << 1);
			innerBody<inner>(stream, sum, product, index, depth); // 6
			sum += 2;
			product += (baseProduct << 1);
			index += (next << 1);
			innerBody<inner>(stream, sum, product, index, depth); // 8
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
	if (sum % 3 != 0) {
		return;
	}
#ifdef EXACT
	if (index % product != 0) {
		return;
	}
	if (index % sum == 0) {
		stream << index << '\n';
	}
#else // !defined(EXACT)
	if (index % product == 0) {
		stream << index << '\n';
	}
#endif // end !defined(EXACT)
}

//#include "Specialization2Digits.cc"
//#include "Specialization3Digits.cc"
#include "Specialization4Digits.cc"
//#include "Specialization5Digits.cc"
//#include "Specialization6Digits.cc"
//#include "Specialization7Digits.cc"
//#include "Specialization8Digits.cc"
//#include "Specialization9Digits.cc"
//#include "Specialization10Digits.cc"
//
template<>
inline void innerBody<8>(std::ostream& stream, u64 sum, u64 product, u64 index, u64 depth) noexcept {
++depth;
innerBody<4>(stream, sum+ 9 + 4 + 6 + 2, product* 9 * 4 * 6 * 2, index + 26490000, depth);
innerBody<4>(stream, sum+ 8 + 6 + 6 + 2, product* 8 * 6 * 6 * 2, index + 26680000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 6 + 2, product* 9 * 8 * 6 * 2, index + 26890000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 6 + 2, product* 7 * 9 * 6 * 2, index + 26970000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 6 + 2, product* 8 * 9 * 6 * 2, index + 26980000, depth);
innerBody<4>(stream, sum+ 8 + 7 + 7 + 2, product* 8 * 7 * 7 * 2, index + 27780000, depth);
innerBody<4>(stream, sum+ 4 + 6 + 8 + 2, product* 4 * 6 * 8 * 2, index + 28640000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 8 + 2, product* 7 * 8 * 8 * 2, index + 28870000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 8 + 2, product* 9 * 9 * 8 * 2, index + 28990000, depth);
innerBody<4>(stream, sum+ 7 + 4 + 9 + 2, product* 7 * 4 * 9 * 2, index + 29470000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 9 + 2, product* 9 * 6 * 9 * 2, index + 29690000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 9 + 2, product* 9 * 8 * 9 * 2, index + 29890000, depth);
innerBody<4>(stream, sum+ 6 + 9 + 9 + 2, product* 6 * 9 * 9 * 2, index + 29960000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 9 + 2, product* 8 * 9 * 9 * 2, index + 29980000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 3 + 3, product* 9 * 6 * 3 * 3, index + 33690000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 3 + 3, product* 9 * 9 * 3 * 3, index + 33990000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 4 + 3, product* 9 * 8 * 4 * 3, index + 34890000, depth);
innerBody<4>(stream, sum+ 3 + 7 + 6 + 3, product* 3 * 7 * 6 * 3, index + 36730000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 6 + 3, product* 7 * 8 * 6 * 3, index + 36870000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 6 + 3, product* 7 * 9 * 6 * 3, index + 36970000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 6 + 3, product* 9 * 9 * 6 * 3, index + 36990000, depth);
innerBody<4>(stream, sum+ 8 + 4 + 7 + 3, product* 8 * 4 * 7 * 3, index + 37480000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 7 + 3, product* 9 * 6 * 7 * 3, index + 37690000, depth);
innerBody<4>(stream, sum+ 6 + 7 + 7 + 3, product* 6 * 7 * 7 * 3, index + 37760000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 7 + 3, product* 9 * 8 * 7 * 3, index + 37890000, depth);
innerBody<4>(stream, sum+ 6 + 9 + 7 + 3, product* 6 * 9 * 7 * 3, index + 37960000, depth);
innerBody<4>(stream, sum+ 3 + 4 + 8 + 3, product* 3 * 4 * 8 * 3, index + 38430000, depth);
innerBody<4>(stream, sum+ 6 + 6 + 8 + 3, product* 6 * 6 * 8 * 3, index + 38660000, depth);
innerBody<4>(stream, sum+ 7 + 6 + 8 + 3, product* 7 * 6 * 8 * 3, index + 38670000, depth);
innerBody<4>(stream, sum+ 9 + 7 + 8 + 3, product* 9 * 7 * 8 * 3, index + 38790000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 8 + 3, product* 8 * 9 * 8 * 3, index + 38980000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 8 + 3, product* 9 * 9 * 8 * 3, index + 38990000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 9 + 3, product* 9 * 6 * 9 * 3, index + 39690000, depth);
innerBody<4>(stream, sum+ 7 + 7 + 9 + 3, product* 7 * 7 * 9 * 3, index + 39770000, depth);
innerBody<4>(stream, sum+ 8 + 7 + 9 + 3, product* 8 * 7 * 9 * 3, index + 39780000, depth);
innerBody<4>(stream, sum+ 4 + 8 + 9 + 3, product* 4 * 8 * 9 * 3, index + 39840000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 9 + 3, product* 7 * 8 * 9 * 3, index + 39870000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 9 + 3, product* 9 * 8 * 9 * 3, index + 39890000, depth);
innerBody<4>(stream, sum+ 4 + 9 + 9 + 3, product* 4 * 9 * 9 * 3, index + 39940000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 9 + 3, product* 8 * 9 * 9 * 3, index + 39980000, depth);
innerBody<4>(stream, sum+ 7 + 7 + 3 + 4, product* 7 * 7 * 3 * 4, index + 43770000, depth);
innerBody<4>(stream, sum+ 4 + 8 + 3 + 4, product* 4 * 8 * 3 * 4, index + 43840000, depth);
innerBody<4>(stream, sum+ 9 + 4 + 4 + 4, product* 9 * 4 * 4 * 4, index + 44490000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 4 + 4, product* 7 * 8 * 4 * 4, index + 44870000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 4 + 4, product* 9 * 8 * 4 * 4, index + 44890000, depth);
innerBody<4>(stream, sum+ 6 + 9 + 4 + 4, product* 6 * 9 * 4 * 4, index + 44960000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 4 + 4, product* 9 * 9 * 4 * 4, index + 44990000, depth);
innerBody<4>(stream, sum+ 6 + 6 + 6 + 4, product* 6 * 6 * 6 * 4, index + 46660000, depth);
innerBody<4>(stream, sum+ 6 + 7 + 6 + 4, product* 6 * 7 * 6 * 4, index + 46760000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 6 + 4, product* 9 * 8 * 6 * 4, index + 46890000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 6 + 4, product* 7 * 9 * 6 * 4, index + 46970000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 6 + 4, product* 9 * 9 * 6 * 4, index + 46990000, depth);
innerBody<4>(stream, sum+ 9 + 2 + 7 + 4, product* 9 * 2 * 7 * 4, index + 47290000, depth);
innerBody<4>(stream, sum+ 9 + 3 + 7 + 4, product* 9 * 3 * 7 * 4, index + 47390000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 7 + 4, product* 9 * 6 * 7 * 4, index + 47690000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 7 + 4, product* 7 * 8 * 7 * 4, index + 47870000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 7 + 4, product* 7 * 9 * 7 * 4, index + 47970000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 7 + 4, product* 8 * 9 * 7 * 4, index + 47980000, depth);
innerBody<4>(stream, sum+ 7 + 6 + 8 + 4, product* 7 * 6 * 8 * 4, index + 48670000, depth);
innerBody<4>(stream, sum+ 8 + 6 + 8 + 4, product* 8 * 6 * 8 * 4, index + 48680000, depth);
innerBody<4>(stream, sum+ 3 + 7 + 8 + 4, product* 3 * 7 * 8 * 4, index + 48730000, depth);
innerBody<4>(stream, sum+ 4 + 7 + 8 + 4, product* 4 * 7 * 8 * 4, index + 48740000, depth);
innerBody<4>(stream, sum+ 8 + 7 + 8 + 4, product* 8 * 7 * 8 * 4, index + 48780000, depth);
innerBody<4>(stream, sum+ 3 + 8 + 8 + 4, product* 3 * 8 * 8 * 4, index + 48830000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 8 + 4, product* 7 * 8 * 8 * 4, index + 48870000, depth);
innerBody<4>(stream, sum+ 8 + 8 + 8 + 4, product* 8 * 8 * 8 * 4, index + 48880000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 8 + 4, product* 9 * 8 * 8 * 4, index + 48890000, depth);
innerBody<4>(stream, sum+ 3 + 9 + 8 + 4, product* 3 * 9 * 8 * 4, index + 48930000, depth);
innerBody<4>(stream, sum+ 6 + 9 + 8 + 4, product* 6 * 9 * 8 * 4, index + 48960000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 8 + 4, product* 8 * 9 * 8 * 4, index + 48980000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 8 + 4, product* 9 * 9 * 8 * 4, index + 48990000, depth);
innerBody<4>(stream, sum+ 4 + 6 + 9 + 4, product* 4 * 6 * 9 * 4, index + 49640000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 9 + 4, product* 9 * 6 * 9 * 4, index + 49690000, depth);
innerBody<4>(stream, sum+ 4 + 7 + 9 + 4, product* 4 * 7 * 9 * 4, index + 49740000, depth);
innerBody<4>(stream, sum+ 7 + 7 + 9 + 4, product* 7 * 7 * 9 * 4, index + 49770000, depth);
innerBody<4>(stream, sum+ 8 + 7 + 9 + 4, product* 8 * 7 * 9 * 4, index + 49780000, depth);
innerBody<4>(stream, sum+ 9 + 7 + 9 + 4, product* 9 * 7 * 9 * 4, index + 49790000, depth);
innerBody<4>(stream, sum+ 3 + 8 + 9 + 4, product* 3 * 8 * 9 * 4, index + 49830000, depth);
innerBody<4>(stream, sum+ 6 + 8 + 9 + 4, product* 6 * 8 * 9 * 4, index + 49860000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 9 + 4, product* 7 * 8 * 9 * 4, index + 49870000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 9 + 4, product* 9 * 8 * 9 * 4, index + 49890000, depth);
innerBody<4>(stream, sum+ 6 + 9 + 9 + 4, product* 6 * 9 * 9 * 4, index + 49960000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 9 + 4, product* 8 * 9 * 9 * 4, index + 49980000, depth);
innerBody<4>(stream, sum+ 9 + 7 + 2 + 6, product* 9 * 7 * 2 * 6, index + 62790000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 2 + 6, product* 9 * 9 * 2 * 6, index + 62990000, depth);
innerBody<4>(stream, sum+ 7 + 7 + 3 + 6, product* 7 * 7 * 3 * 6, index + 63770000, depth);
innerBody<4>(stream, sum+ 3 + 9 + 3 + 6, product* 3 * 9 * 3 * 6, index + 63930000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 3 + 6, product* 8 * 9 * 3 * 6, index + 63980000, depth);
innerBody<4>(stream, sum+ 8 + 6 + 4 + 6, product* 8 * 6 * 4 * 6, index + 64680000, depth);
innerBody<4>(stream, sum+ 8 + 7 + 4 + 6, product* 8 * 7 * 4 * 6, index + 64780000, depth);
innerBody<4>(stream, sum+ 9 + 7 + 4 + 6, product* 9 * 7 * 4 * 6, index + 64790000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 4 + 6, product* 7 * 8 * 4 * 6, index + 64870000, depth);
innerBody<4>(stream, sum+ 6 + 9 + 4 + 6, product* 6 * 9 * 4 * 6, index + 64960000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 4 + 6, product* 8 * 9 * 4 * 6, index + 64980000, depth);
innerBody<4>(stream, sum+ 7 + 2 + 6 + 6, product* 7 * 2 * 6 * 6, index + 66270000, depth);
innerBody<4>(stream, sum+ 8 + 2 + 6 + 6, product* 8 * 2 * 6 * 6, index + 66280000, depth);
innerBody<4>(stream, sum+ 8 + 6 + 6 + 6, product* 8 * 6 * 6 * 6, index + 66680000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 6 + 6, product* 9 * 6 * 6 * 6, index + 66690000, depth);
innerBody<4>(stream, sum+ 7 + 7 + 6 + 6, product* 7 * 7 * 6 * 6, index + 66770000, depth);
innerBody<4>(stream, sum+ 8 + 8 + 6 + 6, product* 8 * 8 * 6 * 6, index + 66880000, depth);
innerBody<4>(stream, sum+ 2 + 9 + 6 + 6, product* 2 * 9 * 6 * 6, index + 66920000, depth);
innerBody<4>(stream, sum+ 6 + 9 + 6 + 6, product* 6 * 9 * 6 * 6, index + 66960000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 6 + 6, product* 7 * 9 * 6 * 6, index + 66970000, depth);
innerBody<4>(stream, sum+ 9 + 2 + 7 + 6, product* 9 * 2 * 7 * 6, index + 67290000, depth);
innerBody<4>(stream, sum+ 9 + 3 + 7 + 6, product* 9 * 3 * 7 * 6, index + 67390000, depth);
innerBody<4>(stream, sum+ 9 + 4 + 7 + 6, product* 9 * 4 * 7 * 6, index + 67490000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 7 + 6, product* 9 * 6 * 7 * 6, index + 67690000, depth);
innerBody<4>(stream, sum+ 8 + 7 + 7 + 6, product* 8 * 7 * 7 * 6, index + 67780000, depth);
innerBody<4>(stream, sum+ 9 + 7 + 7 + 6, product* 9 * 7 * 7 * 6, index + 67790000, depth);
innerBody<4>(stream, sum+ 4 + 8 + 7 + 6, product* 4 * 8 * 7 * 6, index + 67840000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 7 + 6, product* 7 * 8 * 7 * 6, index + 67870000, depth);
innerBody<4>(stream, sum+ 8 + 8 + 7 + 6, product* 8 * 8 * 7 * 6, index + 67880000, depth);
innerBody<4>(stream, sum+ 4 + 9 + 7 + 6, product* 4 * 9 * 7 * 6, index + 67940000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 7 + 6, product* 7 * 9 * 7 * 6, index + 67970000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 7 + 6, product* 8 * 9 * 7 * 6, index + 67980000, depth);
innerBody<4>(stream, sum+ 3 + 3 + 8 + 6, product* 3 * 3 * 8 * 6, index + 68330000, depth);
innerBody<4>(stream, sum+ 6 + 3 + 8 + 6, product* 6 * 3 * 8 * 6, index + 68360000, depth);
innerBody<4>(stream, sum+ 7 + 3 + 8 + 6, product* 7 * 3 * 8 * 6, index + 68370000, depth);
innerBody<4>(stream, sum+ 9 + 3 + 8 + 6, product* 9 * 3 * 8 * 6, index + 68390000, depth);
innerBody<4>(stream, sum+ 2 + 4 + 8 + 6, product* 2 * 4 * 8 * 6, index + 68420000, depth);
innerBody<4>(stream, sum+ 6 + 4 + 8 + 6, product* 6 * 4 * 8 * 6, index + 68460000, depth);
innerBody<4>(stream, sum+ 3 + 6 + 8 + 6, product* 3 * 6 * 8 * 6, index + 68630000, depth);
innerBody<4>(stream, sum+ 8 + 6 + 8 + 6, product* 8 * 6 * 8 * 6, index + 68680000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 8 + 6, product* 9 * 6 * 8 * 6, index + 68690000, depth);
innerBody<4>(stream, sum+ 6 + 7 + 8 + 6, product* 6 * 7 * 8 * 6, index + 68760000, depth);
innerBody<4>(stream, sum+ 7 + 7 + 8 + 6, product* 7 * 7 * 8 * 6, index + 68770000, depth);
innerBody<4>(stream, sum+ 9 + 7 + 8 + 6, product* 9 * 7 * 8 * 6, index + 68790000, depth);
innerBody<4>(stream, sum+ 3 + 8 + 8 + 6, product* 3 * 8 * 8 * 6, index + 68830000, depth);
innerBody<4>(stream, sum+ 4 + 8 + 8 + 6, product* 4 * 8 * 8 * 6, index + 68840000, depth);
innerBody<4>(stream, sum+ 6 + 8 + 8 + 6, product* 6 * 8 * 8 * 6, index + 68860000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 8 + 6, product* 7 * 8 * 8 * 6, index + 68870000, depth);
innerBody<4>(stream, sum+ 8 + 8 + 8 + 6, product* 8 * 8 * 8 * 6, index + 68880000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 8 + 6, product* 9 * 8 * 8 * 6, index + 68890000, depth);
innerBody<4>(stream, sum+ 3 + 9 + 8 + 6, product* 3 * 9 * 8 * 6, index + 68930000, depth);
innerBody<4>(stream, sum+ 6 + 9 + 8 + 6, product* 6 * 9 * 8 * 6, index + 68960000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 8 + 6, product* 7 * 9 * 8 * 6, index + 68970000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 8 + 6, product* 8 * 9 * 8 * 6, index + 68980000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 8 + 6, product* 9 * 9 * 8 * 6, index + 68990000, depth);
innerBody<4>(stream, sum+ 8 + 2 + 9 + 6, product* 8 * 2 * 9 * 6, index + 69280000, depth);
innerBody<4>(stream, sum+ 7 + 3 + 9 + 6, product* 7 * 3 * 9 * 6, index + 69370000, depth);
innerBody<4>(stream, sum+ 8 + 3 + 9 + 6, product* 8 * 3 * 9 * 6, index + 69380000, depth);
innerBody<4>(stream, sum+ 9 + 3 + 9 + 6, product* 9 * 3 * 9 * 6, index + 69390000, depth);
innerBody<4>(stream, sum+ 8 + 4 + 9 + 6, product* 8 * 4 * 9 * 6, index + 69480000, depth);
innerBody<4>(stream, sum+ 3 + 6 + 9 + 6, product* 3 * 6 * 9 * 6, index + 69630000, depth);
innerBody<4>(stream, sum+ 7 + 6 + 9 + 6, product* 7 * 6 * 9 * 6, index + 69670000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 9 + 6, product* 9 * 6 * 9 * 6, index + 69690000, depth);
innerBody<4>(stream, sum+ 6 + 7 + 9 + 6, product* 6 * 7 * 9 * 6, index + 69760000, depth);
innerBody<4>(stream, sum+ 7 + 7 + 9 + 6, product* 7 * 7 * 9 * 6, index + 69770000, depth);
innerBody<4>(stream, sum+ 8 + 7 + 9 + 6, product* 8 * 7 * 9 * 6, index + 69780000, depth);
innerBody<4>(stream, sum+ 9 + 7 + 9 + 6, product* 9 * 7 * 9 * 6, index + 69790000, depth);
innerBody<4>(stream, sum+ 6 + 8 + 9 + 6, product* 6 * 8 * 9 * 6, index + 69860000, depth);
innerBody<4>(stream, sum+ 8 + 8 + 9 + 6, product* 8 * 8 * 9 * 6, index + 69880000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 9 + 6, product* 9 * 8 * 9 * 6, index + 69890000, depth);
innerBody<4>(stream, sum+ 2 + 9 + 9 + 6, product* 2 * 9 * 9 * 6, index + 69920000, depth);
innerBody<4>(stream, sum+ 3 + 9 + 9 + 6, product* 3 * 9 * 9 * 6, index + 69930000, depth);
innerBody<4>(stream, sum+ 6 + 9 + 9 + 6, product* 6 * 9 * 9 * 6, index + 69960000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 9 + 6, product* 9 * 9 * 9 * 6, index + 69990000, depth);
innerBody<4>(stream, sum+ 8 + 6 + 2 + 7, product* 8 * 6 * 2 * 7, index + 72680000, depth);
innerBody<4>(stream, sum+ 9 + 7 + 2 + 7, product* 9 * 7 * 2 * 7, index + 72790000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 2 + 7, product* 9 * 8 * 2 * 7, index + 72890000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 2 + 7, product* 8 * 9 * 2 * 7, index + 72980000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 3 + 7, product* 9 * 6 * 3 * 7, index + 73690000, depth);
innerBody<4>(stream, sum+ 8 + 7 + 3 + 7, product* 8 * 7 * 3 * 7, index + 73780000, depth);
innerBody<4>(stream, sum+ 9 + 7 + 3 + 7, product* 9 * 7 * 3 * 7, index + 73790000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 3 + 7, product* 9 * 8 * 3 * 7, index + 73890000, depth);
innerBody<4>(stream, sum+ 6 + 9 + 3 + 7, product* 6 * 9 * 3 * 7, index + 73960000, depth);
innerBody<4>(stream, sum+ 8 + 2 + 4 + 7, product* 8 * 2 * 4 * 7, index + 74280000, depth);
innerBody<4>(stream, sum+ 6 + 7 + 4 + 7, product* 6 * 7 * 4 * 7, index + 74760000, depth);
innerBody<4>(stream, sum+ 8 + 8 + 4 + 7, product* 8 * 8 * 4 * 7, index + 74880000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 4 + 7, product* 7 * 9 * 4 * 7, index + 74970000, depth);
innerBody<4>(stream, sum+ 7 + 4 + 6 + 7, product* 7 * 4 * 6 * 7, index + 76470000, depth);
innerBody<4>(stream, sum+ 8 + 4 + 6 + 7, product* 8 * 4 * 6 * 7, index + 76480000, depth);
innerBody<4>(stream, sum+ 9 + 4 + 6 + 7, product* 9 * 4 * 6 * 7, index + 76490000, depth);
innerBody<4>(stream, sum+ 4 + 6 + 6 + 7, product* 4 * 6 * 6 * 7, index + 76640000, depth);
innerBody<4>(stream, sum+ 7 + 6 + 6 + 7, product* 7 * 6 * 6 * 7, index + 76670000, depth);
innerBody<4>(stream, sum+ 7 + 7 + 6 + 7, product* 7 * 7 * 6 * 7, index + 76770000, depth);
innerBody<4>(stream, sum+ 3 + 8 + 6 + 7, product* 3 * 8 * 6 * 7, index + 76830000, depth);
innerBody<4>(stream, sum+ 4 + 8 + 6 + 7, product* 4 * 8 * 6 * 7, index + 76840000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 6 + 7, product* 7 * 8 * 6 * 7, index + 76870000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 6 + 7, product* 9 * 8 * 6 * 7, index + 76890000, depth);
innerBody<4>(stream, sum+ 3 + 9 + 6 + 7, product* 3 * 9 * 6 * 7, index + 76930000, depth);
innerBody<4>(stream, sum+ 6 + 9 + 6 + 7, product* 6 * 9 * 6 * 7, index + 76960000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 6 + 7, product* 7 * 9 * 6 * 7, index + 76970000, depth);
innerBody<4>(stream, sum+ 9 + 2 + 7 + 7, product* 9 * 2 * 7 * 7, index + 77290000, depth);
innerBody<4>(stream, sum+ 8 + 3 + 7 + 7, product* 8 * 3 * 7 * 7, index + 77380000, depth);
innerBody<4>(stream, sum+ 7 + 4 + 7 + 7, product* 7 * 4 * 7 * 7, index + 77470000, depth);
innerBody<4>(stream, sum+ 3 + 6 + 7 + 7, product* 3 * 6 * 7 * 7, index + 77630000, depth);
innerBody<4>(stream, sum+ 7 + 6 + 7 + 7, product* 7 * 6 * 7 * 7, index + 77670000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 7 + 7, product* 9 * 6 * 7 * 7, index + 77690000, depth);
innerBody<4>(stream, sum+ 6 + 7 + 7 + 7, product* 6 * 7 * 7 * 7, index + 77760000, depth);
innerBody<4>(stream, sum+ 9 + 7 + 7 + 7, product* 9 * 7 * 7 * 7, index + 77790000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 7 + 7, product* 7 * 8 * 7 * 7, index + 77870000, depth);
innerBody<4>(stream, sum+ 8 + 8 + 7 + 7, product* 8 * 8 * 7 * 7, index + 77880000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 7 + 7, product* 9 * 8 * 7 * 7, index + 77890000, depth);
innerBody<4>(stream, sum+ 6 + 9 + 7 + 7, product* 6 * 9 * 7 * 7, index + 77960000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 7 + 7, product* 8 * 9 * 7 * 7, index + 77980000, depth);
innerBody<4>(stream, sum+ 7 + 2 + 8 + 7, product* 7 * 2 * 8 * 7, index + 78270000, depth);
innerBody<4>(stream, sum+ 3 + 3 + 8 + 7, product* 3 * 3 * 8 * 7, index + 78330000, depth);
innerBody<4>(stream, sum+ 7 + 3 + 8 + 7, product* 7 * 3 * 8 * 7, index + 78370000, depth);
innerBody<4>(stream, sum+ 4 + 4 + 8 + 7, product* 4 * 4 * 8 * 7, index + 78440000, depth);
innerBody<4>(stream, sum+ 7 + 4 + 8 + 7, product* 7 * 4 * 8 * 7, index + 78470000, depth);
innerBody<4>(stream, sum+ 9 + 4 + 8 + 7, product* 9 * 4 * 8 * 7, index + 78490000, depth);
innerBody<4>(stream, sum+ 8 + 6 + 8 + 7, product* 8 * 6 * 8 * 7, index + 78680000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 8 + 7, product* 9 * 6 * 8 * 7, index + 78690000, depth);
innerBody<4>(stream, sum+ 8 + 7 + 8 + 7, product* 8 * 7 * 8 * 7, index + 78780000, depth);
innerBody<4>(stream, sum+ 4 + 8 + 8 + 7, product* 4 * 8 * 8 * 7, index + 78840000, depth);
innerBody<4>(stream, sum+ 8 + 8 + 8 + 7, product* 8 * 8 * 8 * 7, index + 78880000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 8 + 7, product* 9 * 8 * 8 * 7, index + 78890000, depth);
innerBody<4>(stream, sum+ 2 + 9 + 8 + 7, product* 2 * 9 * 8 * 7, index + 78920000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 8 + 7, product* 7 * 9 * 8 * 7, index + 78970000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 8 + 7, product* 9 * 9 * 8 * 7, index + 78990000, depth);
innerBody<4>(stream, sum+ 3 + 2 + 9 + 7, product* 3 * 2 * 9 * 7, index + 79230000, depth);
innerBody<4>(stream, sum+ 9 + 2 + 9 + 7, product* 9 * 2 * 9 * 7, index + 79290000, depth);
innerBody<4>(stream, sum+ 6 + 3 + 9 + 7, product* 6 * 3 * 9 * 7, index + 79360000, depth);
innerBody<4>(stream, sum+ 8 + 3 + 9 + 7, product* 8 * 3 * 9 * 7, index + 79380000, depth);
innerBody<4>(stream, sum+ 3 + 4 + 9 + 7, product* 3 * 4 * 9 * 7, index + 79430000, depth);
innerBody<4>(stream, sum+ 8 + 6 + 9 + 7, product* 8 * 6 * 9 * 7, index + 79680000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 9 + 7, product* 9 * 6 * 9 * 7, index + 79690000, depth);
innerBody<4>(stream, sum+ 8 + 7 + 9 + 7, product* 8 * 7 * 9 * 7, index + 79780000, depth);
innerBody<4>(stream, sum+ 9 + 7 + 9 + 7, product* 9 * 7 * 9 * 7, index + 79790000, depth);
innerBody<4>(stream, sum+ 4 + 8 + 9 + 7, product* 4 * 8 * 9 * 7, index + 79840000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 9 + 7, product* 7 * 8 * 9 * 7, index + 79870000, depth);
innerBody<4>(stream, sum+ 8 + 8 + 9 + 7, product* 8 * 8 * 9 * 7, index + 79880000, depth);
innerBody<4>(stream, sum+ 4 + 9 + 9 + 7, product* 4 * 9 * 9 * 7, index + 79940000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 9 + 7, product* 7 * 9 * 9 * 7, index + 79970000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 9 + 7, product* 8 * 9 * 9 * 7, index + 79980000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 9 + 7, product* 9 * 9 * 9 * 7, index + 79990000, depth);
innerBody<4>(stream, sum+ 8 + 8 + 2 + 8, product* 8 * 8 * 2 * 8, index + 82880000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 2 + 8, product* 7 * 9 * 2 * 8, index + 82970000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 2 + 8, product* 8 * 9 * 2 * 8, index + 82980000, depth);
innerBody<4>(stream, sum+ 7 + 3 + 3 + 8, product* 7 * 3 * 3 * 8, index + 83370000, depth);
innerBody<4>(stream, sum+ 9 + 3 + 3 + 8, product* 9 * 3 * 3 * 8, index + 83390000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 3 + 8, product* 9 * 6 * 3 * 8, index + 83690000, depth);
innerBody<4>(stream, sum+ 8 + 7 + 3 + 8, product* 8 * 7 * 3 * 8, index + 83780000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 3 + 8, product* 7 * 8 * 3 * 8, index + 83870000, depth);
innerBody<4>(stream, sum+ 3 + 9 + 3 + 8, product* 3 * 9 * 3 * 8, index + 83930000, depth);
innerBody<4>(stream, sum+ 4 + 9 + 3 + 8, product* 4 * 9 * 3 * 8, index + 83940000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 3 + 8, product* 9 * 9 * 3 * 8, index + 83990000, depth);
innerBody<4>(stream, sum+ 4 + 6 + 4 + 8, product* 4 * 6 * 4 * 8, index + 84640000, depth);
innerBody<4>(stream, sum+ 8 + 6 + 4 + 8, product* 8 * 6 * 4 * 8, index + 84680000, depth);
innerBody<4>(stream, sum+ 7 + 7 + 4 + 8, product* 7 * 7 * 4 * 8, index + 84770000, depth);
innerBody<4>(stream, sum+ 8 + 7 + 4 + 8, product* 8 * 7 * 4 * 8, index + 84780000, depth);
innerBody<4>(stream, sum+ 4 + 8 + 4 + 8, product* 4 * 8 * 4 * 8, index + 84840000, depth);
innerBody<4>(stream, sum+ 6 + 8 + 4 + 8, product* 6 * 8 * 4 * 8, index + 84860000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 4 + 8, product* 7 * 8 * 4 * 8, index + 84870000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 4 + 8, product* 9 * 8 * 4 * 8, index + 84890000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 4 + 8, product* 9 * 9 * 4 * 8, index + 84990000, depth);
innerBody<4>(stream, sum+ 8 + 2 + 6 + 8, product* 8 * 2 * 6 * 8, index + 86280000, depth);
innerBody<4>(stream, sum+ 4 + 3 + 6 + 8, product* 4 * 3 * 6 * 8, index + 86340000, depth);
innerBody<4>(stream, sum+ 6 + 4 + 6 + 8, product* 6 * 4 * 6 * 8, index + 86460000, depth);
innerBody<4>(stream, sum+ 7 + 4 + 6 + 8, product* 7 * 4 * 6 * 8, index + 86470000, depth);
innerBody<4>(stream, sum+ 9 + 4 + 6 + 8, product* 9 * 4 * 6 * 8, index + 86490000, depth);
innerBody<4>(stream, sum+ 7 + 6 + 6 + 8, product* 7 * 6 * 6 * 8, index + 86670000, depth);
innerBody<4>(stream, sum+ 8 + 6 + 6 + 8, product* 8 * 6 * 6 * 8, index + 86680000, depth);
innerBody<4>(stream, sum+ 2 + 7 + 6 + 8, product* 2 * 7 * 6 * 8, index + 86720000, depth);
innerBody<4>(stream, sum+ 6 + 7 + 6 + 8, product* 6 * 7 * 6 * 8, index + 86760000, depth);
innerBody<4>(stream, sum+ 9 + 7 + 6 + 8, product* 9 * 7 * 6 * 8, index + 86790000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 6 + 8, product* 7 * 8 * 6 * 8, index + 86870000, depth);
innerBody<4>(stream, sum+ 8 + 8 + 6 + 8, product* 8 * 8 * 6 * 8, index + 86880000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 6 + 8, product* 9 * 8 * 6 * 8, index + 86890000, depth);
innerBody<4>(stream, sum+ 2 + 9 + 6 + 8, product* 2 * 9 * 6 * 8, index + 86920000, depth);
innerBody<4>(stream, sum+ 4 + 9 + 6 + 8, product* 4 * 9 * 6 * 8, index + 86940000, depth);
innerBody<4>(stream, sum+ 6 + 9 + 6 + 8, product* 6 * 9 * 6 * 8, index + 86960000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 6 + 8, product* 7 * 9 * 6 * 8, index + 86970000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 6 + 8, product* 8 * 9 * 6 * 8, index + 86980000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 6 + 8, product* 9 * 9 * 6 * 8, index + 86990000, depth);
innerBody<4>(stream, sum+ 9 + 2 + 7 + 8, product* 9 * 2 * 7 * 8, index + 87290000, depth);
innerBody<4>(stream, sum+ 6 + 3 + 7 + 8, product* 6 * 3 * 7 * 8, index + 87360000, depth);
innerBody<4>(stream, sum+ 8 + 3 + 7 + 8, product* 8 * 3 * 7 * 8, index + 87380000, depth);
innerBody<4>(stream, sum+ 6 + 4 + 7 + 8, product* 6 * 4 * 7 * 8, index + 87460000, depth);
innerBody<4>(stream, sum+ 8 + 4 + 7 + 8, product* 8 * 4 * 7 * 8, index + 87480000, depth);
innerBody<4>(stream, sum+ 9 + 4 + 7 + 8, product* 9 * 4 * 7 * 8, index + 87490000, depth);
innerBody<4>(stream, sum+ 6 + 6 + 7 + 8, product* 6 * 6 * 7 * 8, index + 87660000, depth);
innerBody<4>(stream, sum+ 8 + 6 + 7 + 8, product* 8 * 6 * 7 * 8, index + 87680000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 7 + 8, product* 9 * 6 * 7 * 8, index + 87690000, depth);
innerBody<4>(stream, sum+ 7 + 7 + 7 + 8, product* 7 * 7 * 7 * 8, index + 87770000, depth);
innerBody<4>(stream, sum+ 8 + 7 + 7 + 8, product* 8 * 7 * 7 * 8, index + 87780000, depth);
innerBody<4>(stream, sum+ 9 + 7 + 7 + 8, product* 9 * 7 * 7 * 8, index + 87790000, depth);
innerBody<4>(stream, sum+ 3 + 8 + 7 + 8, product* 3 * 8 * 7 * 8, index + 87830000, depth);
innerBody<4>(stream, sum+ 4 + 8 + 7 + 8, product* 4 * 8 * 7 * 8, index + 87840000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 7 + 8, product* 7 * 8 * 7 * 8, index + 87870000, depth);
innerBody<4>(stream, sum+ 8 + 8 + 7 + 8, product* 8 * 8 * 7 * 8, index + 87880000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 7 + 8, product* 9 * 8 * 7 * 8, index + 87890000, depth);
innerBody<4>(stream, sum+ 3 + 9 + 7 + 8, product* 3 * 9 * 7 * 8, index + 87930000, depth);
innerBody<4>(stream, sum+ 4 + 9 + 7 + 8, product* 4 * 9 * 7 * 8, index + 87940000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 7 + 8, product* 7 * 9 * 7 * 8, index + 87970000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 7 + 8, product* 8 * 9 * 7 * 8, index + 87980000, depth);
innerBody<4>(stream, sum+ 6 + 2 + 8 + 8, product* 6 * 2 * 8 * 8, index + 88260000, depth);
innerBody<4>(stream, sum+ 9 + 2 + 8 + 8, product* 9 * 2 * 8 * 8, index + 88290000, depth);
innerBody<4>(stream, sum+ 6 + 3 + 8 + 8, product* 6 * 3 * 8 * 8, index + 88360000, depth);
innerBody<4>(stream, sum+ 7 + 4 + 8 + 8, product* 7 * 4 * 8 * 8, index + 88470000, depth);
innerBody<4>(stream, sum+ 8 + 4 + 8 + 8, product* 8 * 4 * 8 * 8, index + 88480000, depth);
innerBody<4>(stream, sum+ 9 + 4 + 8 + 8, product* 9 * 4 * 8 * 8, index + 88490000, depth);
innerBody<4>(stream, sum+ 6 + 6 + 8 + 8, product* 6 * 6 * 8 * 8, index + 88660000, depth);
innerBody<4>(stream, sum+ 7 + 6 + 8 + 8, product* 7 * 6 * 8 * 8, index + 88670000, depth);
innerBody<4>(stream, sum+ 8 + 6 + 8 + 8, product* 8 * 6 * 8 * 8, index + 88680000, depth);
innerBody<4>(stream, sum+ 3 + 7 + 8 + 8, product* 3 * 7 * 8 * 8, index + 88730000, depth);
innerBody<4>(stream, sum+ 6 + 7 + 8 + 8, product* 6 * 7 * 8 * 8, index + 88760000, depth);
innerBody<4>(stream, sum+ 7 + 7 + 8 + 8, product* 7 * 7 * 8 * 8, index + 88770000, depth);
innerBody<4>(stream, sum+ 8 + 7 + 8 + 8, product* 8 * 7 * 8 * 8, index + 88780000, depth);
innerBody<4>(stream, sum+ 9 + 7 + 8 + 8, product* 9 * 7 * 8 * 8, index + 88790000, depth);
innerBody<4>(stream, sum+ 4 + 8 + 8 + 8, product* 4 * 8 * 8 * 8, index + 88840000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 8 + 8, product* 7 * 8 * 8 * 8, index + 88870000, depth);
innerBody<4>(stream, sum+ 8 + 8 + 8 + 8, product* 8 * 8 * 8 * 8, index + 88880000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 8 + 8, product* 9 * 8 * 8 * 8, index + 88890000, depth);
innerBody<4>(stream, sum+ 3 + 9 + 8 + 8, product* 3 * 9 * 8 * 8, index + 88930000, depth);
innerBody<4>(stream, sum+ 4 + 9 + 8 + 8, product* 4 * 9 * 8 * 8, index + 88940000, depth);
innerBody<4>(stream, sum+ 6 + 9 + 8 + 8, product* 6 * 9 * 8 * 8, index + 88960000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 8 + 8, product* 7 * 9 * 8 * 8, index + 88970000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 8 + 8, product* 8 * 9 * 8 * 8, index + 88980000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 8 + 8, product* 9 * 9 * 8 * 8, index + 88990000, depth);
innerBody<4>(stream, sum+ 9 + 2 + 9 + 8, product* 9 * 2 * 9 * 8, index + 89290000, depth);
innerBody<4>(stream, sum+ 7 + 3 + 9 + 8, product* 7 * 3 * 9 * 8, index + 89370000, depth);
innerBody<4>(stream, sum+ 8 + 3 + 9 + 8, product* 8 * 3 * 9 * 8, index + 89380000, depth);
innerBody<4>(stream, sum+ 9 + 3 + 9 + 8, product* 9 * 3 * 9 * 8, index + 89390000, depth);
innerBody<4>(stream, sum+ 4 + 4 + 9 + 8, product* 4 * 4 * 9 * 8, index + 89440000, depth);
innerBody<4>(stream, sum+ 7 + 4 + 9 + 8, product* 7 * 4 * 9 * 8, index + 89470000, depth);
innerBody<4>(stream, sum+ 8 + 4 + 9 + 8, product* 8 * 4 * 9 * 8, index + 89480000, depth);
innerBody<4>(stream, sum+ 3 + 6 + 9 + 8, product* 3 * 6 * 9 * 8, index + 89630000, depth);
innerBody<4>(stream, sum+ 7 + 6 + 9 + 8, product* 7 * 6 * 9 * 8, index + 89670000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 9 + 8, product* 9 * 6 * 9 * 8, index + 89690000, depth);
innerBody<4>(stream, sum+ 6 + 7 + 9 + 8, product* 6 * 7 * 9 * 8, index + 89760000, depth);
innerBody<4>(stream, sum+ 7 + 7 + 9 + 8, product* 7 * 7 * 9 * 8, index + 89770000, depth);
innerBody<4>(stream, sum+ 8 + 7 + 9 + 8, product* 8 * 7 * 9 * 8, index + 89780000, depth);
innerBody<4>(stream, sum+ 9 + 7 + 9 + 8, product* 9 * 7 * 9 * 8, index + 89790000, depth);
innerBody<4>(stream, sum+ 6 + 8 + 9 + 8, product* 6 * 8 * 9 * 8, index + 89860000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 9 + 8, product* 7 * 8 * 9 * 8, index + 89870000, depth);
innerBody<4>(stream, sum+ 8 + 8 + 9 + 8, product* 8 * 8 * 9 * 8, index + 89880000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 9 + 8, product* 9 * 8 * 9 * 8, index + 89890000, depth);
innerBody<4>(stream, sum+ 2 + 9 + 9 + 8, product* 2 * 9 * 9 * 8, index + 89920000, depth);
innerBody<4>(stream, sum+ 6 + 9 + 9 + 8, product* 6 * 9 * 9 * 8, index + 89960000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 9 + 8, product* 7 * 9 * 9 * 8, index + 89970000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 9 + 8, product* 8 * 9 * 9 * 8, index + 89980000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 2 + 9, product* 7 * 8 * 2 * 9, index + 92870000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 2 + 9, product* 9 * 8 * 2 * 9, index + 92890000, depth);
innerBody<4>(stream, sum+ 6 + 9 + 2 + 9, product* 6 * 9 * 2 * 9, index + 92960000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 2 + 9, product* 8 * 9 * 2 * 9, index + 92980000, depth);
innerBody<4>(stream, sum+ 6 + 4 + 3 + 9, product* 6 * 4 * 3 * 9, index + 93460000, depth);
innerBody<4>(stream, sum+ 8 + 4 + 3 + 9, product* 8 * 4 * 3 * 9, index + 93480000, depth);
innerBody<4>(stream, sum+ 7 + 6 + 3 + 9, product* 7 * 6 * 3 * 9, index + 93670000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 3 + 9, product* 9 * 6 * 3 * 9, index + 93690000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 3 + 9, product* 9 * 8 * 3 * 9, index + 93890000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 3 + 9, product* 9 * 9 * 3 * 9, index + 93990000, depth);
innerBody<4>(stream, sum+ 7 + 3 + 4 + 9, product* 7 * 3 * 4 * 9, index + 94370000, depth);
innerBody<4>(stream, sum+ 8 + 4 + 4 + 9, product* 8 * 4 * 4 * 9, index + 94480000, depth);
innerBody<4>(stream, sum+ 8 + 6 + 4 + 9, product* 8 * 6 * 4 * 9, index + 94680000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 4 + 9, product* 9 * 6 * 4 * 9, index + 94690000, depth);
innerBody<4>(stream, sum+ 9 + 7 + 4 + 9, product* 9 * 7 * 4 * 9, index + 94790000, depth);
innerBody<4>(stream, sum+ 6 + 8 + 4 + 9, product* 6 * 8 * 4 * 9, index + 94860000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 4 + 9, product* 7 * 8 * 4 * 9, index + 94870000, depth);
innerBody<4>(stream, sum+ 8 + 8 + 4 + 9, product* 8 * 8 * 4 * 9, index + 94880000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 4 + 9, product* 7 * 9 * 4 * 9, index + 94970000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 4 + 9, product* 8 * 9 * 4 * 9, index + 94980000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 4 + 9, product* 9 * 9 * 4 * 9, index + 94990000, depth);
innerBody<4>(stream, sum+ 9 + 3 + 6 + 9, product* 9 * 3 * 6 * 9, index + 96390000, depth);
innerBody<4>(stream, sum+ 9 + 4 + 6 + 9, product* 9 * 4 * 6 * 9, index + 96490000, depth);
innerBody<4>(stream, sum+ 4 + 6 + 6 + 9, product* 4 * 6 * 6 * 9, index + 96640000, depth);
innerBody<4>(stream, sum+ 7 + 6 + 6 + 9, product* 7 * 6 * 6 * 9, index + 96670000, depth);
innerBody<4>(stream, sum+ 8 + 6 + 6 + 9, product* 8 * 6 * 6 * 9, index + 96680000, depth);
innerBody<4>(stream, sum+ 3 + 7 + 6 + 9, product* 3 * 7 * 6 * 9, index + 96730000, depth);
innerBody<4>(stream, sum+ 4 + 7 + 6 + 9, product* 4 * 7 * 6 * 9, index + 96740000, depth);
innerBody<4>(stream, sum+ 6 + 7 + 6 + 9, product* 6 * 7 * 6 * 9, index + 96760000, depth);
innerBody<4>(stream, sum+ 7 + 7 + 6 + 9, product* 7 * 7 * 6 * 9, index + 96770000, depth);
innerBody<4>(stream, sum+ 8 + 7 + 6 + 9, product* 8 * 7 * 6 * 9, index + 96780000, depth);
innerBody<4>(stream, sum+ 9 + 7 + 6 + 9, product* 9 * 7 * 6 * 9, index + 96790000, depth);
innerBody<4>(stream, sum+ 3 + 8 + 6 + 9, product* 3 * 8 * 6 * 9, index + 96830000, depth);
innerBody<4>(stream, sum+ 4 + 8 + 6 + 9, product* 4 * 8 * 6 * 9, index + 96840000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 6 + 9, product* 7 * 8 * 6 * 9, index + 96870000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 6 + 9, product* 9 * 8 * 6 * 9, index + 96890000, depth);
innerBody<4>(stream, sum+ 3 + 9 + 6 + 9, product* 3 * 9 * 6 * 9, index + 96930000, depth);
innerBody<4>(stream, sum+ 6 + 9 + 6 + 9, product* 6 * 9 * 6 * 9, index + 96960000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 6 + 9, product* 7 * 9 * 6 * 9, index + 96970000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 6 + 9, product* 9 * 9 * 6 * 9, index + 96990000, depth);
innerBody<4>(stream, sum+ 8 + 2 + 7 + 9, product* 8 * 2 * 7 * 9, index + 97280000, depth);
innerBody<4>(stream, sum+ 7 + 3 + 7 + 9, product* 7 * 3 * 7 * 9, index + 97370000, depth);
innerBody<4>(stream, sum+ 7 + 4 + 7 + 9, product* 7 * 4 * 7 * 9, index + 97470000, depth);
innerBody<4>(stream, sum+ 8 + 4 + 7 + 9, product* 8 * 4 * 7 * 9, index + 97480000, depth);
innerBody<4>(stream, sum+ 9 + 4 + 7 + 9, product* 9 * 4 * 7 * 9, index + 97490000, depth);
innerBody<4>(stream, sum+ 4 + 6 + 7 + 9, product* 4 * 6 * 7 * 9, index + 97640000, depth);
innerBody<4>(stream, sum+ 8 + 6 + 7 + 9, product* 8 * 6 * 7 * 9, index + 97680000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 7 + 9, product* 9 * 6 * 7 * 9, index + 97690000, depth);
innerBody<4>(stream, sum+ 3 + 7 + 7 + 9, product* 3 * 7 * 7 * 9, index + 97730000, depth);
innerBody<4>(stream, sum+ 4 + 7 + 7 + 9, product* 4 * 7 * 7 * 9, index + 97740000, depth);
innerBody<4>(stream, sum+ 6 + 7 + 7 + 9, product* 6 * 7 * 7 * 9, index + 97760000, depth);
innerBody<4>(stream, sum+ 7 + 7 + 7 + 9, product* 7 * 7 * 7 * 9, index + 97770000, depth);
innerBody<4>(stream, sum+ 8 + 7 + 7 + 9, product* 8 * 7 * 7 * 9, index + 97780000, depth);
innerBody<4>(stream, sum+ 9 + 7 + 7 + 9, product* 9 * 7 * 7 * 9, index + 97790000, depth);
innerBody<4>(stream, sum+ 3 + 8 + 7 + 9, product* 3 * 8 * 7 * 9, index + 97830000, depth);
innerBody<4>(stream, sum+ 6 + 8 + 7 + 9, product* 6 * 8 * 7 * 9, index + 97860000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 7 + 9, product* 7 * 8 * 7 * 9, index + 97870000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 7 + 9, product* 9 * 8 * 7 * 9, index + 97890000, depth);
innerBody<4>(stream, sum+ 2 + 9 + 7 + 9, product* 2 * 9 * 7 * 9, index + 97920000, depth);
innerBody<4>(stream, sum+ 4 + 9 + 7 + 9, product* 4 * 9 * 7 * 9, index + 97940000, depth);
innerBody<4>(stream, sum+ 6 + 9 + 7 + 9, product* 6 * 9 * 7 * 9, index + 97960000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 7 + 9, product* 7 * 9 * 7 * 9, index + 97970000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 7 + 9, product* 8 * 9 * 7 * 9, index + 97980000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 7 + 9, product* 9 * 9 * 7 * 9, index + 97990000, depth);
innerBody<4>(stream, sum+ 6 + 3 + 8 + 9, product* 6 * 3 * 8 * 9, index + 98360000, depth);
innerBody<4>(stream, sum+ 9 + 3 + 8 + 9, product* 9 * 3 * 8 * 9, index + 98390000, depth);
innerBody<4>(stream, sum+ 3 + 4 + 8 + 9, product* 3 * 4 * 8 * 9, index + 98430000, depth);
innerBody<4>(stream, sum+ 7 + 4 + 8 + 9, product* 7 * 4 * 8 * 9, index + 98470000, depth);
innerBody<4>(stream, sum+ 8 + 4 + 8 + 9, product* 8 * 4 * 8 * 9, index + 98480000, depth);
innerBody<4>(stream, sum+ 9 + 4 + 8 + 9, product* 9 * 4 * 8 * 9, index + 98490000, depth);
innerBody<4>(stream, sum+ 8 + 6 + 8 + 9, product* 8 * 6 * 8 * 9, index + 98680000, depth);
innerBody<4>(stream, sum+ 3 + 7 + 8 + 9, product* 3 * 7 * 8 * 9, index + 98730000, depth);
innerBody<4>(stream, sum+ 7 + 7 + 8 + 9, product* 7 * 7 * 8 * 9, index + 98770000, depth);
innerBody<4>(stream, sum+ 8 + 7 + 8 + 9, product* 8 * 7 * 8 * 9, index + 98780000, depth);
innerBody<4>(stream, sum+ 6 + 8 + 8 + 9, product* 6 * 8 * 8 * 9, index + 98860000, depth);
innerBody<4>(stream, sum+ 7 + 8 + 8 + 9, product* 7 * 8 * 8 * 9, index + 98870000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 8 + 9, product* 9 * 8 * 8 * 9, index + 98890000, depth);
innerBody<4>(stream, sum+ 4 + 9 + 8 + 9, product* 4 * 9 * 8 * 9, index + 98940000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 8 + 9, product* 7 * 9 * 8 * 9, index + 98970000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 8 + 9, product* 8 * 9 * 8 * 9, index + 98980000, depth);
innerBody<4>(stream, sum+ 9 + 9 + 8 + 9, product* 9 * 9 * 8 * 9, index + 98990000, depth);
innerBody<4>(stream, sum+ 6 + 4 + 9 + 9, product* 6 * 4 * 9 * 9, index + 99460000, depth);
innerBody<4>(stream, sum+ 7 + 4 + 9 + 9, product* 7 * 4 * 9 * 9, index + 99470000, depth);
innerBody<4>(stream, sum+ 8 + 4 + 9 + 9, product* 8 * 4 * 9 * 9, index + 99480000, depth);
innerBody<4>(stream, sum+ 9 + 4 + 9 + 9, product* 9 * 4 * 9 * 9, index + 99490000, depth);
innerBody<4>(stream, sum+ 2 + 6 + 9 + 9, product* 2 * 6 * 9 * 9, index + 99620000, depth);
innerBody<4>(stream, sum+ 3 + 6 + 9 + 9, product* 3 * 6 * 9 * 9, index + 99630000, depth);
innerBody<4>(stream, sum+ 7 + 6 + 9 + 9, product* 7 * 6 * 9 * 9, index + 99670000, depth);
innerBody<4>(stream, sum+ 8 + 6 + 9 + 9, product* 8 * 6 * 9 * 9, index + 99680000, depth);
innerBody<4>(stream, sum+ 9 + 6 + 9 + 9, product* 9 * 6 * 9 * 9, index + 99690000, depth);
innerBody<4>(stream, sum+ 2 + 7 + 9 + 9, product* 2 * 7 * 9 * 9, index + 99720000, depth);
innerBody<4>(stream, sum+ 3 + 7 + 9 + 9, product* 3 * 7 * 9 * 9, index + 99730000, depth);
innerBody<4>(stream, sum+ 7 + 7 + 9 + 9, product* 7 * 7 * 9 * 9, index + 99770000, depth);
innerBody<4>(stream, sum+ 8 + 7 + 9 + 9, product* 8 * 7 * 9 * 9, index + 99780000, depth);
innerBody<4>(stream, sum+ 9 + 7 + 9 + 9, product* 9 * 7 * 9 * 9, index + 99790000, depth);
innerBody<4>(stream, sum+ 2 + 8 + 9 + 9, product* 2 * 8 * 9 * 9, index + 99820000, depth);
innerBody<4>(stream, sum+ 3 + 8 + 9 + 9, product* 3 * 8 * 9 * 9, index + 99830000, depth);
innerBody<4>(stream, sum+ 4 + 8 + 9 + 9, product* 4 * 8 * 9 * 9, index + 99840000, depth);
innerBody<4>(stream, sum+ 6 + 8 + 9 + 9, product* 6 * 8 * 9 * 9, index + 99860000, depth);
innerBody<4>(stream, sum+ 8 + 8 + 9 + 9, product* 8 * 8 * 9 * 9, index + 99880000, depth);
innerBody<4>(stream, sum+ 9 + 8 + 9 + 9, product* 9 * 8 * 9 * 9, index + 99890000, depth);
innerBody<4>(stream, sum+ 3 + 9 + 9 + 9, product* 3 * 9 * 9 * 9, index + 99930000, depth);
innerBody<4>(stream, sum+ 4 + 9 + 9 + 9, product* 4 * 9 * 9 * 9, index + 99940000, depth);
innerBody<4>(stream, sum+ 6 + 9 + 9 + 9, product* 6 * 9 * 9 * 9, index + 99960000, depth);
innerBody<4>(stream, sum+ 7 + 9 + 9 + 9, product* 7 * 9 * 9 * 9, index + 99970000, depth);
innerBody<4>(stream, sum+ 8 + 9 + 9 + 9, product* 8 * 9 * 9 * 9, index + 99980000, depth);
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
