#include "haversine.hpp"

f64 degrees_to_radians(f64 degrees) {
  return 0.01745329251994329577f * degrees;
}

f64 square(f64 a) { return a * a; }

f64 reference_haversine(const latlon& a, const latlon& b, f64 radius) {
  f64 dlat = degrees_to_radians(b.lat - a.lat);
  f64 dlon = degrees_to_radians(b.lon - a.lon);

  f64 lat1 = degrees_to_radians(a.lat);
  f64 lat2 = degrees_to_radians(b.lat);

  f64 z = square(sin(dlat / 2.0)) + cos(lat1) * cos(lat2) * square(sin(dlon / 2.0));
  f64 c = 2.0 * asin(sqrt(z));

  return radius * c;
}
