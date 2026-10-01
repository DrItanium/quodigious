#include <cstdint>
#include <hip/hip_runtime.h>
#include <iostream>
#include <iomanip>

struct DataContainer final {
	__device__ __host__ constexpr DataContainer(uint64_t number = 0, uint64_t sum = 0, uint64_t product = 1) noexcept : _number(number), _sum(sum), _product(product) { }
	__device__ __host__ constexpr auto getNumber() const noexcept { return _number; }
	__device__ __host__ constexpr auto getSum() const noexcept { return _sum; }
	__device__ __host__ constexpr auto getProduct() const noexcept { return _product; }
private:
	uint64_t _number, 
			 _sum, 
			 _product;
};
__device__ DataContainer computeDataContainer(uint32_t stride) noexcept {
	// we can extract up to 30-bits or 10 digits worth of data
	uint64_t d0 = ((stride & 0b111)) + 2;
#define X(field, shamt) uint64_t field = (((stride >> shamt) & 0b111) + 2)
	X(d1, 3);
	X(d2, 6);
	X(d3, 9);
	X(d4, 12);
	X(d5, 15);
	X(d6, 18);
	X(d7, 21);
	X(d8, 24);
	X(d9, 27);
#undef X
	uint64_t tNum = d0 + 
		(d1 * 10ull) + 
		(d2 * 100ull) + 
		(d3 * 1000ull) + 
		(d4 * 10000ull) +
		(d5 * 100000ull) +
		(d6 * 1000000ull) +
		(d7 * 10000000ull) +
		(d8 * 100000000ull) +
		(d9 * 1000000000ull);
	
	uint64_t tSum = d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9;
	uint64_t tProd = d0 * d1 * d2 * d3 * d4 * d5 * d6 * d7 * d8 * d9;
	return DataContainer{tNum, tSum, tProd};
}
__device__ __host__ bool isQuodigious(uint64_t number, uint64_t sum, uint64_t prod) noexcept {
	return ((number % prod) == 0) && ((number % sum) == 0);
}
__device__ __host__ void checkQuodigious(uint64_t number, uint64_t sum, uint64_t prod) noexcept {
	if (isQuodigious(number, sum, prod)) {
		printf("%lu\n", number);
	}
}
template<uint8_t maxDepth, uint8_t currentDepth, uint64_t factor>
__device__ void doQuodigious(uint64_t number, uint64_t sum, uint64_t product) {
	constexpr auto nextDepth = currentDepth + 1;
	for (auto i = 2; i < 10; ++i) {
		auto tNum = number + (i * factor);
		auto tSum = sum + i;
		auto tProd = product * i;
		if constexpr (nextDepth < maxDepth) {
			doQuodigious<maxDepth, nextDepth, factor * 10>(tNum, tSum, tProd);
		} else {
			checkQuodigious(tNum, tSum, tProd);
		}
	}
}


template<uint8_t maxDepth, uint8_t depth, uint64_t factor>
__global__ void quodigiousKernel(int size = 256 * 128, uint64_t number = 0, uint64_t sum = 0, uint64_t product = 1) {
	//auto stride = blockIdx.x * blockDim.x + threadIdx.x;
	// use the stride to determine a fixed digit pattern to add onto the number to compute.
	// We break the stride up into multiple three bit quantities and generate our data off of that.
	for (int i = blockIdx.x * blockDim.x + threadIdx.x; 
			i < size;
			i += blockDim.x * gridDim.x) {
		auto item = computeDataContainer(i);
		if constexpr (maxDepth == (depth + 10)) {
			checkQuodigious(item.getNumber(), item.getSum(), item.getProduct());
		} else {
			doQuodigious<maxDepth, depth + 10, (factor * 10000000000ull)>(
					number + (item.getNumber() * factor),
					item.getSum() + sum, 
					item.getProduct() * product);
		}
	}
}
// we need to setup a sectional splitting so that each thread actually processes increasingly large numbers as we walk through execution
// generally, doing 5-8 digits in a linear fashion is totally safe so we should setup partitioning breaks.
// We want the host CPU to keep kicking things off to the GPU while the GPU processes the deep numbers
// If the number of digits is less than 12 we should just use a linear algorithm instead of even talking to the GPU
// The break down is as follows:
// < 12 digits then just run in a single threaded way
// 12 digits means that we need to break the operation up into 5 digits -> 7 digits each or 3 | 5 | 4 digits 
// 13 digits means that we need to break the operation up into 1|5|7 or 2|5|6 (or another combination)
// 14 digits means that we need to 
template<uint8_t depth, uint8_t currentDepth = 0, uint64_t factor = 1>
__host__ void performQuodigious(uint64_t number = 0, uint64_t sum = 0, uint64_t product = 1) {
	constexpr auto nextDepth = currentDepth + 1;
	if constexpr (depth < 10) {
		for (auto i = 2; i < 10; ++i) {
			auto tNum = number + (i * factor);
			auto tSum = sum + i;
			auto tProd = product * i;
			if constexpr (nextDepth < depth) {
				performQuodigious<depth, nextDepth, factor * 10>(tNum, tSum, tProd);
			} else {
				if (isQuodigious(tNum, tSum, tProd)) {
					std::cout << "\t\t" << tNum << std::endl;
				}
			}
		}
	} else {
		// starting at depth 10 we just directly dispatch to the GPU, I tested it on an RX9070 and it makes a huge difference execution wise
		static_assert(currentDepth == 0);
		quodigiousKernel<depth, currentDepth, factor><<<256, 128>>>(32768 * 32768);
	} 
}

__host__ bool dispatchQuodigious(uint64_t idx) noexcept {

	switch (idx) {
#define X(index) case index : performQuodigious< index , 0, 1> () ; return true
		X(1);
		X(2);
		X(3);
		X(4);
		X(5);
		X(6);
		X(7);
		X(8);
		X(9);
		X(10);
		X(11);
		X(12);
		X(13);
		X(14);
		X(15);
		X(16);
		X(17);
		X(18);
		X(19);
#undef X
		default:
			std::cerr << "Illegal index " << idx << std::endl;
			return false;
			
	}
}

int main() {
    while(std::cin.good()) {
        uint64_t currentIndex = 0;
        std::cin >> currentIndex;
        if (std::cin.good()) {
			if (!dispatchQuodigious(currentIndex)) {
				return 1;
			}
            std::cout << std::endl;
        }
    }
	return 0;
}
