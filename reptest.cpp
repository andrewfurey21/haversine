#include "./src/parser.hpp"

#include <iostream>
#include <cstdio>
#include <string>

#include <filesystem>

#include <sys/resource.h>
#include <sys/mman.h>

#include "reptester.hpp"

void repetition_test_fread(std::string filename) {
  std::cout << "-------------- testing fread --------------\n";
  RepetitionTester rep_test = RepetitionTester("fread", NUM_TESTS);
  u64 buffer_size = std::filesystem::file_size(filename);
  rep_test.register_bytes_read(buffer_size);

  while (rep_test.testing()) {
    FILE *f = fopen(filename.data(), "rb");
    if (f) {
      rep_test.begin_timer();

      Buffer buffer = Buffer(buffer_size); // definitly doing something wrong. os doing something fishy
      u64 num_bytes = fread(buffer.data, 1, buffer_size, f);
      u64 total = 0;
      for (int i = 0; i < buffer_size; i++) {
        total += buffer.data[i];
      }
      rep_test.end_timer();
    } else {
      rep_test.report_error(__func__, "could not read " + filename);
    }
    fclose(f);
  }

  rep_test.report_test();
}

// the graph looks weird i don't know what linux is doing, maybe getrusage isn't as precise as i think it is?
void repetition_test_mmap(std::string filename) {
  std::cout << "-------------- testing mmap --------------\n";
  RepetitionTester rep_test = RepetitionTester("mmap", NUM_TESTS);
  const u64 num_bytes = 4096 * 128;
  rep_test.register_bytes_read(num_bytes);

  u8 *mem = (u8 *)mmap(NULL, num_bytes, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
  // u8 *mem = (u8 *)mmap(NULL, num_bytes, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE | MAP_POPULATE, -1, 0);
  // madvise(mem, num_bytes, MADV_WILLNEED);

  // while (rep_test.testing()) {
    for (u64 i = 0; i < num_bytes; i++) {
      rep_test.begin_timer();
      mem[i] = (u8)i;
      rep_test.end_timer();
    }
  // }
  munmap(mem, num_bytes);

  rep_test.report_test();
}

// TODO: compare mmap'd io vs fread vs read vs fstream.read. read paper. prove mmap is bad. which one is best? why?
// TODO: answer the q: why one big fread over small freads. does fread mmap? when do changes get written to disk?
// TODO: what is populate actually doing? madvise doesn't seem to help.
// TODO: enable huge pages. does linux lock to physical like windows? mmap'd files using hugepages? transparent vs explicit.
// TODO: automatic circular buffer, change detection, sparse memory

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: ./reptest <file>\n";
    return 1;
  }
  // repetition_test_fread(argv[1]);
  repetition_test_mmap("mmap");
  return 0;
}
