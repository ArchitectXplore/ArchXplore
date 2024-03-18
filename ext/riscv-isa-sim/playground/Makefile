CXX := $(shell which g++)
INCLUDE_PATH := -I/work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/ \
-I/work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/build/
CFLAGS := -std=c++17 -fPIC
default: compile run
compile: try.cpp
	$(CXX) $(CFLAGS) $(INCLUDE_PATH) try.cpp -o try 
run: try
	./try
	