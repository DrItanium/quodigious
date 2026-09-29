#include <cstdint>
#include <hip/hip_runtime.h>
#include <iostream>

__device__ __host__ bool isQuodigious(uint64_t number, uint64_t sum, uint64_t prod) noexcept {
	return ((number % prod) == 0) && ((number % sum) == 0);
}
__device__ void doQuodigious(int maxDepth, int depth, uint64_t number, uint64_t sum, uint64_t product, uint64_t factor) {
    auto nextDepth = depth + 1;
    for (auto i = 2; i < 10; ++i) {
        auto tNum = number + (i * factor);
        auto tSum = sum + i;
        auto tProd = product * i;
        if (isQuodigious(tNum, tSum, tProd)) {
			printf("\t\t%lu\n", tNum);
        }
        if (nextDepth < maxDepth) {
			doQuodigious(maxDepth, nextDepth, tNum, tSum, tProd, factor * 10);
        }
    }
}

__global__ void quodigiousKernel(int maxDepth, int depth, uint64_t number, uint64_t sum, uint64_t product, uint64_t factor) {
	auto stride = blockIdx.x * blockDim.x + threadIdx.x;
	// assume four digits
	auto d0 = ((stride & 0b111)) + 2;
	auto d1 = (((stride >> 3) & 0b111)) + 2;
	auto d2 = (((stride >> 6) & 0b111)) + 2;
	auto d3 = (((stride >> 9) & 0b111)) + 2;
	auto d4 = (((stride >> 12) & 0b111)) + 2;

	auto tNum = number + (d0 * factor) + (d1 * 10 * factor) + (d2 * 100 * factor) + (d3 * 1000 * factor)  + (d4 * 10000 * factor);
	auto tSum = d0 + d1 + d2 + d3 + d4 + sum;
	auto tProd = d0 * d1 * d2 * d3 * d4 * product;
	doQuodigious(maxDepth, depth + 5, tNum, tSum, tProd, factor * 100000);
}

int main() {
    while(std::cin.good()) {
        uint64_t currentIndex = 0;
        std::cin >> currentIndex;
        if (std::cin.good()) {
            if ((currentIndex > 5) && (currentIndex < 20)) {
				quodigiousKernel<<<256, 128>>>(currentIndex, 0, 0, 0, 1, 1);
				hipDeviceSynchronize();
            } else {
                std::cout << "Illegal index " << currentIndex << std::endl;
                return 1;
            }
            std::cout << std::endl;
        }
    }
	return 0;
}
