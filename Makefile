.PHONY: all clean

CC = gcc
CXX = g++

ifdef DEBUG
	OPT = -g -O0
else
	OPT = -O3
endif

CFLAGS = $(OPT) -Wall
CXXFLAGS = $(OPT) -Wall -std=c++11 -Wno-extern-c-compat -I.

all: weerun

clean:
	rm -f weerun *.o

test: weerun
	./weerun -test

#weerun: vm.h weerun.c common.h common.c ir.c weewasm.h illegal.h weerun.hpp weerun_dis.cpp weerun_inst.cpp weerun_parse.cpp weerun_runtime.cpp
#	cc -g -O0 -Wall -c weerun.c common.c ir.c
#	c++ -g -O0 -Wall -std=c++11 -Wno-extern-c-compat -o weerun -I. \
#		weerun.o common.o ir.o \
#		weerun_dis.cpp weerun_inst.cpp weerun_parse.cpp weerun_jit.cpp weerun_runtime.cpp
#
weeify: vm.h weeify.c common.h common.c weewasm.h illegal.h
	cc -o weeify weeify.c common.c

weerun: weerun.o common.o ir.o weerun_dis.o weerun_inst.o weerun_jit.o weerun_parse.o weerun_runtime.o
	$(CXX) -o $@ $^
