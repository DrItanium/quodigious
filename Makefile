# Copyright (c) 2017 Joshua Scoggins
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

# Makefile for the quodigious application
CXXFLAGS = -std=c++14
CXXFLAGS += -O3 -march=native -ftemplate-backtrace-limit=0

# enable debugging
#CXXFLAGS += -DDEBUG -g3

LXXFLAGS = -O3 -flto -fwhole-program -march=native

QLOOPS_PROG64 = quodigious64
APPROX_PROG64 = product_approximate_quodigious
PROGS = ${QLOOPS_PROG} ${QLOOPS_PROG64} ${APPROX_PROG64}
all: ${PROGS}

help:
	@echo "available options: "
	@echo "  - all : builds the quodigious programs "
	@echo "  - ${QLOOPS_PROG64}: program to compute 64-bit quodigious values"
	@echo "  - ${APPROX_PROG64}: program to compute probable 64-bit quodigious values by identifying numbers that are divisible by their digit-product"
	@echo "  - clean : cleans the program artifacts"

${QLOOPS_PROG64}: loops64.o
	@echo -n "Building 64-bit number quodigious computer ..."
	@${CXX} -lpthread ${LXXFLAGS} -o ${QLOOPS_PROG64} loops64.o
	@echo done.

${APPROX_PROG64}: approximationQuodigious.o
	@echo -n "Building 64-bit number quodigious computer ..."
	@${CXX} -lpthread ${LXXFLAGS} -o ${APPROX_PROG64} approximationQuodigious.o
	@echo done.


%.o: %.cc
	@echo -n Compiling $< into $@ ...
	@${CXX} ${CXXFLAGS} -c $< -o $@
	@echo done.

clean:
	@echo -n cleaning...
	@rm -rf *.o ${PROGS}
	@echo done.

loops64.o: qlib.h
approximationQuodigious.o: qlib.h
