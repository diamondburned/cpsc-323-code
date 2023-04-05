.PHONY: all

CXX ?= g++
CXXFLAGS ?= $(shell echo $$(cat compile_flags.txt))

LIBCXXFILES := $(shell find lib -type f -name '*.cpp')
LIBHXXFILES := $(shell find lib -type f -name '*.hpp')

all: stage1.out

stage1.out: stage1.cpp $(LIBCXXFILES) $(LIBHXXFILES)
	$(CXX) $(CXXFLAGS) -O1 -o $@ $< $(LIBCXXFILES)
