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

QLOOPS_PROG = quodigious
QLOOPS_PROG32 = quodigious32
PROGS = ${QLOOPS_PROG} ${QLOOPS_PROG32}
all: ${PROGS}

help:
	@echo "available options: "
	@echo "  - all : builds the quodigious programs "
	@echo "  - quodigious: program to compute 64-bit quodigious values"
	@echo "  - quodigious32: program to compute 32-bit quodigious values"
	@echo "  - tests: runs short regression tests"
	@echo "  - longer_tests: runs longer regression tests (more digits)"
	@echo "  - timed_tests: runs short regression tests"
	@echo "  - timed_longer_tests: runs longer regression tests (more digits)"
	@echo "  - clean : cleans the program artifacts"


${QLOOPS_PROG32}: loops32.o
	@echo -n Building 32-bit number quodigious quodigious ...
	@${CXX} ${LXXFLAGS} -o ${QLOOPS_PROG32} loops32.o
	@echo done.

${QLOOPS_PROG}: inverted-loops.o
	@echo -n Building 64-bit number quodigious inverted loop bodies ...
	@${CXX} -pthread ${LXXFLAGS} -o ${QLOOPS_PROG} inverted-loops.o
	@echo done.

timed_tests: ${QLOOPS_PROG}
	@echo Running simple testing suite with time analysis
	@echo "12 digits"
	@time echo 12 | ./${QLOOPS_PROG} | diff -B -- - outputs/qnums12
	@echo "11 digits"
	@time echo 11 | ./${QLOOPS_PROG} | diff -B -- - outputs/qnums11

timed_longer_tests: ${QLOOPS_PROG}
	@echo Running longer tests with time analysis
	@echo "13 digits"
	@time echo 13 | ./${QLOOPS_PROG} | diff -B -- - outputs/qnums13

tests: ${QLOOPS_PROG}
	@echo Running simple testing suite
	@echo "12 digits"
	@echo 12 | ./${QLOOPS_PROG} | diff -B -- - outputs/qnums12
	@echo "11 digits"
	@echo 11 | ./${QLOOPS_PROG} | diff -B -- - outputs/qnums11

longer_tests: ${QLOOPS_PROG}
	@echo Running longer tests
	@echo "13 digits"
	@echo 13 | ./${QLOOPS_PROG} | diff -B -- - outputs/qnums13

%.o: %.cc
	@echo -n Compiling $< into $@ ...
	@${CXX} ${CXXFLAGS} -c $< -o $@
	@echo done.

clean:
	@echo -n cleaning...
	@rm -rf *.o ${PROGS}
	@echo done.

inverted-loops.o: qlib.h

loops32.o: qlib.h

.PHONY: tests longer_tests
