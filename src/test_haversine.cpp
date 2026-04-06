
#include "haversine.hpp"
#include "parser.hpp"
#include <cmath>
#include <cstdio>

#include <string>
#include <unordered_map>

#include <sys/time.h>
#include <x86intrin.h>

class Profiler {
public:
  static Profiler& GetInstance() {
    static Profiler profiler;
    return profiler;
  }

  static void start_timer(const std::string& id) {
    Profiler& p = GetInstance();
    if (p.timings.find(id) != p.timings.end()) {
      p.timings.at(id).first = get_cpu_clocks();
    } else {
      p.timings.insert({id, {get_cpu_clocks(), 0}});
    }
  }

  static void end_timer(const std::string& id) {
    Profiler& p = GetInstance();
    if (p.timings.find(id) != p.timings.end()) {
      std::pair<u64, u64>& times = p.timings.at(id);
      times.second = get_cpu_clocks();
      p.total_clocks += times.second - times.first;
    } else {
      printf("Warning: ended timer at %s without starting.\n", id.data());
      p.timings.insert({id, {0, get_cpu_clocks()}});
    }
  }

  static void print_results() {
    Profiler& p = GetInstance();
    for (auto& block_times : p.timings) {
      const std::string& id = block_times.first;
      const std::pair<u64, u64> times = block_times.second;
      const u64 num_clocks = times.second - times.first;
      const f64 perc = 100.0 * (f64)num_clocks / p.total_clocks;
      printf("%s: clocks: %lu, %f%%\n", id.data(), num_clocks, perc);
    }
  }

private:
  Profiler() : timings(), total_clocks(0ull) {}
  Profiler(const Profiler& other) = delete;
  Profiler(Profiler&& other) = delete;
  Profiler& operator=(const Profiler& other) = delete;
  Profiler& operator=(Profiler&& other) = delete;
  ~Profiler() {}

  static u64 get_cpu_clocks() { return __rdtsc(); }

  std::unordered_map<std::string, std::pair<u64, u64>> timings;
  u64 total_clocks;
};

int main(int argc, char ** argv) {
  if (argc != 3) {
    printf("Usage: ./test <json> <outputs>\n");
    return 1;
  }

  f64 epsilon = 0.001;

  const char * json_filename = argv[1];
  const char * outputs_filename = argv[2];

  FILE * json_file = NULL, * outputs_file = NULL;


  Profiler::start_timer("Open JSON file");
  json_file = fopen(json_filename, "r");
  if (json_file == NULL) {
    printf("Could not open json_file\n");
    return 1;
  }
  Profiler::end_timer("Open JSON file");

  Profiler::start_timer("Open Outputs file");
  outputs_file = fopen(outputs_filename, "rb");
  if (outputs_file == NULL) {
    printf("Could not open outputs file\n");
    return 1;
  }
  Profiler::end_timer("Open Outputs file");

  Profiler::start_timer("Parse JSON");
  JSONElement * json  = parse_json(json_file);
  Profiler::end_timer("Parse JSON");

  Profiler::start_timer("Get Pairs object");
  JSONElement * pairs = get_object_value(json, "pairs");
  Profiler::end_timer("Get Pairs object");

  JSONElement * current_pair = pairs;
  u64 counter = 0;
  Profiler::start_timer("Calculate haversines");
  while (current_pair != NULL) {

    f64 x0 = convert_to_number(get_object_value(current_pair->value, "x0"));
    f64 x1 = convert_to_number(get_object_value(current_pair->value, "x1"));
    f64 y0 = convert_to_number(get_object_value(current_pair->value, "y0"));
    f64 y1 = convert_to_number(get_object_value(current_pair->value, "y1"));

    f64 h = reference_haversine(x0, y0, x1, y1, EARTH_RADIUS);
    f64 actual = 0;
    u64 bytes = fread(&actual, 1, sizeof(actual), outputs_file);
    if (bytes != sizeof(actual) * 1) {
      printf("Error reading actual outputs. Num bytes: %lu, should be %lu\n", bytes, sizeof(actual));
      break;
    }

    if (fabs(actual - h) > epsilon) {
      printf("%f, %f, %f, %f = %f\n", x0, y0, x1, y1, h);
      printf("Error %lu: bad match. %f != %f\n", counter, actual, h);
      break;
    }

    current_pair = current_pair->next;
    counter++;
  }
  Profiler::end_timer("Calculate haversines");

  Profiler::start_timer("Destroy json and close files");
  destroy_json(json);
  fclose(outputs_file);
  fclose(json_file);
  Profiler::end_timer("Destroy json and close files");


  printf("Success.\n");
  Profiler::print_results();
  return 0;
}
