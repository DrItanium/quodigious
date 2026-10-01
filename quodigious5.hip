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
__device__ DataContainer DataElements5[256*128];
__device__ DataContainer DataElements4[64*64];
__device__ DataContainer DataElements3[32*16];
__global__ void computeDataElements5() {
	auto stride = blockIdx.x * blockDim.x + threadIdx.x;
	// use the stride to determine a fixed digit pattern to add onto the number to compute.
	// We break the stride up into multiple three bit quantities and generate our data off of that.
	printf("%lu\n", blockDim.x * gridDim.x);
	// This means the design is actually based off of the number of threads you dispatch on.
	//
	// This is garbage code so who cares!
	auto d0 = ((stride & 0b111)) + 2;
	auto d1 = (((stride >> 3) & 0b111)) + 2;
	auto d2 = (((stride >> 6) & 0b111)) + 2;
	auto d3 = (((stride >> 9) & 0b111)) + 2;
	auto d4 = (((stride >> 12) & 0b111)) + 2;

	auto tNum = d0 + (d1 * 10) + (d2 * 100) + (d3 * 1000) + (d4 * 10000);
	auto tSum = d0 + d1 + d2 + d3 + d4;
	auto tProd = d0 * d1 * d2 * d3 * d4;
	DataElements5[stride] = DataContainer{tNum, tSum, tProd};
}
__global__ void computeDataElements4() {
	auto stride = blockIdx.x * blockDim.x + threadIdx.x;
	// use the stride to determine a fixed digit pattern to add onto the number to compute.
	// We break the stride up into multiple three bit quantities and generate our data off of that.

	// This means the design is actually based off of the number of threads you dispatch on.
	//
	// This is garbage code so who cares!
	auto d0 = ((stride & 0b111)) + 2;
	auto d1 = (((stride >> 3) & 0b111)) + 2;
	auto d2 = (((stride >> 6) & 0b111)) + 2;
	auto d3 = (((stride >> 9) & 0b111)) + 2;

	auto tNum = d0 + (d1 * 10) + (d2 * 100) + (d3 * 1000);
	auto tSum = d0 + d1 + d2 + d3;
	auto tProd = d0 * d1 * d2 * d3;
	DataElements4[stride] = DataContainer{tNum, tSum, tProd};
}
__global__ void computeDataElements3() {
	auto stride = blockIdx.x * blockDim.x + threadIdx.x;
	// use the stride to determine a fixed digit pattern to add onto the number to compute.
	// We break the stride up into multiple three bit quantities and generate our data off of that.

	// This means the design is actually based off of the number of threads you dispatch on.
	//
	// This is garbage code so who cares!
	auto d0 = ((stride & 0b111)) + 2;
	auto d1 = (((stride >> 3) & 0b111)) + 2;
	auto d2 = (((stride >> 6) & 0b111)) + 2;

	auto tNum = d0 + (d1 * 10) + (d2 * 100);
	auto tSum = d0 + d1 + d2;
	auto tProd = d0 * d1 * d2;
	DataElements3[stride] = DataContainer{tNum, tSum, tProd};
}
__device__ __host__ bool isQuodigious(uint64_t number, uint64_t sum, uint64_t prod) noexcept {
	return ((number % prod) == 0) && ((number % sum) == 0);
}
__device__ __host__ void checkQuodigious(uint64_t number, uint64_t sum, uint64_t prod) noexcept {
	if (isQuodigious(number, sum, prod)) {
		printf("\t\t%lu\n", number);
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
__global__ void quodigiousKernel(uint64_t number = 0, uint64_t sum = 0, uint64_t product = 1) {
	auto stride = blockIdx.x * blockDim.x + threadIdx.x;
	// use the stride to determine a fixed digit pattern to add onto the number to compute.
	// We break the stride up into multiple three bit quantities and generate our data off of that.
	auto item = DataElements5[stride];
	doQuodigious<maxDepth, depth + 5, factor * 100000>(item.getNumber(), item.getSum(), item.getProduct());
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
		quodigiousKernel<depth, currentDepth, factor><<<256, 128>>>();
		(void)hipDeviceSynchronize();
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
	computeDataElements5<<<256, 128>>>();
	computeDataElements4<<<64, 64>>>();
	computeDataElements3<<<32, 16>>>();
	(void)hipDeviceSynchronize();
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
