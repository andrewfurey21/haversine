
#include "haversine.hpp"
#include "parser.hpp"
#include "profiler.hpp"

#include <chrono>

#include <cmath>
#include <cstdio>
#include <ctime>

int main(int argc, char ** argv) {

  using namespace std::chrono;
  time_point<steady_clock> start = steady_clock::now();



  PROFILE_FUNCTION;
  if (argc != 3) {
    printf("Usage: ./test <json> <outputs>\n");
    return 1;
  }

  f64 epsilon = 0.001;

  const char * json_filename = argv[1];
  const char * outputs_filename = argv[2];

  FILE * json_file = NULL, * outputs_file = NULL;


  json_file = fopen(json_filename, "r");
  if (json_file == NULL) {
    printf("Could not open json_file\n");
    return 1;
  }

  outputs_file = fopen(outputs_filename, "rb");
  if (outputs_file == NULL) {
    printf("Could not open outputs file\n");
    return 1;
  }

  JSONElement * json  = parse_json(json_file);

  JSONElement * pairs = get_object_value(json, "pairs");

  JSONElement * current_pair = pairs;
  u64 counter = 0;
  while (current_pair != NULL) {
    PROFILE_BLOCK("calculating haversines");

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

  destroy_json(json);
  fclose(outputs_file);
  fclose(json_file);

  printf("Success.\n");

  time_point<steady_clock> end = steady_clock::now();
  milliseconds wall_time = duration_cast<milliseconds>(end - start);
  std::cout << "Time (in ms): " << wall_time.count() << "\n";
  return 0;
}
