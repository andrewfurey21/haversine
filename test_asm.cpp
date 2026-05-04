
#include "src/types.hpp"
#include "reptester.hpp"
#include "./src/parser.hpp"

extern "C" void test_loop_mov(u8 *data, u64 count);
extern "C" void test_loop_nop(u8 *data, u64 count);
extern "C" void test_loop_dec(u8 *data, u64 count);

typedef void (*test_function_t)(u8 *, u64);

void rep_test_fn(test_function_t f) {
  RepetitionTester rep_test("mov", 100);

  const u64 num_bytes = 1024 * 1024 * 1024;
  Buffer buffer = Buffer(num_bytes);
  rep_test.register_bytes_read(num_bytes);

  u64 counter = 0;
  while (rep_test.testing()) {
    rep_test.begin_timer();
    f(buffer.data, buffer.capacity);
    rep_test.end_timer();
    // std::cout << counter++ << "\n";
  }

  rep_test.report_test();
}

int main() {
  rep_test_fn(test_loop_mov);
  rep_test_fn(test_loop_nop);
  rep_test_fn(test_loop_dec);
  return 0;
}
