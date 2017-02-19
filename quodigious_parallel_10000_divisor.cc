#include <iostream>
#include <cstdint>
#include <cmath>
#include <future>
#include <vector>

using u64 = uint64_t;
using vec64 = std::vector<u64>;


constexpr auto Len7 = 10000000u;
u64 sums[Len7] = { 0 };
u64 productsLen7[Len7] = { 0 };
bool predicatesLen7[Len7] = { false };
constexpr auto Len6 = 1000000u;
u64 productsLen6[Len6] = { 0 };
bool predicatesLen6[Len6] = { false };
constexpr auto Len5 = 100000u;
u64 productsLen5[Len5] = { 0 };
bool predicatesLen5[Len5] = { false };
constexpr auto Len4 = 10000u;
u64 productsLen4[Len4] = { 0 };
bool predicatesLen4[Len4] = { false };
constexpr auto Len3 = 1000u;
u64 productsLen3[Len3] = { 0 };
bool predicatesLen3[Len3] = { false };

inline void initialize() noexcept {
	// precompute all of the sums and products for 7 digit numbers and below (not 100 or 10 though!)
	// It is super fast to do and only consumes space. That way when we iterate
	// through numbers we can reduce the number of divides, remainders, adds,
	// and multiplies greatly!.
	// we can also express more digits in terms of smaller digit groups
	//
	// There is only one sum array because those values will not change
	// regardless of number width. However, the product and predicate lists
	// must be separate because the extra digits (like 0) are still there and
	// must be accounted for. For example, the number 222 is legal when there
	// are three digits, however when it is four digits then it is 0222 and
	// thus illegal (also the product becomes zero!). Thus we have separate
	// lists for each number width when dealing with products and predicates
	// Len7
	for (int k = 0; k < 10; ++k) {
		auto kPred = k >= 2;
		auto kSum = k;
		auto kMul = k;
		auto kInd = (k * Len6);
		for (int h = 0; h < 10; ++h) {
			auto hPred = (h >= 2) && kPred ;
			auto hSum = h + kSum;
			auto hMul = h * kMul;
			auto hInd = (h * Len5) + kInd;
			for (int w = 0; w < 10; ++w) {
				auto wPred = (w >= 2) && hPred;
				auto wSum = w + hSum;
				auto wMul = w * hMul;
				auto wInd = (w * Len4) + hInd;
				for (int y = 0; y < 10; ++y) {
					auto yPred = (y >= 2) && wPred;
					auto ySum = y + wSum;
					auto yMul = y * wMul;
					auto yInd = (y * Len3) + wInd;
					for (int z = 0; z < 10; ++z) {
						auto zPred = z >= 2 && yPred;
						auto zSum = z + ySum;
						auto zMul = z * yMul;
						auto zInd = (z * 100) + yInd ;
						for (int x = 0; x < 10; ++x) {
							auto outerMul = x * zMul;
							auto combinedInd = (x * 10) + zInd;
							auto outerSum = x + zSum;
							auto outerPredicate = ((x >= 2) && zPred);
							sums[combinedInd + 0] = outerSum;
							productsLen7[combinedInd + 0] = 0;
							predicatesLen7[combinedInd + 0] = false;
							sums[combinedInd + 1] = outerSum + 1;
							productsLen7[combinedInd + 1] = outerMul ;
							predicatesLen7[combinedInd + 1] = false;
							sums[combinedInd + 2] = outerSum + 2;
							productsLen7[combinedInd + 2] = outerMul * 2;
							predicatesLen7[combinedInd + 2] = outerPredicate;
							sums[combinedInd + 3] = outerSum + 3;
							productsLen7[combinedInd + 3] = outerMul * 3;
							predicatesLen7[combinedInd + 3] = outerPredicate; 
							sums[combinedInd + 4] = outerSum + 4;
							productsLen7[combinedInd + 4] = outerMul * 4;
							predicatesLen7[combinedInd + 4] = outerPredicate; 
							sums[combinedInd + 5] = outerSum + 5;
							productsLen7[combinedInd + 5] = outerMul * 5;
							predicatesLen7[combinedInd + 5] = outerPredicate;
							sums[combinedInd + 6] = outerSum + 6;
							productsLen7[combinedInd + 6] = outerMul * 6;
							predicatesLen7[combinedInd + 6] = outerPredicate;
							sums[combinedInd + 7] = outerSum + 7;
							productsLen7[combinedInd + 7] = outerMul * 7;
							predicatesLen7[combinedInd + 7] = outerPredicate;
							sums[combinedInd + 8] = outerSum + 8;
							productsLen7[combinedInd + 8] = outerMul * 8;
							predicatesLen7[combinedInd + 8] = outerPredicate;
							sums[combinedInd + 9] = outerSum + 9;
							productsLen7[combinedInd + 9] = outerMul * 9;
							predicatesLen7[combinedInd + 9] = outerPredicate;
						}
					}
				}
			}
		}
	}
	// Len6
	for (int h = 0; h < 10; ++h) {
		auto hPred = h >= 2;
		auto hSum = h;
		auto hMul = h;
		auto hInd = (h * Len5);
		for (int w = 0; w < 10; ++w) {
			auto wPred = (w >= 2) && hPred;
			auto wMul = w * hMul;
			auto wInd = (w * Len4) + hInd;
			for (int y = 0; y < 10; ++y) {
				auto yPred = (y >= 2) && wPred;
				auto yMul = y * wMul;
				auto yInd = (y * Len3) + wInd;
				for (int z = 0; z < 10; ++z) {
					auto zPred = z >= 2 && yPred;
					auto zMul = z * yMul;
					auto zInd = (z * 100) + yInd ;
					for (int x = 0; x < 10; ++x) {
						auto outerMul = x * zMul;
						auto combinedInd = (x * 10) + zInd;
						auto outerPredicate = ((x >= 2) && zPred);
						productsLen6[combinedInd + 0] = 0;
						predicatesLen6[combinedInd + 0] = false;
						productsLen6[combinedInd + 1] = outerMul ;
						predicatesLen6[combinedInd + 1] = false;
						productsLen6[combinedInd + 2] = outerMul * 2;
						predicatesLen6[combinedInd + 2] = outerPredicate;
						productsLen6[combinedInd + 3] = outerMul * 3;
						predicatesLen6[combinedInd + 3] = outerPredicate; 
						productsLen6[combinedInd + 4] = outerMul * 4;
						predicatesLen6[combinedInd + 4] = outerPredicate; 
						productsLen6[combinedInd + 5] = outerMul * 5;
						predicatesLen6[combinedInd + 5] = outerPredicate;
						productsLen6[combinedInd + 6] = outerMul * 6;
						predicatesLen6[combinedInd + 6] = outerPredicate;
						productsLen6[combinedInd + 7] = outerMul * 7;
						predicatesLen6[combinedInd + 7] = outerPredicate;
						productsLen6[combinedInd + 8] = outerMul * 8;
						predicatesLen6[combinedInd + 8] = outerPredicate;
						productsLen6[combinedInd + 9] = outerMul * 9;
						predicatesLen6[combinedInd + 9] = outerPredicate;
					}
				}
			}
		}
	}
	// Len5
	for (int w = 0; w < 10; ++w) {
		auto wPred = w >= 2;
		auto wMul = w;
		auto wInd = (w * Len4);
		for (int y = 0; y < 10; ++y) {
			auto yPred = (y >= 2) && wPred;
			auto yMul = y * wMul;
			auto yInd = (y * Len3) + wInd;
			for (int z = 0; z < 10; ++z) {
				auto zPred = z >= 2 && yPred;
				auto zMul = z * yMul;
				auto zInd = (z * 100) + yInd ;
				for (int x = 0; x < 10; ++x) {
					auto outerMul = x * zMul;
					auto combinedInd = (x * 10) + zInd;
					auto outerPredicate = ((x >= 2) && zPred);
					productsLen5[combinedInd + 0] = 0;
					predicatesLen5[combinedInd + 0] = false;
					productsLen5[combinedInd + 1] = outerMul ;
					predicatesLen5[combinedInd + 1] = false;
					productsLen5[combinedInd + 2] = outerMul * 2;
					predicatesLen5[combinedInd + 2] = outerPredicate;
					productsLen5[combinedInd + 3] = outerMul * 3;
					predicatesLen5[combinedInd + 3] = outerPredicate; 
					productsLen5[combinedInd + 4] = outerMul * 4;
					predicatesLen5[combinedInd + 4] = outerPredicate; 
					productsLen5[combinedInd + 5] = outerMul * 5;
					predicatesLen5[combinedInd + 5] = outerPredicate;
					productsLen5[combinedInd + 6] = outerMul * 6;
					predicatesLen5[combinedInd + 6] = outerPredicate;
					productsLen5[combinedInd + 7] = outerMul * 7;
					predicatesLen5[combinedInd + 7] = outerPredicate;
					productsLen5[combinedInd + 8] = outerMul * 8;
					predicatesLen5[combinedInd + 8] = outerPredicate;
					productsLen5[combinedInd + 9] = outerMul * 9;
					predicatesLen5[combinedInd + 9] = outerPredicate;
				}
			}
		}
	}
	// Len4
	for (int y = 0; y < 10; ++y) {
		auto yPred = (y >= 2) ;
		auto yMul = y ;
		auto yInd = (y * Len3) ;
		for (int z = 0; z < 10; ++z) {
			auto zPred = z >= 2 && yPred;
			auto zMul = z * yMul;
			auto zInd = (z * 100) + yInd ;
			for (int x = 0; x < 10; ++x) {
				auto outerMul = x * zMul;
				auto combinedInd = (x * 10) + zInd;
				auto outerPredicate = ((x >= 2) && zPred);
				productsLen4[combinedInd + 0] = 0;
				predicatesLen4[combinedInd + 0] = false;
				productsLen4[combinedInd + 1] = outerMul ;
				predicatesLen4[combinedInd + 1] = false;
				productsLen4[combinedInd + 2] = outerMul * 2;
				predicatesLen4[combinedInd + 2] = outerPredicate;
				productsLen4[combinedInd + 3] = outerMul * 3;
				predicatesLen4[combinedInd + 3] = outerPredicate; 
				productsLen4[combinedInd + 4] = outerMul * 4;
				predicatesLen4[combinedInd + 4] = outerPredicate; 
				productsLen4[combinedInd + 5] = outerMul * 5;
				predicatesLen4[combinedInd + 5] = outerPredicate;
				productsLen4[combinedInd + 6] = outerMul * 6;
				predicatesLen4[combinedInd + 6] = outerPredicate;
				productsLen4[combinedInd + 7] = outerMul * 7;
				predicatesLen4[combinedInd + 7] = outerPredicate;
				productsLen4[combinedInd + 8] = outerMul * 8;
				predicatesLen4[combinedInd + 8] = outerPredicate;
				productsLen4[combinedInd + 9] = outerMul * 9;
				predicatesLen4[combinedInd + 9] = outerPredicate;
			}
		}
	}
	// Len3
	for (int z = 0; z < 10; ++z) {
		auto zPred = z >= 2;
		auto zMul = z;
		auto zInd = (z * 100);
		for (int x = 0; x < 10; ++x) {
			auto outerMul = x * zMul;
			auto combinedInd = (x * 10) + zInd;
			auto outerPredicate = ((x >= 2) && zPred);
			productsLen3[combinedInd + 0] = 0;
			predicatesLen3[combinedInd + 0] = false;
			productsLen3[combinedInd + 1] = outerMul ;
			predicatesLen3[combinedInd + 1] = false;
			productsLen3[combinedInd + 2] = outerMul * 2;
			predicatesLen3[combinedInd + 2] = outerPredicate;
			productsLen3[combinedInd + 3] = outerMul * 3;
			predicatesLen3[combinedInd + 3] = outerPredicate; 
			productsLen3[combinedInd + 4] = outerMul * 4;
			predicatesLen3[combinedInd + 4] = outerPredicate; 
			productsLen3[combinedInd + 5] = outerMul * 5;
			predicatesLen3[combinedInd + 5] = outerPredicate;
			productsLen3[combinedInd + 6] = outerMul * 6;
			predicatesLen3[combinedInd + 6] = outerPredicate;
			productsLen3[combinedInd + 7] = outerMul * 7;
			predicatesLen3[combinedInd + 7] = outerPredicate;
			productsLen3[combinedInd + 8] = outerMul * 8;
			predicatesLen3[combinedInd + 8] = outerPredicate;
			productsLen3[combinedInd + 9] = outerMul * 9;
			predicatesLen3[combinedInd + 9] = outerPredicate;
		}
	}
}
/**
 * This is used all over the place, it is the actual code to check to see if a
 * number is actuall quodigious. All of the work before hand is to get the sum
 * and products (if we get this far).
 */
