all: generate.cpp
	g++ generate.cpp -g -O0 -o generate

clean:
	rm generate
