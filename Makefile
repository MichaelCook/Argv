CC = g++ -Wall -Werror -std=c++17

.PHONY: default
default:
	echo "Use 'make all' to build everything and run the tests"

.PHONY: all
all: ex1 test

ex1: ex1.cpp Argv.cpp Argv.hpp
	$(CC) -O3 -o $@ ex1.cpp Argv.cpp

.PHONY: test
test:
	$(CC) -g -O0 Argv_test.cpp Argv.cpp
	./a.out