inline constexpr bool performQCheck(u64 value, u64 sum, u64 prod) noexcept {
	return (value % sum == 0) && (value % prod == 0);
}

template<u64 length>
inline bool isQuodigious(u64 value) noexcept {
	// If not explicitly specified, we use this much slower implementation of
	// quodigious but carving out sections that are 4 digits wide. The problem
	// with this default implementation is that it is SLOW becuase of the loop
	// at the bottom as well as the remainder checks before hand. I have
	// optimized it as much as possible but since it is general purpose it
	// requires more operations to complete compared to the specialized
	// versions.
	static constexpr auto count = Len4;
	static constexpr auto digits = 4;
	static constexpr auto remainder = length % digits;
	static constexpr auto len = length - remainder;
	u64 current = value;
	u64 sum = 0;
	u64 product = 1;
	if (remainder == 3) {
		auto result = current % Len3;
		if (!predicatesLen3[result]) {
			return false;
		}
		sum = sums[result];
		product = productsLen3[result]; 
		current /= Len3;
	} else if (remainder == 2) {
		auto tmp = current % 10;
		if (tmp < 2) {
			return false;
		}
		product = tmp;
		current /= 10;

		tmp = current % 10;
		if (tmp < 2) {
			return false;
		}
		sum += tmp;
		product *= tmp;
		current /= 10;

	} else if (remainder == 1) {
		auto tmp = current % 10;
		if (tmp < 2) {
			return false;
		}
		sum = tmp;
		product = tmp;
		current /= 10;
	}
	for (u64 i = 0 ; i < len; i += digits) {
		u64 result = current % count;
		if (!predicatesLen4[result]) {
			return false;
		} 
		product *= productsLen4[result];
		sum += sums[result];
		current /= count; // there will be an extra wasted divide because it won't be used!
	}
	return performQCheck(value, sum, product);
}


