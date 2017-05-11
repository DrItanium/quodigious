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
CXXFLAGS = -std=c++14 -O3 
# enable debugging
#CXXFLAGS += -DDEBUG -g3

LXXFLAGS = -lpthread -O3 -flto -fwhole-program 
TITLE = quodigious
FILES = quodigious.o

all: q8 qsinglenumbercheck

help:
	@echo "available options: "
	@echo "  - all : builds the program"
	@echo "  - q8: compiles the program (8 thread version)"
	@echo "  - clean : cleans the program artifacts"

q8: quodigious.o
	@echo -n Building ${TITLE} ...
	@${CXX} ${LXXFLAGS} -o ${TITLE} ${FILES}
	@echo done.

qsinglenumbercheck: singlenumbercheck.o
	@echo -n Building ${TITLE} ...
	@${CXX} ${LXXFLAGS} -o qsinglenumbercheck singlenumbercheck.o
	@echo done.



%.o: %.cc
	@echo -n Compiling $< into $@ ...
	@${CXX} ${CXXFLAGS} -c $< -o $@
	@echo done.

clean:
	@echo -n cleaning...
	@rm -rf ${FILES} ${TITLE} singlenumbercheck.o qsinglenumbercheck
	@echo done.

quodigious.o: qlib.h notations.def

singlenumbercheck.o: qlib.h
