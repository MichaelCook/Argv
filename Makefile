CC = g++ -Wall -Werror -std=c++17

.PHONY: test
test: test
	$(CC) -g -O0 Argv_test.cpp Argv.cpp
	./a.out