template<> inline bool isQuodigious<8>(u64 value) noexcept { 
	// 4 + 4
	auto result = value % Len4;
	if (!predicatesLen4[result]) {
		return false;
	}
	auto sum = sums[result];
	auto product = productsLen4[result];
	auto previous = result;
	auto current = value / Len4;

	result = current % Len4;
	if (previous == result) {
		return performQCheck(value, sum + sum, product * product);
	} else {
		return predicatesLen4[result] && performQCheck(value, sum + sums[result], product * productsLen4[result]);
	}
}

template<> inline bool isQuodigious<9>(u64 value) noexcept { 
	// 6 + 3
	auto result = value % Len3;
	if (!predicatesLen3[result]) {
		return false;
	}
	auto sum = sums[result];
	auto product = productsLen3[result];
	auto current = value / Len3;
	result = current % Len6;
	return predicatesLen6[result] && performQCheck(value, sum + sums[result], product * productsLen6[result]);
}

template<>
inline bool isQuodigious<10>(u64 value) noexcept {
	// 5 + 5
	static constexpr auto count = Len5;
	//5
	auto result = value % count;
	if (!predicatesLen5[result]) {
		return false;
	} 
	auto product = productsLen5[result];
	auto sum = sums[result];
	auto previous = result;
	auto current = value / count;

	//10
	result = current % count;
	if (previous == result) {
		// if the two halves are the same and we got this far then we don't
		// need to check again!
		return performQCheck(value, sum + sum, product * product);
	} else {
		// if they aren't equal, then perform actions as normal
		return predicatesLen5[result] && performQCheck(value, sum + sums[result], product * productsLen5[result]);
	}
}


