.PHONY: all run

CXX ?= g++
CXXFLAGS ?= $(shell echo $$(cat compile_flags.txt))

LIBCXXFILES := $(shell find lib -type f -name '*.cpp')
LIBHXXFILES := $(shell find lib -type f -name '*.hpp')

all: main.out

run: main.out
	./main.out program.txt

main.out: main.cpp $(LIBCXXFILES) $(LIBHXXFILES)
	$(CXX) $(CXXFLAGS) -O1 -g -o $@ $< $(LIBCXXFILES)
