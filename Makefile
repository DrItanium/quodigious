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
QLOOPS_PROG64 = quodigious64
QUODIGIOUS13 = quodigious13
QUODIGIOUS12 = quodigious12
QUODIGIOUS14 = quodigious14
QUODIGIOUS15 = quodigious15
QUODIGIOUS16 = quodigious16
QUODIGIOUS17 = quodigious17
BINARY_ENCODING_GENERATOR = bingen
PROGS = ${QLOOPS_PROG} ${QLOOPS_PROG32} ${QUODIGIOUS13} ${BINARY_ENCODING_GENERATOR} ${QUODIGIOUS12} ${QUODIGIOUS14} ${QUODIGIOUS15} ${QUODIGIOUS16} ${QUODIGIOUS17} ${QLOOPS_PROG64}
all: ${PROGS}

help:
	@echo "available options: "
	@echo "  - all : builds the quodigious programs "
	@echo "  - ${QLOOPS_PROG}: program to compute 64-bit quodigious values"
	@echo "  - ${QLOOPS_PROG32}: program to compute 32-bit quodigious values"
	@echo "  - ${QLOOPS_PROG64}: program to compute 64-bit quodigious values using no threads"
	@echo "  - ${QUODIGIOUS17}: a program that only computes 17 digits quodigious values"
	@echo "  - ${QUODIGIOUS16}: a program that only computes 16 digits quodigious values"
	@echo "  - ${QUODIGIOUS15}: a program that only computes 15 digits quodigious values"
	@echo "  - ${QUODIGIOUS14}: a program that only computes 14 digits quodigious values"
	@echo "  - ${QUODIGIOUS13}: a program that only computes 13 digits quodigious values"
	@echo "  - ${QUODIGIOUS12}: a program that only computes 12 digits quodigious values"
	@echo "  - ${BINARY_ENCODING_GENERATOR}: a program that generates a binary encoding for the larger value computation"
	@echo "  - tests: runs short regression tests"
	@echo "  - longer_tests: runs longer regression tests (more digits)"
	@echo "  - timed_tests: runs short regression tests"
	@echo "  - timed_longer_tests: runs longer regression tests (more digits)"
	@echo "  - clean : cleans the program artifacts"


${QLOOPS_PROG32}: loops32.o
	@echo -n "Building 32-bit number quodigious computer (single-thread) ..."
	@${CXX} ${LXXFLAGS} -o ${QLOOPS_PROG32} loops32.o
	@echo done.

${QLOOPS_PROG64}: loops64.o
	@echo -n "Building 64-bit number quodigious computer ..."
	@${CXX} -lpthread ${LXXFLAGS} -o ${QLOOPS_PROG64} loops64.o
	@echo done.

${QLOOPS_PROG}: inverted-loops.o cache.bin
	@echo -n Building 64-bit number quodigious inverted loop bodies ...
	@${CXX} -pthread ${LXXFLAGS} -o ${QLOOPS_PROG} inverted-loops.o
	@echo done.



${QUODIGIOUS13}: quodigious13.o cache.bin cache2.bin
	@echo -n Building 13 digit quodigious program ...
	@${CXX} -pthread ${LXXFLAGS} -o ${QUODIGIOUS13} quodigious13.o
	@echo done.

${QUODIGIOUS12}: q12.o cache.bin cache2.bin
	@echo -n Building 12 digit quodigious program ...
	@${CXX} -pthread ${LXXFLAGS} -o ${QUODIGIOUS12} q12.o
	@echo done.

${QUODIGIOUS14}: quodigious14.o cache.bin cache3.bin cache4.bin
	@echo -n Building 14 digit quodigious program ...
	@${CXX} -pthread ${LXXFLAGS} -o ${QUODIGIOUS14} quodigious14.o
	@echo done.
${QUODIGIOUS15}: quodigious15.o cache.bin cache4.bin cache5.bin
	@echo -n Building 15 digit quodigious program ...
	@${CXX} -pthread ${LXXFLAGS} -o ${QUODIGIOUS15} quodigious15.o
	@echo done.