template<>
inline bool isQuodigious<11>(u64 value) noexcept {
	// 6 + 5
	auto result = value % Len6;
	if (!predicatesLen6[result]) {
		return false;
	}
	auto product = productsLen6[result];
	auto sum = sums[result];
	auto current = value / Len6;

	result = current % Len5;
	return predicatesLen5[result] && performQCheck(value, sum + sums[result], product * productsLen5[result]);
}

template<>
inline bool isQuodigious<12>(u64 value) noexcept {
	// 6 + 6
	static constexpr auto count = Len6;
	//6
	auto result = value % count;
	if (!predicatesLen6[result]) {
		return false;
	} 
	auto product = productsLen6[result];
	auto sum = sums[result];
	auto previous = result;
	auto current = value / count;

	//12
	result = current % count;
	if (previous == result) {
		return performQCheck(value, sum + sum, product * product);
	} else {
		return predicatesLen6[result] && performQCheck(value, sum + sums[result], product * productsLen6[result]);
	}
}

template<>
inline bool isQuodigious<13>(u64 value) noexcept {
	// 6 + 7
	//6
	auto result = value % Len6;
	if (!predicatesLen6[result]) {
		return false;
	} 
	auto product = productsLen6[result];
	auto sum = sums[result];
	auto current = value / Len6;

	// 13
	result = current % Len7;
	return predicatesLen7[result] && performQCheck(value, sum + sums[result], product * productsLen7[result]);
}

