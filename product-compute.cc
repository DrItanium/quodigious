#include <cstdint>
#include <iostream>
#include <unordered_set>
#include <sstream>
using u64 = std::uint64_t;
using u8 = std::uint8_t;
using unique_set = std::unordered_set<std::uint64_t>;
constexpr auto skipFive = true;
void computeProduct(unique_set& set, u8 depth) noexcept {
	if (depth == 1) {
		set.emplace(2);
		set.emplace(3);
		set.emplace(4);
		set.emplace(6);
		set.emplace(7);
		set.emplace(8);
		set.emplace(9);
	} else if (depth == 0) {
		set.emplace(1);
	} else {
		unique_set inner;
		computeProduct(inner, depth - 1);
		for (auto const p : inner) {
			for (int i = 2; i < 10; ++i) {
                if constexpr (skipFive) {
                    if (i == 5) {
                        continue;
                    }
                }
				set.emplace(p * i);
			}
		}
	}
}


int main(int argc, char** argv) {
	if (argc == 2) {
		std::string a0(argv[1]);
		std::istringstream number(a0);
		int depth;
		number >> depth;
		std::cerr << "number: " << depth << std::endl;
		if (depth > 19) {
			std::cout << "Please provide a number between zero and 19" << std::endl;
			return 1;
		} else {
			unique_set set;
			computeProduct(set, depth);
			for (const auto a : set) {
				std::cout << a << std::endl;
			}
			return 0;
		}
	} else {
		std::cout << "Please provide at least one number to process to the cmdline args" << std::endl;
		return 1;
	}
}
