#include <cstdint>
#include <hip/hip_runtime.h>
#include <iostream>
#include <iomanip>

__device__ __host__ bool isQuodigious(uint64_t number, uint64_t sum, uint64_t prod) noexcept {
	return ((number % prod) == 0) && ((number % sum) == 0);
}
__device__ void doQuodigious(int maxDepth, int depth, uint64_t number, uint64_t sum, uint64_t product, uint64_t factor) {
    auto nextDepth = depth + 1;
    for (auto i = 2; i < 10; ++i) {
        auto tNum = number + (i * factor);
        auto tSum = sum + i;
        auto tProd = product * i;
        if (nextDepth < maxDepth) {
			doQuodigious(maxDepth, nextDepth, tNum, tSum, tProd, factor * 10);
        } else {
			if (isQuodigious(tNum, tSum, tProd)) {
				printf("\t\t%lu\n", tNum);
			}
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

	auto tNum = number + (d0 * factor) + (d1 * factor * 10) + (d2 * factor * 100) + (d3 * factor * 1000);
	auto tSum = d0 + d1 + sum + d2 + d3;
	auto tProd = d0 * d1 * product * d2 * d3;
	doQuodigious(maxDepth, depth + 4, tNum, tSum, tProd, factor * 10000);
}

__host__ void performQuodigious(uint8_t depth, uint8_t currentDepth, uint64_t number, uint64_t sum, uint64_t product, uint64_t factor) {
    auto nextDepth = currentDepth + 1;
    for (auto i = 2; i < 10; ++i) {
        auto tNum = number + (i * factor);
        auto tSum = sum + i;
        auto tProd = product * i;
        if (nextDepth < depth) {
			switch (nextDepth) {
				case 8 ... 19:
					if ((depth - nextDepth) > 4) {
						quodigiousKernel<<<64, 64>>>(depth, nextDepth, tNum, tSum, tProd, factor * 10);
					} else {
						performQuodigious( depth, nextDepth, tNum, tSum, tProd, factor * 10);
					}
					break;
				default:
            		performQuodigious( depth, nextDepth, tNum, tSum, tProd, factor * 10);
					break;
			}
        } else {
			if (isQuodigious(tNum, tSum, tProd)) {
				std::cout << "\t\t" << tNum << std::endl;
			}
		}
    }
}

int main() {
    while(std::cin.good()) {
        uint64_t currentIndex = 0;
        std::cin >> currentIndex;
        if (std::cin.good()) {
            if ((currentIndex > 0) && (currentIndex < 20)) {
				performQuodigious(currentIndex, 0, 0, 0, 1, 1);
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