template<>
inline bool isQuodigious<14>(u64 value) noexcept {
	// 7 + 7
	//7
	auto result = value % Len7;
	if (!predicatesLen7[result]) {
		return false;
	} 
	auto product = productsLen7[result];
	auto sum = sums[result];
	auto current = value / Len7;

	// 14
	result = current % Len7;
	return predicatesLen7[result] && performQCheck(value, sum + sums[result], product * productsLen7[result]);
}

template<>
inline bool isQuodigious<15>(u64 value) noexcept {
	//5 + 5 + 5
	auto result = value % Len5;
	if (!predicatesLen5[result]) {
		return false;
	}
	u64 product = productsLen5[result];
	u64 sum = sums[result];
	u64 current = value / Len5;
	//8
	result = current % Len5;
	if (!predicatesLen5[result]) {
		return false;
	} 
	product *= productsLen5[result];
	sum += sums[result];
	current /= Len5;

	//15
	result = current % Len5;
	return predicatesLen5[result] && performQCheck(value, sum + sums[result], product * productsLen5[result]);
}

template<u64 length>
inline int performQuodigiousCheck(u64 start, u64 end, vec64& results) noexcept {
	// assume that we start at 2.222222222222
	// skip over the 9th and 10th numbers from this position!
	auto fn = [&results](u64 value) { if (isQuodigious<length>(value)) { results.emplace_back(value); }};
	for (auto value = start; value < end; value += 10) {
		// if we can compute the upper portion of the number ahead of time and
		// see if it is legal then we could save a ton on checking digits
		fn(value+0);
		fn(value+1);
		fn(value+2);
		fn(value+3);
		fn(value+4);
		fn(value+5);
		fn(value+6);
		fn(value+7);
	}
	return 0;
}
template<u64 length>
inline constexpr u64 fastPow10() noexcept {
	return fastPow10<length - 1>() * 10;
}
template<>
inline constexpr u64 fastPow10<0>() noexcept {
	return 1;
}
template<>
inline int performQuodigiousCheck<10>(u64 start, u64 end, vec64& results) noexcept {
	// compute the upper, lower, and 1 bit locations
	// this should always be at position 2!
	static constexpr auto factor5Wide = fastPow10<5>();
	static constexpr auto factor4Wide = fastPow10<1>();
	static constexpr auto oneDigit = fastPow10<1>();
	static constexpr auto fourDigits = fastPow10<4>();
	static constexpr auto fiveDigits = fastPow10<5>();
	auto start1Wide = start % oneDigit;
	auto start4Wide = ((start / oneDigit) % fourDigits);
	auto start5Wide = (((start / oneDigit) / fourDigits) % fiveDigits);
	auto end5Wide = ((end / oneDigit) / fourDigits) % fiveDigits;
	if (end5Wide == 0) {
		end5Wide = fiveDigits;
	}
	for (auto i = start5Wide; i < end5Wide; ++i) {
		if (predicatesLen5[i]) {
			// okay, now compute this out
			// this is the upper 5 bits so we have to do
			auto upperSum = sums[i];
			auto upperProduct = productsLen5[i];
			auto upperIndex = i * factor5Wide;
			for (auto j = start4Wide; j < fourDigits; ++j) {
				if (predicatesLen4[j]) {
					// okay, now we're in the right spot
					auto innerSum = sums[j] + upperSum;
					auto innerProduct = productsLen4[j] * upperProduct;
					auto innerIndex = (j * factor4Wide) + upperIndex;
					// okay, start at two here
					for (auto k = start1Wide; k < oneDigit; ++k) {
						// let's just be concise right now
						auto product = innerProduct * k;
						auto sum = innerSum + k;
						auto value = innerIndex + k;
						if (performQCheck(value, sum, product)) {
							results.emplace_back(value);
						}
					}
				}
			}
		}
	}
	return 0;
}

