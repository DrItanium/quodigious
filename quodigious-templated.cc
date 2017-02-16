#include <iostream>
#include <cstdint>
#include <cmath>
#include <future>
#include <vector>

using u64 = uint64_t;
using vec64 = std::vector<u64>;

inline bool isQuodigious(u64 value, u64 length) noexcept {
	u64 current = value;
	u64 sum = 0;
	u64 prod = 1;
	for (u64 i = 0u; i < length; ++i) {
		u64 result = current % 10u;
		if (result < 2) {
			return false;
		} else {
			sum += result;
			prod *= result;
			current /= 10u;
		}
	}
	return (value % sum == 0) && (value % prod == 0);
}
int performQuodigiousCheck(u64 length, u64 start, u64 end, vec64& results) {
	for (auto value = start; value < end; ++value) {
		if (value % 10 >= 2) {
			if (isQuodigious(value, length)) {
				results.emplace_back(value);
			}
		}
	}
	return 0;
}
template<u64 currentIndex>
inline void quodigiousBody() noexcept {
	static_assert(currentIndex < 20, "Can't do quodigious over 19 digits wide!");
	vec64 l0, l1, l2, l3;
	auto base = static_cast<u64>(2.2 * pow(10, currentIndex - 1));
	auto end = static_cast<u64>(pow(10, currentIndex));

	auto start0 = base;
	auto end0 = end / 4;
	auto fut0 = std::async(std::launch::async, [len = currentIndex, start = start0, end = end0, &l0]() { return performQuodigiousCheck(len, start, end, l0); });
	auto start1 = end0;
	auto end1 = end / 2;
	auto fut1 = std::async(std::launch::async, [len = currentIndex, start = start1, end = end1, &l1]() { return performQuodigiousCheck(len, start, end, l1); });
	auto start2 = end1;
	auto end2 = static_cast<u64>(0.75 * end);
	auto fut2 = std::async(std::launch::async, [len = currentIndex, start = start2, end = end2, &l2]() { return performQuodigiousCheck(len, start, end, l2); });
	auto start3 = end2;
	auto end3 = end;
	auto fut3 = std::async(std::launch::async, [len = currentIndex, start = start3, end = end3, &l3]() { return performQuodigiousCheck(len, start, end, l3); });
	fut0.get();
	fut1.get();
	fut2.get();
	fut3.get();
	for (auto value : l0) {
		std::cout << value << std::endl;
	}
	for (auto value : l1) {
		std::cout << value << std::endl;
	}
	for (auto value : l2) {
		std::cout << value << std::endl;
	}
	for (auto value : l3) {
		std::cout << value << std::endl;
	}
}
template<u64 len, u64 start, u64 end> 
inline void singleThreadBody() noexcept {
	vec64 l0;
	performQuodigiousCheck(len, start, end, l0);
	for (auto value : l0) {
		std::cout << value << std::endl;
	}
}
template<> inline void quodigiousBody<1>() noexcept { singleThreadBody<1,2,10>(); }
template<> inline void quodigiousBody<2>() noexcept { singleThreadBody<2,22,100>(); } 
template<> inline void quodigiousBody<3>() noexcept { singleThreadBody<3,222,1000>(); } 
template<> inline void quodigiousBody<4>() noexcept { singleThreadBody<4,2222,10000>(); } 
template<> inline void quodigiousBody<5>() noexcept { singleThreadBody<5,22222,100000>(); } 
template<> inline void quodigiousBody<6>() noexcept { singleThreadBody<6,222222,1000000>(); } 
template<> inline void quodigiousBody<7>() noexcept { singleThreadBody<7,2222222,10000000>(); } 
template<> inline void quodigiousBody<8>() noexcept { singleThreadBody<8,22222222,100000000>(); } 
template<> inline void quodigiousBody<9>() noexcept { singleThreadBody<9,222222222,1000000000>(); } 




int main() {
	while(std::cin.good()) {
		u64 currentIndex = 0;
		std::cin >> currentIndex;
		if (std::cin.good()) {
			if (currentIndex < 20) {
				switch(currentIndex) {
					case 1:
						quodigiousBody<1>();
						break;
					case 2:
						quodigiousBody<2>();
						break;
					case 3:
						quodigiousBody<3>();
						break;
					case 4:
						quodigiousBody<4>();
						break;
					case 5:
						quodigiousBody<5>();
						break;
					case 6:
						quodigiousBody<6>();
						break;
					case 7:
						quodigiousBody<7>();
						break;
					case 8:
						quodigiousBody<8>();
						break;
					case 9:
						quodigiousBody<9>();
						break;
					case 10:
						quodigiousBody<10>();
						break;
					case 11:
						quodigiousBody<11>();
						break;
					case 12:
						quodigiousBody<12>();
						break;
					case 13:
						quodigiousBody<13>();
						break;
					case 14:
						quodigiousBody<14>();
						break;
					case 15:
						quodigiousBody<15>();
						break;
					case 16:
						quodigiousBody<16>();
						break;
					case 17:
						quodigiousBody<17>();
						break;
					case 18:
						quodigiousBody<18>();
						break;
					case 19:
						quodigiousBody<19>();
						break;
					default:
						throw 0;
				}
				std::cout << std::endl;
			}
		}
	}
	return 0;
}
