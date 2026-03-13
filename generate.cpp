#include <cassert>
#include <cmath>
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

void generate_points_json(i32 random_seed, u64 num_pairs) {
  std::string file_name = std::to_string(random_seed) + "_" + std::to_string(num_pairs) + ".json";
  std::ofstream file(file_name);

  std::random_device device; // operater() overload produces a random seed
  std::mt19937_64 rng(device());
  // std::mt19937_64 rng(random_seed);
  std::uniform_real_distribution<f64> distribution(0.0, 1.0);

  file << "{\"pairs\":[";

  f64 sum = 0.0;
  for (u64 i{}; i < num_pairs; i++) {
    f64 x0 = distribution(rng) * 360 - 180;
    f64 x1 = distribution(rng) * 360 - 180;

    f64 y0 = distribution(rng) * 180 - 90;
    f64 y1 = distribution(rng) * 180 - 90;

    std::string x0s = std::to_string(x0);
    std::string y0s = std::to_string(y0);
    std::string x1s = std::to_string(x1);
    std::string y1s = std::to_string(y1);

    latlon a = {
      .lat = x0,
      .lon = y0,
    };

    latlon b = {
      .lat = x1,
      .lon = y1,
    };
    const f64 earth_radius = 6372.8;
    f64 h = reference_haversine(a, b, earth_radius);
    sum += h;

    file << "{\"x0\":" + x0s + ", \"y0\":" + y0s + ", \"x1\":" + x1s + ", \"y1\":" + y1s + "}";
    if (i != num_pairs - 1) file << ",";
    file << "\n";
  }

  sum /= num_pairs;
  std::cout << "average haversine: " << sum << "\n";

  file << "]}";
  file.close();
}

i32 main(i32 argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage: ./haversine <random seed> <number of coordinate pairs to generate>" << "\n";
    return -1;
  }

  i32 random_seed = std::stoi(argv[1]);
  u64 num_pairs = std::stoull(argv[2]);

  std::cout << "generating: " << ", random seed: " << random_seed << ", num pairs: " << num_pairs << "\n";

  generate_points_json(random_seed, num_pairs);
  return 0;
}
