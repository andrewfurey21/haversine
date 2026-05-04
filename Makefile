all: build generate parser test rep_test test_asm

build:
	mkdir -p build

# Generate haversine json
generate: ./build/generate.o
	g++ -std=c++17 ./build/generate.o -o generate

./build/generate.o: ./src/generate.cpp
	g++ -std=c++17 ./src/generate.cpp -g -lm -O0 -c -o ./build/generate.o


# Parse haversine json
parser: ./build/parser.o
	g++ ./build/parser.o -o parser

./build/parser.o: ./src/parser.cpp ./src/parser.hpp
	g++ -std=c++17 ./src/parser.cpp -O0 -g -lm -c -o ./build/parser.o

clean:
	rm ./build/*.o generate parser

test: ./src/test_haversine.cpp ./src/parser.hpp ./src/haversine.hpp ./src/profiler.hpp
	g++ -std=c++17 ./src/test_haversine.cpp -lm -O0 -g -o test

reset-tests:
	rm ./tests/*

rep_test:
	g++ reptest.cpp -O0 -g -pg -o reptest

test_asm: routines.s test_asm.cpp
	as routines.s -c -o routines.o
	g++ test_asm.cpp -c -o test_asm.o
	g++ test_asm.o routines.o -o test_asm
