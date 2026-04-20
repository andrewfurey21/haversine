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
#include "profiler.hpp"

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

inline f64 degrees_to_radians(f64 degrees) {
    return 0.01745329251994329577f * degrees;
}

inline f64 square(f64 a) { return a * a; }
// f64 reference_haversine(const lonlat& a, const lonlat& b, f64 radius);
inline f64 reference_haversine(f64 x0, f64 y0, f64 x1, f64 y1, f64 radius) {
  PROFILE_FUNCTION;
  f64 dlat = degrees_to_radians(y1 - y0);
  f64 dlon = degrees_to_radians(x1 - x0);

  f64 lat1 = degrees_to_radians(y0);
  f64 lat2 = degrees_to_radians(y1);

  f64 z = square(sin(dlat / 2.0)) + cos(lat1) * cos(lat2) * square(sin(dlon / 2.0));
  f64 c = 2.0 * asin(sqrt(z));

  return radius * c;
}

inline void print_success(const std::string& msg) {
  std::cout << "\033[92m" << msg << "\033[0m\n";
}

inline void print_error(const std::string& msg) {
  std::cout << "\033[91m" << msg << "\033[0m\n";
}

#endif
