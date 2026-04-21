
#include "haversine.hpp"
#include "parser.hpp"
#include "profiler.hpp"

#include <cmath>
#include <cstdio>
#include <ctime>

int main(int argc, char ** argv) {
  if (argc != 3) {
    printf("Usage: ./test <json> <outputs>\n");
    return 1;
  }

  f64 epsilon = 0.001;

  const char * json_filename = argv[1];
  const char * outputs_filename = argv[2];

  FileBuffer json_file(json_filename);
  FileBuffer outputs_file(outputs_filename);

  PROFILE_BANDWIDTH("main", json_file.size() + outputs_file.size());

  JSONElement * json  = parse_json(json_file);

  JSONElement * pairs = get_object_value(json, "pairs");

  JSONElement * current_pair = pairs;
  u64 counter = 0;
  while (current_pair != NULL) {
    PROFILE_BLOCK("loop calculating haversines");
    f64 x0 = convert_to_number(get_object_value(current_pair->value, "x0"));
    f64 x1 = convert_to_number(get_object_value(current_pair->value, "x1"));
    f64 y0 = convert_to_number(get_object_value(current_pair->value, "y0"));
    f64 y1 = convert_to_number(get_object_value(current_pair->value, "y1"));

    f64 h = reference_haversine(x0, y0, x1, y1, EARTH_RADIUS);
    f64 actual = 0;
    u64 bytes = outputs_file.read_bytes(&actual, 1, sizeof(actual));
    if (bytes != sizeof(actual) * 1) {
      printf("Error (%lu) reading actual outputs. Num bytes: %lu, should be %lu\n", counter, bytes, sizeof(actual));
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
  printf("Success.\n");
  return 0;
}
