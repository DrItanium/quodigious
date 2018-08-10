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
CXXFLAGS += -std=c++17 -Ofast -fwhole-program -march=native

# enable debugging
#CXXFLAGS += -DDEBUG -g3

LXXFLAGS = -Ofast -march=native -std=c++17

PRODUCT_COMPUTATION = product-compute
SUM_COMPUTATION = sum-compute
QLOOPS_PROG64 = quodigious64
FREQUENCY_ANALYSIS = fanalysis
ENCODING = ocEncoding 
SIMPLE_LOOPS = simpleLoops 
COMPUTE_NINE_DIGITS = compute9digs
PROGS = ${PRODUCT_COMPUTATION} ${QLOOPS_PROG64} ${SUM_COMPUTATION} ${FREQUENCY_ANALYSIS} ${ENCODING} ${SIMPLE_LOOPS} ${COMPUTE_NINE_DIGITS}
all: ${PROGS}

${FREQUENCY_ANALYSIS}:
	@echo -n "Building 64-bit number quodigious computer with frequency analyzer..."
	@${CXX} -lpthread -flto ${LXXFLAGS} -o ${FREQUENCY_ANALYSIS} numericReduction.cc FrequencyAnalyzer.cc
	@echo done.

${QLOOPS_PROG64}: loops64.o
	@echo -n "Building 64-bit number quodigious computer ..."
	@${CXX} -lpthread ${LXXFLAGS} -o ${QLOOPS_PROG64} loops64.o
	@echo done.


${PRODUCT_COMPUTATION}: product-compute.o
	@echo -n "Building unique product computer ... "
	@${CXX} ${LXXFLAGS} -o ${PRODUCT_COMPUTATION} product-compute.o
	@echo done.


${SUM_COMPUTATION}: sum-compute.o
	@echo -n "Building unique sum computer ... "
	@${CXX} ${LXXFLAGS} -o ${SUM_COMPUTATION} sum-compute.o
	@echo done.


${ENCODING}: octalLikeEncoding.o
	@echo -n "Building special octal computer ... "
	@${CXX} -lpthread ${LXXFLAGS} -o ${ENCODING} octalLikeEncoding.o
	@echo done.

${SIMPLE_LOOPS}: simpleLoops.o
	@echo -n "Building special simple loops computer ... "
	@${CXX} -lpthread ${LXXFLAGS} -o ${SIMPLE_LOOPS} simpleLoops.o
	@echo done.

${COMPUTE_NINE_DIGITS}: ComputeNineDigits.o
	@echo -n "Building nine digit partial computer... "
	@${CXX} ${LXXFLAGS} -o ${COMPUTE_NINE_DIGITS} ComputeNineDigits.o
	@echo done.

%.o: %.cc
	@echo -n Compiling $< into $@ ...
	@${CXX} ${CXXFLAGS} -c $< -o $@
	@echo done.

clean:
	@echo -n cleaning...
	@rm -rf *.o ${PROGS}
	@echo done.

loops64.o: qlib.h Specialization8Digits.cc
numericReduction.o: qlib.h FrequencyAnalyzer.h
FrequencyAnalyzer.o: qlib.h FrequencyAnalyzer.h
octalLikeEncoding.o: qlib.h
ComputeNineDigits.o: qlib.h
