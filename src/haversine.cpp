#include "haversine.hpp"

f64 degrees_to_radians(f64 degrees) {
  return 0.01745329251994329577f * degrees;
}

f64 square(f64 a) { return a * a; }

// f64 reference_haversine(const lonlat& a, const lonlat& b, f64 radius) {
f64 reference_haversine(f64 x0, f64 y0, f64 x1, f64 y1, f64 radius) {
  f64 dlat = degrees_to_radians(y1 - y0);
  f64 dlon = degrees_to_radians(x1 - x0);

  f64 lat1 = degrees_to_radians(y0);
  f64 lat2 = degrees_to_radians(y1);

  f64 z = square(sin(dlat / 2.0)) + cos(lat1) * cos(lat2) * square(sin(dlon / 2.0));
  f64 c = 2.0 * asin(sqrt(z));

  return radius * c;
}
