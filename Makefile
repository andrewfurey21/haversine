all: generate parser

parser: parser.o haversine.o
	g++ parser.o haversine.o -o parser

parser.o: parser.cpp haversine.hpp
	g++ parser.cpp -O0 -g -c -o parser.o

generate: generate.o haversine.o
	g++ generate.o haversine.o -o generate

haversine.o: haversine.cpp haversine.hpp
	g++ haversine.cpp -g -O0 -c -o haversine.o -lm

generate.o: generate.cpp
	g++ generate.cpp -g -O0 -c -o generate.o

clean:
	rm *.o generate

reset:
	rm *.json *.f64
