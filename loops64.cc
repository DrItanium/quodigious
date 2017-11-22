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

template<u64 index>
inline void initialBody() noexcept {
    // we don't want main aware of any details of how computation is performed.
    // This allows the decoupling of details from main and the computation body itself
    body<index>(std::cout);
}

template<>
inline void initialBody<13>() noexcept {
    auto fn = [](auto index) noexcept {
        std::stringstream str;
        body<12>(str, index, index, index * fastPow10<12>);
        return str.str();
    };
    auto t0 = std::async(std::launch::async, fn, 2);
    auto t1 = std::async(std::launch::async, fn, 3);
    auto t2 = std::async(std::launch::async, fn, 4);
    auto t3 = std::async(std::launch::async, fn, 6);
    auto t4 = std::async(std::launch::async, fn, 7);
    auto t5 = std::async(std::launch::async, fn, 8);
    auto t6 = std::async(std::launch::async, fn, 9);
    std::cout << t0.get() << t1.get() << t2.get() << t3.get() << t4.get() << t5.get() << t6.get();
}

template<>
inline void initialBody<14>() noexcept {
    auto fn = [](auto index, auto offset) noexcept {
        std::stringstream str;
        body<12>(str, index + offset, index * offset, (index + offset) * fastPow10<12>);
        return str.str();
    };
    for (u64 i = 2; i < 10; ++i) {
        if (i == 5) {
            continue;
        }
        auto t0 = std::async(std::launch::async, fn, i, 2);
        auto t1 = std::async(std::launch::async, fn, i, 3);
        auto t2 = std::async(std::launch::async, fn, i, 4);
        auto t3 = std::async(std::launch::async, fn, i, 6);
        auto t4 = std::async(std::launch::async, fn, i, 7);
        auto t5 = std::async(std::launch::async, fn, i, 8);
        auto t6 = std::async(std::launch::async, fn, i, 9);
        std::cout << t0.get() << t1.get() << t2.get() << t3.get() << t4.get() << t5.get() << t6.get();
    }
}

template<>
inline void initialBody<15>() noexcept {
    auto fn = [](auto sum, auto product, auto index, auto offset) noexcept {
        std::stringstream str;
        body<12>(str, sum + offset, product * offset, (index + offset) * fastPow10<12>);
        return str.str();
    };
    for (u64 j = 2; j < 10; ++j) {
        if (j == 5) {
            continue;
        }
        auto v0 = j * 10;
        auto s0 = j;
        auto p0 = j;
        for (u64 i = 2; i < 10; ++i) {
            if (i == 5) {
                continue;
            }
            auto v1 = (v0 + i) * 10;
            auto s1 = s0 + i;
            auto p1 = p0 * i;
            auto t0 = std::async(std::launch::async, fn, s1, p1, v1, 2);
            auto t1 = std::async(std::launch::async, fn, s1, p1, v1, 3);
            auto t2 = std::async(std::launch::async, fn, s1, p1, v1, 4);
            auto t3 = std::async(std::launch::async, fn, s1, p1, v1, 6);
            auto t4 = std::async(std::launch::async, fn, s1, p1, v1, 7);
            auto t5 = std::async(std::launch::async, fn, s1, p1, v1, 8);
            auto t6 = std::async(std::launch::async, fn, s1, p1, v1, 9);
            std::cout << t0.get() << t1.get() << t2.get() << t3.get() << t4.get() << t5.get() << t6.get();
        }
    }
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
