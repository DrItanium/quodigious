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
PROGS = ${QLOOPS_PROG} ${QLOOPS_PROG32} ${QLOOPS_PROG64}
all: ${PROGS}

help:
	@echo "available options: "
	@echo "  - all : builds the quodigious programs "
	@echo "  - ${QLOOPS_PROG}: program to compute 64-bit quodigious values"
	@echo "  - ${QLOOPS_PROG32}: program to compute 32-bit quodigious values"
	@echo "  - ${QLOOPS_PROG64}: program to compute 64-bit quodigious values using no threads"
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
loops64.o: qlib.h