${QUODIGIOUS16}: quodigious16.o cache.bin cache5.bin
	@echo -n Building 16 digit quodigious program ...
	@${CXX} -pthread ${LXXFLAGS} -o ${QUODIGIOUS16} quodigious16.o
	@echo done.

${QUODIGIOUS17}: quodigious17.o cache.bin cache6.bin
	@echo -n Building 17 digit quodigious program ...
	@${CXX} -pthread ${LXXFLAGS} -o ${QUODIGIOUS17} quodigious17.o
	@echo done.

${BINARY_ENCODING_GENERATOR}: binary-encoding.o
	@echo -n Building binary cache generator ...
	@${CXX} ${LXXFLAGS} -o ${BINARY_ENCODING_GENERATOR} binary-encoding.o
	@echo done.

cache.bin: ${BINARY_ENCODING_GENERATOR}
	@echo -n Generating binary cache file ...
	@./${BINARY_ENCODING_GENERATOR} > cache.bin
	@echo done.
cache2.bin: ${BINARY_ENCODING_GENERATOR}
	@echo -n Generating binary cache file of 2 digits ...
	@./${BINARY_ENCODING_GENERATOR} 2 > cache2.bin
	@echo done.
cache3.bin: ${BINARY_ENCODING_GENERATOR}
	@echo -n Generating binary cache file of 3 digits ...
	@./${BINARY_ENCODING_GENERATOR} 3 > cache3.bin
	@echo done.
cache4.bin: ${BINARY_ENCODING_GENERATOR}
	@echo -n Generating binary cache file of 4 digits ...
	@./${BINARY_ENCODING_GENERATOR} 4 > cache4.bin
	@echo done.
cache5.bin: ${BINARY_ENCODING_GENERATOR}
	@echo -n Generating binary cache file of 5 digits ...
	@./${BINARY_ENCODING_GENERATOR} 5 > cache5.bin
	@echo done.
cache6.bin: ${BINARY_ENCODING_GENERATOR}
	@echo -n Generating binary cache file of 6 digits ...
	@./${BINARY_ENCODING_GENERATOR} 6 > cache6.bin
	@echo done.
cache9.bin: ${BINARY_ENCODING_GENERATOR}
	@echo -n Generating binary cache file of 9 digits ...
	@./${BINARY_ENCODING_GENERATOR} 9 > cache9.bin
	@echo done.


timed_tests: ${QLOOPS_PROG}
	@echo Running simple testing suite with time analysis
	@echo "12 digits"
	@time echo 12 | ./${QLOOPS_PROG} | sort | diff -B -- - outputs/qnums12
	@echo "11 digits"
	@time echo 11 | ./${QLOOPS_PROG} | sort | diff -B -- - outputs/qnums11

timed_longer_tests: ${QLOOPS_PROG}
	@echo Running longer tests with time analysis
	@echo "13 digits"
	@time echo 13 | ./${QLOOPS_PROG} | sort | diff -B -- - outputs/qnums13

tests: ${QLOOPS_PROG}
	@echo Running simple testing suite
	@echo "12 digits"
	@echo 12 | ./${QLOOPS_PROG} | sort | diff -B -- - outputs/qnums12
	@echo "11 digits"
	@echo 11 | ./${QLOOPS_PROG} | sort | diff -B -- - outputs/qnums11

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
	@rm -rf *.o ${PROGS} cache.bin cache2.bin cache3.bin cache4.bin cache5.bin cache9.bin cache6.bin
	@echo done.

inverted-loops.o: qlib.h

loops32.o: qlib.h
loops64.o: qlib.h
binary-encoding.o: qlib.h
q12.o: qlib.h
quodigious13.o: qlib.h
quodigious14.o: qlib.h
quodigious15.o: qlib.h
quodigious16.o: qlib.h
quodigious17.o: qlib.h
.PHONY: tests longer_tests
