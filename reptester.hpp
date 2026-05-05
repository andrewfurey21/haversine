#include "./src/types.hpp"
#include <string>
#include <vector>
#include "./src/profiler.hpp"
#include <fstream>

const u64 NUM_TESTS = 1;

class RepetitionTester {
public:
  RepetitionTester(const char * name, u64 num_tests) :
    test_name(name),
    begin_clocks(0),
    begin_page_faults(0),
    num_tests(num_tests),
    tests_completed(0),
    max_clocks(0),
    min_clocks(UINT64_MAX),
    total_clocks(0),
    bytes_read(0),
    all_page_faults{},
    all_clocks{},
    state(TESTING)
  {}

  bool testing() { return state == TESTING; }
  void report_error(const std::string& fname, const std::string& msg) {
    std::cerr << "Error at " << fname << "(): " << msg << "\n";
    state = ERROR;
  }

  void pretty_test(const char * name, f64 cpu_freq_hz, f64 clocks) {
    f64 time_ms = clocks / cpu_freq_hz * 1000.f;
    f64 bandwidth = (bytes_read / (time_ms / 1000.f)) / (1024 * 1024 * 1024);
    std::cout << name << ": " << clocks << " (" << time_ms <<  "ms) ";
    if (bytes_read != 0) {
      std::cout << bytes_read / 1000.f << "kB at " << bandwidth << "GB/s\n";
    }
  }

  void report_test() {
    if (state == COMPLETED) {
      f64 cpu_freq = estimate_cpu_freq();
      std::cout << "Estimate cpu freq: " << cpu_freq << "\n";
      pretty_test("min", cpu_freq, min_clocks);
      pretty_test("avg", cpu_freq, (f64)total_clocks / tests_completed);
      pretty_test("max", cpu_freq, max_clocks);
      std::cout << "\n\n";
      // std::ofstream all_clocks_file(test_name + "_clocks");
      // std::ofstream all_page_faults_file(test_name + "_page_faults");
      // for (int i = 0; i < all_clocks.size(); i++) {
      //   all_clocks_file << all_clocks[i] << "\n";
      // }
      // for (int i = 0; i < all_page_faults.size(); i++) {
      //   all_page_faults_file << all_page_faults[i] << "\n";
      // }
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
    begin_page_faults = get_page_faults();
    begin_clocks = get_clocks();
  }
  void end_timer() {
    u64 clocks = get_clocks() - begin_clocks;
    total_clocks += clocks;
    // all_clocks.push_back(clocks);

    // all_page_faults.push_back(get_page_faults()- begin_page_faults);

    if (state != TIMING) { report_error(__func__, "must end time in TIMING state."); }
    tests_completed++;
    max_clocks = std::max(max_clocks, clocks);
    min_clocks = std::min(min_clocks, clocks);
    if (tests_completed >= num_tests) {
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
  u64 begin_page_faults;

  u64 max_clocks;
  u64 min_clocks;
  u64 total_clocks;
  u64 bytes_read;

  std::vector<u64> all_page_faults;
  std::vector<u64> all_clocks;

  enum {
    TESTING,
    TIMING,
    COMPLETED,
    ERROR,
  } state;
};

