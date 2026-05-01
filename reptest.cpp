#include "./src/parser.hpp"
#include "./src/profiler.hpp"

#include <fstream>
#include <vector>
#include <iostream>
#include <cstdio>
#include <string>

#include <cstdint>
#include <filesystem>

using namespace std::chrono;

const u64 NUM_TESTS = 1'000;

class RepititionTester {
public:
  RepititionTester(const char * name, u64 num_tests) :
    test_name(name),
    begin_clocks(0),
    num_tests(num_tests),
    tests_completed(0),
    max_clocks(0),
    min_clocks(UINT64_MAX),
    total_clocks(0),
    bytes_read(0),
    state(TESTING)
  {}

  bool testing() { return state == TESTING; }
  void report_error(const std::string& fname, const std::string& msg) {
    std::cerr << "Error at " << fname << "(): " << msg << "\n";
    state = ERROR;
  }

  void pretty_test(const char * name, f64 cpu_freq_hz, f64 clocks) {
    f64 time_ms = clocks / cpu_freq_hz * 1000.f;
    f64 bandwidth = (bytes_read / (time_ms / 1000.f)) / 1'000'000'000;
    std::cout << name << ": " << clocks << " (" << time_ms <<  "ms) ";
    if (bytes_read != 0) {
      std::cout << bytes_read / 1000.f << "kB at " << bandwidth << "GB/s\n";
    }
  }

  void report_test() {
    if (state == COMPLETED) {
      f64 cpu_freq = estimate_cpu_freq();
      pretty_test("min", cpu_freq, min_clocks);
      pretty_test("avg", cpu_freq, (f64)total_clocks / tests_completed);
      pretty_test("max", cpu_freq, max_clocks);
      std::cout << "-------------------------------------------\n\n";
      std::ofstream all_clocks_file(test_name);
      for (int i = 0; i < all_clocks.size(); i++) {
        all_clocks_file << all_clocks[i] << "\n";
      }
      return;
    }

    if (state != ERROR) {
      std::cerr << "Warning: ended repetition testing during a test (" << state << ").\n";
      return;
    }

    std::cerr << "Exiting because of error.\n";
  }
  void register_bytes_read(u64 num_bytes) {
    bytes_read = num_bytes;
  }
  void begin_timer() {
    state = TIMING;
    begin_clocks = get_clocks();
  }
  void end_timer() {
    u64 clocks = get_clocks() - begin_clocks;
    total_clocks += clocks;
    all_clocks.push_back(clocks);

    if (state != TIMING) { report_error(__func__, "must end time in TIMING state."); }
    tests_completed++;
    max_clocks = std::max(max_clocks, clocks);
    min_clocks = std::min(min_clocks, clocks);
    if (tests_completed == num_tests) {
      state = COMPLETED;
    } else {
      state = TESTING;
    }
  }
  u64 completed_test() { return tests_completed; }
private:

  std::string test_name;
  u64 num_tests;
  u64 tests_completed;

  u64 begin_clocks;

  u64 max_clocks;
  u64 min_clocks;
  u64 total_clocks;
  u64 bytes_read;

  std::vector<u64> all_clocks;

  enum {
    TESTING,
    TIMING,
    COMPLETED,
    ERROR,
  } state;
};

void repetition_test_fread(std::string filename) {
  std::cout << "-------------- testing fread --------------\n";
  RepititionTester rep_test = RepititionTester("fread", NUM_TESTS);
  u64 buffer_size = std::filesystem::file_size(filename);
  rep_test.register_bytes_read(buffer_size);

  while (rep_test.testing()) {
    FILE *f = fopen(filename.data(), "rb");
    if (f) {
      rep_test.begin_timer();

      Buffer buffer = Buffer(buffer_size);
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

void repetition_test_fstream_read(std::string filename) {
  std::cout << "------------- testing fstream -------------\n";
  RepititionTester rep_test = RepititionTester("fstream.read", NUM_TESTS);
  u64 buffer_size = std::filesystem::file_size(filename);
  rep_test.register_bytes_read(buffer_size);

  Buffer buffer = Buffer(buffer_size);

  while (rep_test.testing()) {
    std::fstream f = std::fstream();
    f.open(filename, std::ios_base::binary | std::ios_base::in);

    if (f.is_open()) {
      rep_test.begin_timer();
      Buffer buffer = Buffer(buffer_size);
      f.read((char *)buffer.data, buffer.capacity);
      rep_test.end_timer();
    } else {
      rep_test.report_error(__func__, "could not read " + filename);
    }

    f.close();
  }

  rep_test.report_test();
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: ./reptest <file>\n";
    return 1;
  }
  repetition_test_fread(argv[1]);
  repetition_test_fstream_read(argv[1]);
  return 0;
}
