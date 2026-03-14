#include "haversine.hpp"

void generate_points_json(i32 random_seed, u64 num_pairs) {
  std::string json_name =
    std::to_string(random_seed) + "_" + std::to_string(num_pairs) + ".json";
  std::string bin_name =
    std::to_string(random_seed) + "_" + std::to_string(num_pairs) + ".f64";

  std::ofstream json(json_name);
  std::ofstream bin(bin_name);

  std::mt19937_64 rng(random_seed);
  std::uniform_real_distribution<f64> distribution(0.0, 1.0);

  json << "{\"pairs\":[";

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

    lonlat a = {.lon = x0, .lat = y0, };
    lonlat b = { .lon = x1, .lat = y1, };

    f64 h = reference_haversine(a, b, EARTH_RADIUS);
    sum += h;

    json << "{\"x0\":" + x0s + ", \"y0\":" + y0s + ", \"x1\":" + x1s + ", \"y1\":" + y1s + "}";
    if (i != num_pairs - 1) json << ",";
    json << "\n";
  }

  sum /= num_pairs; // TODO: come up with better tests. maybe store all?

  std::cout << "Output: " << sum << "\n";
  const char * f = reinterpret_cast<const char *>(&sum);
  bin.write(f, sizeof(f64));

  json << "]}";
  json.close();
  bin.close();
}

i32 main(i32 argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage: ./generate <random seed> <number of coordinate pairs to generate>\n";
    return -1;
  }

  i32 random_seed = std::stoi(argv[1]);
  u64 num_pairs = std::stoull(argv[2]);

  std::cout << "generating: " << ", random seed: " << random_seed << ", num pairs: " << num_pairs << "\n";

  generate_points_json(random_seed, num_pairs);
  return 0;
}
