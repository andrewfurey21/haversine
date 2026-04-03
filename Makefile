all: build generate parser

build:
	mkdir -p build

# Generate haversine json
generate: ./build/generate.o ./build/haversine.o
	g++ ./build/generate.o ./build/haversine.o -o generate

./build/haversine.o: ./src/haversine.cpp ./src/haversine.hpp
	g++ ./src/haversine.cpp -g -O0 -c -o ./build/haversine.o -lm

./build/generate.o: ./src/generate.cpp
	g++ ./src/generate.cpp -g -O0 -c -o ./build/generate.o


# Parse haversine json
parser: ./build/parser.o
	g++ ./build/parser.o -o parser

./build/parser.o: ./src/parser.cpp ./src/parser.hpp
	g++ ./src/parser.cpp -O0 -g -c -o ./build/parser.o

clean:
	rm ./build/*.o generate parser

reset-tests:
	rm ./tests/*
