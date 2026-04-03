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

  printf("Key: %.*s, Value: %.*s\n",
         (u32)json->key.buffer.capacity,
         json->key.buffer.data,
         (u32)json->value->key.buffer.capacity,
         json->value->key.buffer.data);

  printf("Key: %.*s, Value: %.*s\n",
         (u32)json->next->key.buffer.capacity,
         json->next->key.buffer.data,
         (u32)json->next->value->key.buffer.capacity,
         json->next->value->key.buffer.data);

  destroy_json(&json);

  fclose(f);
  return 0;
}

