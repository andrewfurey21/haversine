all: build generate parser test test_profiled

build:
	mkdir -p build

# Generate haversine json
generate: ./build/generate.o ./build/haversine.o
	g++ -std=c++17 ./build/generate.o ./build/haversine.o -o generate

./build/haversine.o: ./src/haversine.cpp ./src/haversine.hpp
	g++ -std=c++17 ./src/haversine.cpp -O3 -c -o ./build/haversine.o -lm

./build/generate.o: ./src/generate.cpp
	g++ -std=c++17 ./src/generate.cpp -g -O0 -c -o ./build/generate.o


# Parse haversine json
parser: ./build/parser.o
	g++ ./build/parser.o -o parser

./build/parser.o: ./src/parser.cpp ./src/parser.hpp
	g++ -std=c++17 ./src/parser.cpp -O0 -g -c -o ./build/parser.o

clean:
	rm ./build/*.o generate parser

test: ./src/test_haversine.cpp ./src/haversine.cpp ./src/parser.hpp ./src/haversine.hpp
	g++ -std=c++17 ./src/test_haversine.cpp ./src/haversine.cpp -O3 -o test

test_profiled: ./src/test_haversine.cpp ./src/haversine.cpp ./src/parser.hpp ./src/haversine.hpp
	g++ -std=c++17 ./src/test_haversine.cpp ./src/haversine.cpp -O3 -pg -o test_profiled
reset-tests:
	rm ./tests/*

