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
OPTIMIZATION_FLAGS := -Ofast -fwhole-program -march=native
# enable debugging
#DEBUG_FLAGS := -DDEBUG -g3
CXXFLAGS += -std=c++17 ${OPTIMIZATION_FLAGS} ${DEBUG_FLAGS}


LXXFLAGS = -std=c++17 ${OPTIMIZATION_FLAGS} -flto

PROGRAM = quodigious
PROGRAM2 = lquodigious 
PROGS = ${PROGRAM} ${PROGRAM2}
all: ${PROGS}

${PROGRAM}: quodigious.o
	@echo -n "Building quodigious... "
	@${CXX} ${LXXFLAGS} -o ${PROGRAM} quodigious.o -lpthread
	@echo done.

${PROGRAM2}: linearQuodigious.o
	@echo -n "Building non-threaded quodigious... "
	@${CXX} ${LXXFLAGS} -o ${PROGRAM2} linearQuodigious.o
	@echo done.

%.o: %.cc
	@echo -n Compiling $< into $@ ...
	@${CXX} ${CXXFLAGS} -c $< -o $@
	@echo done.

clean:
	@echo -n cleaning...
	@rm -rf *.o ${PROGS}
	@echo done.

quodigious.o: qlib.h
linearQuodigious.o: qlib.h