template<>
inline int performQuodigiousCheck<11>(u64 start, u64 end, vec64& results) noexcept {
	// compute the upper, lower, and 1 bit locations
	// this should always be at position 2!
	static constexpr auto upperFactor = fastPow10<6>();
	static constexpr auto lowerFactor = fastPow10<1>();
	static constexpr auto oneDigit = fastPow10<1>();
	static constexpr auto fiveDigits = fastPow10<5>();
	static constexpr auto fourDigits = fastPow10<4>();
	auto start1Wide = start % oneDigit;
	auto current = start / oneDigit;
	auto startInner = ((start / oneDigit) % fiveDigits);
	auto startOuter = (((start / oneDigit) / fiveDigits) % fiveDigits);
	auto endOuter = ((end / oneDigit) / fiveDigits) % fiveDigits;
	if (endOuter == 0) {
		endOuter = fiveDigits;
	}
	for (auto i = startOuter; i < endOuter; ++i) {
		if (predicatesLen5[i]) {
			// okay, now compute this out
			// this is the upper 5 bits so we have to do
			auto upperSum = sums[i];
			auto upperProduct = productsLen5[i];
			auto upperIndex = i * upperFactor;
			for (auto j = startInner; j < fiveDigits; ++j) {
				if (predicatesLen5[j]) {
					// okay, now we're in the right spot
					auto innerSum = sums[j] + upperSum;
					auto innerProduct = productsLen5[j] * upperProduct;
					auto innerIndex = (j * lowerFactor)  + upperIndex;
					// okay, start at two here
					for (auto k = start1Wide; k < oneDigit; ++k) {
						// let's just be concise right now
						auto product = innerProduct * k;
						auto sum = innerSum + k;
						auto value = innerIndex + k;
						if (performQCheck(value, sum, product)) {
							results.emplace_back(value);
						}
					}
				}
			}
		}
	}
	return 0;
}


template<>
inline int performQuodigiousCheck<12>(u64 start, u64 end, vec64& results) noexcept {
	// -------------
	// | 5 | 6 | 1 |
	// -------------
	static constexpr auto outerFactor = fastPow10<5>();
	static constexpr auto innerFactor = fastPow10<6>();
	static constexpr auto innerMostFactor = fastPow10<1>();
	static constexpr auto upperFactor = fastPow10<7>();
	static constexpr auto lowerFactor = fastPow10<1>();
	auto startInnerMost = start % innerMostFactor;
	auto current = start / innerMostFactor;
	auto startInner = current % innerFactor;
	current /= innerFactor;
	auto startOuter = current % outerFactor;
	auto endInner = ((end / innerMostFactor) % innerFactor);
	if (endInner == 0) {
		endInner = innerFactor;
	}
	auto endInnerMost = innerMostFactor;
	auto innerPredicates = predicatesLen6;
	auto innerProducts = productsLen6;
	auto outerPredicates = predicatesLen5;
	auto outerProducts = productsLen5;

	auto endOuter = ((end / innerMostFactor) / innerFactor) % outerFactor;
	if (endOuter == 0) {
		endOuter = outerFactor;
	}
	for (auto i = startOuter; i < endOuter; ++i) {
		if (outerPredicates[i]) {
			auto upperSum = sums[i];
			auto upperProduct = outerProducts[i];
			auto upperIndex = i * upperFactor;
			for (auto j = startInner; j < endInner; ++j) {
				if (innerPredicates[j]) {
					auto innerSum = sums[j] + upperSum;
					auto innerProduct = innerProducts[j] * upperProduct;
					auto innerIndex = (j * lowerFactor) + upperIndex;
					for (auto k = startInnerMost; k < endInnerMost; ++k) {
						auto product = innerProduct * k;
						auto sum = innerSum + k;
						auto value = innerIndex + k;
						if (performQCheck(value, sum, product)) {
							results.emplace_back(value);
						}
					}
				}
			}
		}
	}
	return 0;
}

