.PHONY: all clean

CC = gcc
CXX = g++

ifdef DEBUG
	OPT = -g -O0
else
	OPT = -O3
endif

CFLAGS = $(OPT) -Wall
CXXFLAGS = $(OPT) -Wall -std=c++11 -I.

all: weerun

clean:
	rm -f weerun *.o

test: weerun
	./weerun -test

weerun: weerun.o common.o ir.o weerun_dis.o weerun_inst.o weerun_jit.o weerun_parse.o weerun_runtime.o
	$(CXX) -o $@ $^
