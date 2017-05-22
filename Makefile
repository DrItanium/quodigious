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

LXXFLAGS = -lpthread
LXXFLAGS += -O3 -flto -fwhole-program -march=native
TITLE = quodigious
FILES = quodigious.o

all: q8 qsinglenumbercheck qloops

help:
	@echo "available options: "
	@echo "  - all : builds the program"
	@echo "  - q8: compiles the program (8 thread version)"
	@echo "  - qloops: compiles the nested loop variant"
	@echo "  - clean : cleans the program artifacts"

q8: quodigious.o
	@echo -n Building ${TITLE} ...
	@${CXX} ${LXXFLAGS} -o ${TITLE} ${FILES}
	@echo done.

qsinglenumbercheck: singlenumbercheck.o
	@echo -n Building single number checker  ...
	@${CXX} ${LXXFLAGS} -o qsinglenumbercheck singlenumbercheck.o
	@echo done.

qloops: loops.o
	@echo -n Building looped quodigious ...
	@${CXX} ${LXXFLAGS} -o qloops loops.o
	@echo done.


%.o: %.cc
	@echo -n Compiling $< into $@ ...
	@${CXX} ${CXXFLAGS} -c $< -o $@
	@echo done.

clean:
	@echo -n cleaning...
	@rm -rf ${FILES} ${TITLE} singlenumbercheck.o qsinglenumbercheck qloops loops.o
	@echo done.

quodigious.o: qlib.h notations.def

singlenumbercheck.o: qlib.h

loops.o: qlib.h
