
#include "src/types.hpp"
#include "reptester.hpp"
#include "./src/parser.hpp"

extern "C" void test_loop_mov(u8 *data, u64 count);
extern "C" void test_loop_nop(u8 *data, u64 count);
extern "C" void test_loop_dec(u8 *data, u64 count);

typedef void (*test_function_t)(u8 *, u64);

#define RUN_TEST(f) { std::cout << "running: " << #f << "\n"; rep_test_fn(f); }

void rep_test_fn(test_function_t f) {
  RepetitionTester rep_test("mov", 10000);

  const u64 num_bytes = 1024 * 1024 * 4;
  Buffer buffer = Buffer(num_bytes);
  rep_test.register_bytes_read(num_bytes);

  while (rep_test.testing()) {
    rep_test.begin_timer();
    f(buffer.data, buffer.capacity);
    rep_test.end_timer();
  }

  rep_test.report_test();
}

int main() {
  RUN_TEST(test_loop_mov);
  RUN_TEST(test_loop_nop);
  RUN_TEST(test_loop_dec);
  return 0;
}
