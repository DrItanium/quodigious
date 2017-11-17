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
#include "qlib.h"


constexpr auto threadCount = 7;
constexpr auto dimensionCount = 8;
constexpr auto secondaryDimensionCount = 2;
constexpr auto digitCount = 13;

int main(int argc, char* argv[]) {
	auto* primaryDataCache = new container[dataCacheSize<dimensionCount>];
	auto* secondaryDataCache = new container[dataCacheSize<secondaryDimensionCount>];
	if (!loadDataCache<1>("cache.bin", primaryDataCache, dataCacheSize<dimensionCount>) || !loadDataCache<9>("cache2.bin", secondaryDataCache, dataCacheSize<secondaryDimensionCount>)) {
		return 1;
	}
	doMaskedSuperWorker<digitCount, dimensionCount, secondaryDimensionCount, threadCount>(readInputDescription(), std::cout, primaryDataCache, secondaryDataCache, std::launch::deferred);
	delete [] primaryDataCache;
	delete [] secondaryDataCache;
	return 0;
}
