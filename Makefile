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

PROGS = qloops qloops32
all: ${PROGS}

help:
	@echo "available options: "
	@echo "  - all : builds the program qloops "
	@echo "  - qloops: compiles the nested loop variant"
	@echo "  - qloops32: compiles the nested loop variant for 32-bit only targets"
	@echo "  - tests: runs regression tests"
	@echo "  - clean : cleans the program artifacts"

qloops: loops.o
	@echo -n Building looped quodigious ...
	@${CXX} -pthread ${LXXFLAGS} -o qloops loops.o
	@echo done.

qloops32: loops32.o
	@echo -n Building looped quodigious ...
	@${CXX} ${LXXFLAGS} -o qloops32 loops32.o
	@echo done.


tests: qloops
	@echo Running simple testing suite with time analysis
	@echo "12 digits"
	@time echo 12 | ./qloops | diff -B -- - outputs/qnums12
	@echo "11 digits"
	@time echo 11 | ./qloops | diff -B -- - outputs/qnums11

longer_tests: qloops
	@echo Running longer tests with time analysis
	@echo "13 digits"
	@time echo 13 | ./qloops | diff -B -- - outputs/qnums13

%.o: %.cc
	@echo -n Compiling $< into $@ ...
	@${CXX} ${CXXFLAGS} -c $< -o $@
	@echo done.

clean:
	@echo -n cleaning...
	@rm -rf *.o ${PROGS}
	@echo done.

loops.o: qlib.h

loops32.o: qlib.h

.PHONY: tests longer_tests
