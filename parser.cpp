#include "haversine.hpp"

f64 parse_and_compute(const std::ifstream& json_file) {
  return 0.0;
}

int main(int argc, char ** argv) {
  if (argc != 2 && argc != 3) {
    std::cerr << "Usage: ./parse <json> <answer binary>\n";
    return -1;
  }

  std::string json_name = argv[1];
  std::ifstream json_file;
  json_file.open(json_name);

  if (!json_file.is_open()) {
    std::cerr  << "Error: could not open " << json_name << "\n";
    return -1;
  }

  f64 expected = parse_and_compute(json_file);
  std::cout << "Expected: " << expected << "\n";
  if (argc == 3) {
    std::ifstream binary;
    binary.open(argv[2]);
    f64 actual = 0.0;
    char * f = reinterpret_cast<char *>(&f);
    binary.read(f, sizeof(actual));
    std::cout << "Actual: " << actual << "\n";
  }
}
