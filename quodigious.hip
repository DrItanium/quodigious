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
        if (nextDepth < depth) {
			doQuodigious(maxDepth, nextDepth, tNum, tSum, tProd, factor * 10);
        }
    }
}
