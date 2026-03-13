#include <cmath>
#include <cassert>
#include <cstdio>

#include <iostream>
#include <fstream>

#include <ostream>
#include <random>
#include <string>


typedef double f64;
typedef unsigned long long u64;
typedef int i32;

struct latlon {
  f64 lat;
  f64 lon;
};

f64 degrees_to_radians(f64 degrees);
f64 square(f64 a);
f64 reference_haversine(const latlon& a, const latlon& b, f64 radius);

