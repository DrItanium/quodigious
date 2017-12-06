#include <cstdint>
#include <iostream>
#include <unordered_set>
#include <future>
#include <sstream>
using u64 = std::uint64_t;
using u8 = std::uint8_t;
using unique_set = std::unordered_set<std::uint64_t>;
void computeProduct(unique_set& set, u8 depth, u64 product = 1) noexcept {
	if (depth == 0) {
		set.emplace(product);
	} else if (depth >= 10) {
		auto fn = [](u8 depth, u64 product, int p0, int p1, int p2) noexcept {
			unique_set set;
			auto newDepth = depth - 1;
			computeProduct(set, newDepth , product * p0);
			computeProduct(set, newDepth , product * p1);
			computeProduct(set, newDepth , product * p2);
			return set;
		};

		auto p0 = std::async(std::launch::async, fn, depth, product, 3, 4, 6);
		auto p1 = std::async(std::launch::async, fn, depth, product, 7, 8, 9);

		computeProduct(set, depth - 1, product * 2);
		auto s0 = p0.get();
		for (auto const p : s0) {
			set.emplace(p);
		}
		auto s1 = p1.get();
		for (auto const p : s1) {
			set.emplace(p);
		}
	} else {
		for (int i = 2; i < 10; ++i) {
			if (i != 5) {
				computeProduct(set, depth - 1, product * i);
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
