//  Copyright (c) 2017 Joshua Scoggins
//
//  This software is provided 'as-is', without any express or implied
//  warranty. In no event will the authors be held liable for any damages
//  arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any purpose,
//  including commercial applications, and to alter it and redistribute it
//  freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//  2. Altered source versions must be plainly marked as such, and must not be
//     misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source distribution.

// Perform quodigious checks on numbers using tons of different C++ tricks
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <functional>
#include <future>
#include <map>
#include <vector>
#include "qlib.h"

#include "CL/cl.h"


constexpr auto threadCount = 7;
constexpr auto dimensionCount = 6;
constexpr auto secondaryDimensionCount = 2;
constexpr auto digitCount = 13;
container primaryDataCache[dataCacheSize<6>];
container secondaryDataCache[dataCacheSize<2>];

constexpr auto maxSourceSize = 0x100000;

std::string getErrorString(cl_int error)
{
	switch(error){
		// run-time and JIT compiler errors
		case 0: return "CL_SUCCESS";
		case -1: return "CL_DEVICE_NOT_FOUND";
		case -2: return "CL_DEVICE_NOT_AVAILABLE";
		case -3: return "CL_COMPILER_NOT_AVAILABLE";
		case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
		case -5: return "CL_OUT_OF_RESOURCES";
		case -6: return "CL_OUT_OF_HOST_MEMORY";
		case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
		case -8: return "CL_MEM_COPY_OVERLAP";
		case -9: return "CL_IMAGE_FORMAT_MISMATCH";
		case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
		case -11: return "CL_BUILD_PROGRAM_FAILURE";
		case -12: return "CL_MAP_FAILURE";
		case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
		case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
		case -15: return "CL_COMPILE_PROGRAM_FAILURE";
		case -16: return "CL_LINKER_NOT_AVAILABLE";
		case -17: return "CL_LINK_PROGRAM_FAILURE";
		case -18: return "CL_DEVICE_PARTITION_FAILED";
		case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

				  // compile-time errors
		case -30: return "CL_INVALID_VALUE";
		case -31: return "CL_INVALID_DEVICE_TYPE";
		case -32: return "CL_INVALID_PLATFORM";
		case -33: return "CL_INVALID_DEVICE";
		case -34: return "CL_INVALID_CONTEXT";
		case -35: return "CL_INVALID_QUEUE_PROPERTIES";
		case -36: return "CL_INVALID_COMMAND_QUEUE";
		case -37: return "CL_INVALID_HOST_PTR";
		case -38: return "CL_INVALID_MEM_OBJECT";
		case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
		case -40: return "CL_INVALID_IMAGE_SIZE";
		case -41: return "CL_INVALID_SAMPLER";
		case -42: return "CL_INVALID_BINARY";
		case -43: return "CL_INVALID_BUILD_OPTIONS";
		case -44: return "CL_INVALID_PROGRAM";
		case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
		case -46: return "CL_INVALID_KERNEL_NAME";
		case -47: return "CL_INVALID_KERNEL_DEFINITION";
		case -48: return "CL_INVALID_KERNEL";
		case -49: return "CL_INVALID_ARG_INDEX";
		case -50: return "CL_INVALID_ARG_VALUE";
		case -51: return "CL_INVALID_ARG_SIZE";
		case -52: return "CL_INVALID_KERNEL_ARGS";
		case -53: return "CL_INVALID_WORK_DIMENSION";
		case -54: return "CL_INVALID_WORK_GROUP_SIZE";
		case -55: return "CL_INVALID_WORK_ITEM_SIZE";
		case -56: return "CL_INVALID_GLOBAL_OFFSET";
		case -57: return "CL_INVALID_EVENT_WAIT_LIST";
		case -58: return "CL_INVALID_EVENT";
		case -59: return "CL_INVALID_OPERATION";
		case -60: return "CL_INVALID_GL_OBJECT";
		case -61: return "CL_INVALID_BUFFER_SIZE";
		case -62: return "CL_INVALID_MIP_LEVEL";
		case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
		case -64: return "CL_INVALID_PROPERTY";
		case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
		case -66: return "CL_INVALID_COMPILER_OPTIONS";
		case -67: return "CL_INVALID_LINKER_OPTIONS";
		case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

				  // extension errors
		case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
		case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
		case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
		case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
		case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
		case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
		default: return "Unknown OpenCL error";
	}
}

