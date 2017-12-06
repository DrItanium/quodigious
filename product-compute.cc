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

void computeProductParallel(unique_set& top, u8 depth, u64 product = 1) noexcept {
	// do parallel computation at the top level
	auto merge = [&top](auto& collection) noexcept {
		for (const auto p : collection) {
			top.emplace(p);
		}
	};
	unique_set s0, s1, s2, s3, s4, s5, s6;
	auto p0 = std::async(std::launch::async, computeProduct, std::ref(s0), depth - 1, 2);
	auto p1 = std::async(std::launch::async, computeProduct, std::ref(s1), depth - 1, 3);
	auto p2 = std::async(std::launch::async, computeProduct, std::ref(s2), depth - 1, 4);
	auto p3 = std::async(std::launch::async, computeProduct, std::ref(s3), depth - 1, 6);
	auto p4 = std::async(std::launch::async, computeProduct, std::ref(s4), depth - 1, 7);
	auto p5 = std::async(std::launch::async, computeProduct, std::ref(s5), depth - 1, 8);
	auto p6 = std::async(std::launch::async, computeProduct, std::ref(s6), depth - 1, 9);
	p0.get();
	merge(s0);
	p1.get();
	merge(s1);
	p2.get();
	merge(s2);
	p3.get();
	merge(s3);
	p4.get();
	merge(s4);
	p5.get();
	merge(s5);
	p6.get();
	merge(s6);
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
			computeProductParallel(set, depth);
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
