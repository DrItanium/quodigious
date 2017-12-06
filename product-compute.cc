#include <cstdint>
#include <iostream>
#include <unordered_set>
using u64 = std::uint64_t;
using u8 = std::uint8_t;
using unique_set = std::unordered_set<std::uint64_t>;
void computeProduct(unique_set& set, u8 depth, u64 product = 1) noexcept {
	if (depth == 0) {
		set.emplace(product);
	} else {
		for (int i = 2; i < 10; ++i) {
			if (i != 5) {
				computeProduct(set, depth - 1, product * i);
			}
		}
	}
}

int main() {
	unique_set set;
	computeProduct(set, 11);
	for(const auto a : set) {
		std::cout << a << std::endl;
	}
	return 0;
}
