CXX ?= g++
CXXFLAGS ?= $(shell echo $$(cat compile_flags.txt))

main.out: *.cpp *.hpp
	$(CXX) $(CXXFLAGS) -O1 -o main.out *.cpp
