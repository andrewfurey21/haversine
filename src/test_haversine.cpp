
#include "haversine.hpp"
#include "parser.hpp"
#include <cmath>
#include <cstdio>

#include <sys/time.h>
#include <x86intrin.h>

static inline u64 read_cpu_timer() { return __rdtsc(); }

int main(int argc, char ** argv) {
  if (argc != 3) {
    printf("Usage: ./test <json> <outputs>\n");
    return 1;
  }

  f64 epsilon = 0.001;

  const char * json_filename = argv[1];
  const char * outputs_filename = argv[2];

  FILE * json_file = NULL, * outputs_file = NULL;

  u64 open_json_file_start = read_cpu_timer();
  json_file = fopen(json_filename, "r");
  if (json_file == NULL) {
    printf("Could not open json_file\n");
    return 1;
  }
  u64 open_json_file_end = read_cpu_timer();

  u64 open_answer_file_start = read_cpu_timer();
  outputs_file = fopen(outputs_filename, "rb");
  if (outputs_file == NULL) {
    printf("Could not open outputs file\n");
    return 1;
  }
  u64 open_answer_file_end = read_cpu_timer();

  u64 parse_json_start = read_cpu_timer();
  JSONElement * json  = parse_json(json_file);
  u64 parse_json_end = read_cpu_timer();

  u64 calculate_haversine_start = read_cpu_timer();
  JSONElement * pairs = get_object_value(json, "pairs");

  JSONElement * current_pair = pairs;
  u64 counter = 0;
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
  u64 calculate_haversine_end = read_cpu_timer();

  u64 destroy_json_start = read_cpu_timer();
  destroy_json(json);
  fclose(outputs_file);
  fclose(json_file);
  u64 destroy_json_end = read_cpu_timer();


  printf("Success.\n");
  u64 open_json_file_clocks = open_json_file_end - open_json_file_start;
  u64 open_answer_file_clocks = open_answer_file_end - open_answer_file_end;
  u64 parse_json_clocks = parse_json_end - parse_json_start;
  u64 calculate_haversine_clocks = calculate_haversine_end - calculate_haversine_start;
  u64 destroy_json_clocks = destroy_json_end - destroy_json_start;

  u64 total_clocks = open_json_file_clocks + open_answer_file_clocks + parse_json_clocks + calculate_haversine_clocks + destroy_json_clocks;

  printf("Open json file: %f\n", 100.0 * (f64)open_json_file_clocks / total_clocks);
  printf("Open answer file: %f\n", 100.0 * (f64)open_answer_file_clocks / total_clocks);
  printf("Parse json file: %f\n", 100.0 * (f64)parse_json_clocks / total_clocks);
  printf("Calculate haversine: %f\n", 100.0 * (f64)calculate_haversine_clocks / total_clocks);
  printf("Destroy json: %f\n", 100.0 * (f64)destroy_json_clocks / total_clocks);
  return 0;
}