template<>
inline int performQuodigiousCheck<13>(u64 start, u64 end, vec64& results) noexcept {
	// -------------
	// | 6 | 6 | 1 |
	// -------------
	static constexpr auto outerFactor = fastPow10<6>();
	static constexpr auto innerFactor = fastPow10<6>();
	static constexpr auto innerMostFactor = fastPow10<1>();
	static constexpr auto upperFactor = fastPow10<7>();
	static constexpr auto lowerFactor = fastPow10<1>();
	auto startInnerMost = start % innerMostFactor;
	auto current = start / innerMostFactor;
	auto startInner = current % innerFactor;
	current /= innerFactor;
	auto startOuter = current % outerFactor;
	auto endInner = ((end / innerMostFactor) % innerFactor);
	if (endInner == 0) {
		endInner = innerFactor;
	}
	auto endInnerMost = innerMostFactor;
	auto innerPredicates = predicatesLen6;
	auto innerProducts = productsLen6;
	auto outerPredicates = predicatesLen6;
	auto outerProducts = productsLen6;

	auto endOuter = ((end / innerMostFactor) / innerFactor) % outerFactor;
	if (endOuter == 0) {
		endOuter = outerFactor;
	}
	for (auto i = startOuter; i < endOuter; ++i) {
		if (outerPredicates[i]) {
			auto upperSum = sums[i];
			auto upperProduct = outerProducts[i];
			auto upperIndex = i * upperFactor;
			for (auto j = startInner; j < endInner; ++j) {
				if (innerPredicates[j]) {
					auto innerSum = sums[j] + upperSum;
					auto innerProduct = innerProducts[j] * upperProduct;
					auto innerIndex = (j * lowerFactor) + upperIndex;
					for (auto k = startInnerMost; k < endInnerMost; ++k) {
						auto product = innerProduct * k;
						auto sum = innerSum + k;
						auto value = innerIndex + k;
						if (performQCheck(value, sum, product)) {
							results.emplace_back(value);
						}
					}
				}
			}
		}
	}
	return 0;
}



template<>
inline int performQuodigiousCheck<7>(u64 start, u64 end, vec64& results) noexcept {
	for (auto value = start; value < end; ++value) {
		if (predicatesLen7[value] && performQCheck(value, sums[value], productsLen7[value])) {
			results.emplace_back(value);
		}
	}
	return 0;
}


void printout(vec64& l) noexcept {
	for (auto v : l) {
		std::cout << v << std::endl;
	}
	l.clear();
}

template<u64 length>
inline void body() noexcept {
	static vec64 l0, l1, l2, l3, l4, l5, l6, l7;
	static constexpr auto factor = 2.0 + (2.0 / 9.0);
	static constexpr auto skip5 = length > 4 && length < 14;
	auto base = static_cast<u64>(pow(10, length - 1));
	auto st = static_cast<u64>(factor * base);

	auto fut0 = std::async(std::launch::async, [start = st, end = 3 * base]() { return performQuodigiousCheck<length>(start, end, l0); });
	auto fut1 = std::async(std::launch::async, [start = st + base, end = 4 * base]() { return performQuodigiousCheck<length>(start, end, l1); });
	auto fut2 = std::async(std::launch::async, [start = st + (base * 2), end = 5 * base]() { return performQuodigiousCheck<length>( start, end, l2); });

	auto fut3 = std::async(std::launch::async, [start = st + (base * 3), end = 6 * base]() { return skip5 ? 0 : performQuodigiousCheck<length>( start, end, l3); });

	auto fut4 = std::async(std::launch::async, [start = st + (base * 4), end = 7 * base]() { return performQuodigiousCheck<length>( start, end, l4); });
	auto fut5 = std::async(std::launch::async, [start = st + (base * 5), end = 8 * base]() { return performQuodigiousCheck<length>( start, end, l5); });
	auto fut6 = std::async(std::launch::async, [start = st + (base * 6), end = 9 * base]() { return performQuodigiousCheck<length>( start, end, l6); });
	performQuodigiousCheck<length>(st + (base * 7), 10 * base, l7);
	fut0.get();
	fut1.get();
	fut2.get();
	fut3.get();
	fut4.get();
	fut5.get();
	fut6.get();
	printout(l0);
	printout(l1);
	printout(l2);
	printout(l3);
	printout(l4);
	printout(l5);
	printout(l6);
	printout(l7);
	std::cout << std::endl;
}

