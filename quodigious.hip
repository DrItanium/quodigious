#include <cstdint>
#include <hip/hip_runtime.h>

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

__global__ void quodigiousKernel(int maxDepth) {
	auto stride = blockIdx.x * blockDim.x + threadIdx.x;
	// assume four digits
	auto d0 = ((stride & 0b111)) + 2;
	auto d1 = (((stride >> 3) & 0b111)) + 2;
	auto d2 = (((stride >> 6) & 0b111)) + 2;
	auto d3 = (((stride >> 9) & 0b111)) + 2;

	auto number = d0 + (d1 * 10) + (d2 * 100) + (d3 * 1000);
	auto sum = d0 + d1 + d2 + d3;
	auto prod = d0 * d1 * d2 * d3;
	doQuodigious(maxDepth, 4, number, sum, prod, 10000);
}

int main() {
	quodigiousKernel<<<64, 64>>>(13);
	hipDeviceSynchronize();
	return 0;
}
