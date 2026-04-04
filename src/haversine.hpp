#ifndef HAVERSINE_HPP
#define HAVERSINE_HPP


#include <cmath>
#include <cassert>
#include <cstdio>

#include <iostream>
#include <fstream>
#include <random>

#include <ostream>
#include <string>

#include "types.hpp"

inline void panic_if(bool expr, const std::string &msg) {
  if (expr) {
    std::cerr << msg << "\n";
    exit(-1);
  }
}

struct lonlat {
  f64 lon;
  f64 lat;
};

const f64 EARTH_RADIUS = 6372.8;

f64 degrees_to_radians(f64 degrees);
f64 square(f64 a);
// f64 reference_haversine(const lonlat& a, const lonlat& b, f64 radius);
f64 reference_haversine(f64 x0, f64 y0, f64 x1, f64 y1, f64 radius);

inline void print_success(const std::string& msg) {
  std::cout << "\033[92m" << msg << "\033[0m\n";
}

inline void print_error(const std::string& msg) {
  std::cout << "\033[91m" << msg << "\033[0m\n";
}

#endif