template<>
inline void body<1>() noexcept {
	std::cout << 2 << std::endl;
	std::cout << 3 << std::endl;
	std::cout << 4 << std::endl;
	std::cout << 5 << std::endl;
	std::cout << 6 << std::endl;
	std::cout << 7 << std::endl;
	std::cout << 8 << std::endl;
	std::cout << 9 << std::endl;
	std::cout << std::endl;
}
template<>
inline void body<2>() noexcept {
	static constexpr auto divisor = 10u;
	for (auto value = 22; value < 100; ++value) {
		auto result = value % divisor;
		if (result >= 2) {
			u64 temp = result;
			result = (value / divisor) % divisor;
			if ((result >= 2) && performQCheck(value, temp + result, temp * result)) {
				std::cout << value << std::endl;
			}
		}
	}
	std::cout << std::endl;
}

template<>
inline void body<3>() noexcept {
	for (auto value = 222u; value < 1000u; ++value) {
		if (predicatesLen3[value] && performQCheck(value, sums[value], productsLen3[value])) {
			std::cout << value << std::endl;
		}
	}
	std::cout << std::endl;
}

template<>
inline void body<4>() noexcept {
	for (auto value = 2222u; value < 10000u; ++value) {
		if (predicatesLen4[value] && performQCheck(value, sums[value], productsLen4[value])) {
			std::cout << value << std::endl;
		}
	}
	std::cout << std::endl;
}

template<>
inline void body<5>() noexcept {
	for (auto value = 22222u; value < 100000u; ++value) {
		if (predicatesLen5[value] && performQCheck(value, sums[value], productsLen5[value])) {
			std::cout << value << std::endl;
		}
	}
	std::cout << std::endl;
}

template<>
inline void body<6>() noexcept {
	auto fn = [](auto start, auto end) {
		for (auto value = start; value < end; ++value) {
			if (predicatesLen6[value] && performQCheck(value, sums[value], productsLen6[value])) {
				std::cout << value << std::endl;
			}
		}
	};
	fn(222222u, 300000u);
	fn(322222u, 400000u);
	fn(422222u, 500000u);
	// skip the 500000 - 622222 range
	fn(622222u, 700000u);
	fn(722222u, 800000u);
	fn(822222u, 900000u);
	fn(922222u, 1000000u);
	std::cout << std::endl;
}

/*
template<>
inline void body<11>() noexcept {
	auto fn = [](auto start, auto end) {
		static vec64 tmp;
		performQuodigiousCheck<11>(start, end, tmp);
	};
	fn(22222222222u, 30000000000u);
	fn(32222222222u, 40000000000u);
	fn(42222222222u, 50000000000u);
	fn(62222222222u, 70000000000u);
	fn(72222222222u, 80000000000u);
	fn(82222222222u, 90000000000u);
	fn(92222222222u, 100000000000u);
	std::cout << std::endl;
}
*/

int main() {
	initialize();
	while(std::cin.good()) {
		u64 currentIndex = 0;
		std::cin >> currentIndex;
		if (std::cin.good()) {
			switch(currentIndex) {
				case 1: body<1>(); break;
				case 2: body<2>(); break;
				case 3: body<3>(); break;
				case 4: body<4>(); break;
				case 5: body<5>(); break;
				case 6: body<6>(); break;
				case 7: body<7>(); break;
				case 8: body<8>(); break;
				case 9: body<9>(); break;
				case 10: body<10>(); break;
				case 11: body<11>(); break;
				case 12: body<12>(); break;
				case 13: body<13>(); break;
				case 14: body<14>(); break;
				case 15: body<15>(); break;
				case 16: body<16>(); break;
				case 17: body<17>(); break;
				case 18: body<18>(); break;
				case 19: body<19>(); break;
				default:
						 std::cerr << "Illegal index " << currentIndex << std::endl;
						 return 1;
			}
		}
	}
	return 0;
}
