//  Copyright (c) 2018 Joshua Scoggins
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

// Perform numeric quodigious checks
#include "qlib.h"
#include <future>
#include <list>
#include <tuple>
struct ComputationRequest {
    ComputationRequest(u64 w, u64 n, u64 s, u64 p) : _width(w), _number(n), _sum(s), _product(p) { };
    ComputationRequest() : ComputationRequest(0,0,0,0) { };
    u64 getWidth() const noexcept { return _width; }
    u64 getNumber() const noexcept { return _number; }
    u64 getSum() const noexcept { return _sum; }
    u64 getProduct() const noexcept { return _product; }
private:
    u64 _width;
    u64 _number;
    u64 _sum;
    u64 _product;
};
template<u64 length>
void body(std::list<u64>& values, u64 sum = 0, u64 product = 1, u64 index = 0) noexcept {
	static_assert(length >= 10, "Cannot go lower than 10 digits!");
    if constexpr (length == 10) {
        if (sum % 3 != 0) {
            return;
        }
        if ((index % product == 0) && (index % sum == 0)) {
            values.emplace_back(index);
        }
    } else if constexpr(length >= 15) {
	    constexpr auto inner = length - 1;
	    constexpr auto next = fastPow10<inner>;
        auto t0 = std::async(std::launch::async, [](auto s, auto p, auto idx) {
                    std::list<u64> _values;
                    for (auto i = 2; i < 5; ++i) {
                        body<inner>(_values, s + i, p * i , idx + (i * next));
                    }
                    return _values;
                }, sum, product, index);
        auto t1 = std::async(std::launch::async, [](auto s, auto p, auto idx) {
                    std::list<u64> _values;
                    for (auto i = 6; i < 10; ++i) {
                        body<inner>(_values, s + i, p * i , idx + (i * next));
                    }
                    return _values;
                }, sum, product, index);
        auto v0 = t0.get();
        values.splice(values.cbegin(), v0);
        auto v1 = t1.get();
        values.splice(values.cbegin(), v1);
    } else {
	    constexpr auto inner = length - 1;
	    constexpr auto next = fastPow10<inner>;
	    // unlike the 64-bit version of this code, doing the 32-bit ints for 9 digit
	    // numbers (this code is not used when you request 64-bit numbers!)
	    // does not require as much optimization. We can walk through digit level
	    // by digit level (even if the digit does not contribute too much to the
	    // overall process!).
	    for (auto i = 2; i < 10; ++i) {
            if (i != 5) {
                body<inner>(values, sum + i, product * i, index + (i * next));
            }
	    }
    }
}
std::list<u64> performComputation(const ComputationRequest& r) {
    std::list<u64> values;
    switch(r.getWidth()) {
        case 11: body<11>(values, r.getSum(), r.getProduct(), r.getNumber()); break;
        case 12: body<12>(values, r.getSum(), r.getProduct(), r.getNumber()); break;
        case 13: body<13>(values, r.getSum(), r.getProduct(), r.getNumber()); break;
        case 14: body<14>(values, r.getSum(), r.getProduct(), r.getNumber()); break;
        case 15: body<15>(values, r.getSum(), r.getProduct(), r.getNumber()); break;
        case 16: body<16>(values, r.getSum(), r.getProduct(), r.getNumber()); break;
        case 17: body<17>(values, r.getSum(), r.getProduct(), r.getNumber()); break;
        default:
                 std::cerr << "Illegal index " << r.getNumber() << std::endl;
                 break;
    }
    return values;
}
int main(int argc, char** argv) {
    if (argc != 2) {
        return 1;
    }
    u64 currentWidth = 0;
    std::string tmp(argv[1]);
    std::istringstream w(tmp);
    w >> currentWidth;
    auto targetWorker = 0u;
    ComputationRequest a, b, c, d;
	while(std::cin.good()) {
        if (targetWorker == 4) {
            auto t0 = std::async(std::launch::async, performComputation, std::ref(a));
            auto t1 = std::async(std::launch::async, performComputation, std::ref(b));
            auto t2 = std::async(std::launch::async, performComputation, std::ref(c));
            auto t3 = std::async(std::launch::async, performComputation, std::ref(d));
            auto v0 = t0.get();
            for (auto v : v0) {
                std::cout << v << std::endl;
            }
            auto v1 = t1.get();
            for (auto v : v1) {
                std::cout << v << std::endl;
            }
            auto v2 = t2.get();
            for (auto v : v2) {
                std::cout << v << std::endl;
            }
            auto v3 = t3.get();
            for (auto v : v3) {
                std::cout << v << std::endl;
            }
            targetWorker = 0;
        } else {
            u64 currentIndex = 0;
            u64 currentSum = 0;
            u64 currentProduct = 0;
            std::cin >> currentSum;
            if (!std::cin.good()) { break; }
            std::cin >> currentProduct;
            if (!std::cin.good()) { break; }
            std::cin >> currentIndex;
            if (!std::cin.good()) { break; }
            switch(targetWorker) {
                case 0:
                    a = ComputationRequest(currentWidth, currentIndex, currentSum, currentProduct);
                    break;
                case 1:
                    b = ComputationRequest(currentWidth, currentIndex, currentSum, currentProduct);
                    break;
                case 2:
                    c = ComputationRequest(currentWidth, currentIndex, currentSum, currentProduct);
                    break;
                case 3:
                    d = ComputationRequest(currentWidth, currentIndex, currentSum, currentProduct);
                    break;
                default:
                    break;
            }
            ++targetWorker;
        }
	}
    if (targetWorker > 0) {
        auto v0 = performComputation(a);
        for (const auto v : v0) {
            std::cout << v << std::endl;
        }
        if (targetWorker > 1) {
            auto v1 = performComputation(b);
            for (const auto v : v1) {
                std::cout << v << std::endl;
            }
            if (targetWorker > 2) {
                auto v2 = performComputation(c);
                for (const auto v : v2) {
                    std::cout << v << std::endl;
                }
                if (targetWorker > 3) {
                    auto v3 = performComputation(d);
                    for (const auto v : v3) {
                        std::cout << v << std::endl;
                    }
                }

            }
        }
    }
	return 0;
}
