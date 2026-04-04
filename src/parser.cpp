#include <iostream>

#include <cstdio>
#include <cstdlib>

#include "parser.hpp"

int main(int argc, char ** argv) {
  if (argc != 2) {
    std::cout << "Usage: ./parser <json file>\n";
    return 1;
  }

  FILE * f;
  f = fopen(argv[1], "r");
  if (!f) {
    printf("Error: could not read file: %s\n", argv[1]);
    return 1;
  }
  JSONElement * json = parse_json(f);

  JSONElement * pairs_list = get_object_value(json, "pairs");

  assert(json->value == pairs_list);
  assert(json->next == NULL);

  JSONElement * current = pairs_list;

  while (current != NULL) {
    f64 x0 = convert_to_number(get_object_value(current->value, "x0"));
    f64 x1 = convert_to_number(get_object_value(current->value, "x1"));
    f64 y0 = convert_to_number(get_object_value(current->value, "y0"));
    f64 y1 = convert_to_number(get_object_value(current->value, "y1"));

    printf("x0: %f, y0: %f, x1: %f, y1: %f\n", x0, y0, x1, y1);
    fflush(stdout);
    current = current->next;
  }
  destroy_json(json);
  fclose(f);
  return 0;
}