int main(int argc, char* argv[]) {
	if (!loadDataCache<1>("cache6.bin", primaryDataCache, dataCacheSize<6>) || !loadDataCache<9>("cache2.bin", secondaryDataCache, dataCacheSize<2>)) {
		return 1;
	}
	cl_uint platformIdCount = 0;
	clGetPlatformIDs(0, nullptr, &platformIdCount);

	std::vector<cl_platform_id> platformIds (platformIdCount);
	clGetPlatformIDs(platformIdCount, platformIds.data(), nullptr);
	constexpr auto listSize = dataCacheSize<2> * 4 * 7 * 7 * 7 * 7;
	u64* sum = new u64[listSize];
	u64* product = new u64[listSize];
	u64* value = new u64[listSize];
	u64* result = new u64[listSize];
	// Load the kernel source code into the array source_str
	FILE *fp;
	char *source_str;
	size_t source_size;

	fp = fopen("vector_add_kernel.cl", "r");
	if (!fp) {
		std::cerr << "Failed to load kernel" << std::endl;
		return 1;
	}
	source_str = (char*)malloc(maxSourceSize);
	source_size = fread( source_str, 1, maxSourceSize, fp);
	fclose( fp );
	// Get platform and device information
	cl_platform_id platform_id = nullptr;
	cl_device_id device_id = nullptr;   
	cl_uint ret_num_devices = 0;
	cl_uint ret_num_platforms = 0;
	auto ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 1, &device_id, &ret_num_devices);

	// Create an OpenCL context
	cl_context context = clCreateContext( nullptr, 1, &device_id, nullptr, nullptr, &ret);

	// Create a command queue
	cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
	// Create memory buffers on the device for each vector 
	cl_mem sum_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, listSize * sizeof(u64), nullptr, &ret);
	cl_mem product_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, listSize * sizeof(u64), nullptr, &ret);
	cl_mem value_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, listSize * sizeof(u64), nullptr, &ret);
	cl_mem result_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, listSize * sizeof(u64), nullptr, &ret);
	// Create a program from the kernel source
	auto program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);

	if (ret != CL_SUCCESS) {
		std::cout << "ERROR OCCURRED DURING CREATION OF PROGRAM WITH SOURCE: " << getErrorString(ret) << std::endl;
		return 1;
	}

	// Build the program
	ret = clBuildProgram(program, 1, &device_id, nullptr, nullptr, nullptr);
	if (ret != CL_SUCCESS) {
		size_t len = 0;
		ret = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, nullptr, &len);
		char* buffer = new char[len ];
		for (int i = 0; i < len; ++i) {
			buffer[i] = 0;
		}
		ret = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, len, buffer, nullptr);
		std::cout << "ERROR OCCURRED PROGRAM BUILD : " << getErrorString(ret) << "log: " << buffer << std::endl;
		delete [] buffer;
		return 1;
	}

	// Create the OpenCL kernel
	auto kernel = clCreateKernel(program, "vector_add", &ret);

	if (ret != CL_SUCCESS) {
		std::cout << "ERROR OCCURRED CREATING KERNEL: " << getErrorString(ret) << std::endl;
		return 1;
	}

	// Set the arguments of the kernel
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&sum_mem_obj);
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&product_mem_obj);
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&value_mem_obj);
	ret = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&result_mem_obj);
	for (const auto& outer : primaryDataCache) {
		auto* s = sum;
		auto* p = product;
		auto* v = value;
		for (int i = 2; i < 10; ++i) {
			if (i == 5) {
				continue;
			}
			auto iv = i * fastPow10<12> + outer.value;
			for (int j = 2; j < 10; ++j) {
				if (j == 5) {
					continue;
				}
				auto jv = j * fastPow10<11> + iv;
				for (int h = 2; h < 10; ++h) {
					if (h == 5) {
						continue;
					}
					auto hv = h * fastPow10<8> + jv;
					for (int z = 2; z < 10; ++z) {
						if (z == 5) {
							continue;
						}
						auto zv = z * fastPow10<7> + hv;
						for (auto const& inner : secondaryDataCache) {
							auto nv = inner.value + zv;
							for (int k = 2; k < 9; ++k) {
								if ((k % 2) == 0) {
									*s = i + j + outer.sum + inner.sum + k + h + z;
									*p = i * j * outer.product * inner.product * k * h * z;
									*v = nv + k;
									++s;
									++p;
									++v;
								}
							}
						}
					}
				}
			}
		}
		// Copy the lists A and B to their respective memory buffers
		ret = clEnqueueWriteBuffer(command_queue, sum_mem_obj, CL_TRUE, 0, listSize * sizeof(u64), sum, 0, nullptr, nullptr);
		if (ret != CL_SUCCESS) {
			std::cout << "ERROR OCCURRED: " << getErrorString(ret) << std::endl;
			return 1;
		}
		ret = clEnqueueWriteBuffer(command_queue, product_mem_obj, CL_TRUE, 0, listSize * sizeof(u64), product, 0, nullptr, nullptr);
		if (ret != CL_SUCCESS) {
			std::cout << "ERROR OCCURRED: " << getErrorString(ret) << std::endl;
			return 1;
		}
		ret = clEnqueueWriteBuffer(command_queue, value_mem_obj, CL_TRUE, 0, listSize * sizeof(u64), value, 0, nullptr, nullptr);
		if (ret != CL_SUCCESS) {
			std::cout << "ERROR OCCURRED: " << getErrorString(ret) << std::endl;
			return 1;
		}

		// Execute the OpenCL kernel on the list
		size_t global_item_size[]= {listSize}; // Process the entire lists
		ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, nullptr, global_item_size, nullptr, 0, nullptr, nullptr);

		// Read the memory buffer C on the device to the local variable C
		if (ret != CL_SUCCESS) {
			std::cout << "max work item sizes: " << CL_DEVICE_MAX_WORK_ITEM_SIZES << std::endl;
			std::cout << "Kernel work group size: " << CL_KERNEL_WORK_GROUP_SIZE << std::endl;
			std::cout << "Workgroup max size: " << CL_DEVICE_MAX_WORK_GROUP_SIZE << std::endl;
			std::cout << "CL_DEVICE_MAX_WORK_ITEM_SIZES[0] = " << CL_DEVICE_MAX_WORK_ITEM_SIZES << std::endl;
			std::cout << "ERROR OCCURRED: " << getErrorString(ret) << std::endl;
			return 1;
		}
		ret = clEnqueueReadBuffer(command_queue, result_mem_obj, CL_TRUE, 0, listSize * sizeof(u64), result, 0, nullptr, nullptr);
		if (ret != CL_SUCCESS) {
			std::cout << "ERROR OCCURRED during buffer read: " << getErrorString(ret) << std::endl;
			return 1;
		}

		// Display the result to the screen
		for (int i = 0; i < listSize; ++i) {
			merge(result[i], std::cout);
		}
	}
	// Clean up
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(sum_mem_obj);
	ret = clReleaseMemObject(value_mem_obj);
	ret = clReleaseMemObject(product_mem_obj);
	ret = clReleaseMemObject(result_mem_obj);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);
	delete [] sum;
	delete [] product;
	delete [] value;
	delete [] result;

	return 0;
}
