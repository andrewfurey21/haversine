#include <cstdint>
#include <cstdio>

#include <x86intrin.h>
#include <sys/time.h>

using u64 = uint64_t;
using f64 = double;

#include <string>
#include <iostream>
inline void _panic_if(bool expr, const std::string& func_name, const std::string& msg) {
  if (expr) {
    std::cerr << "Error at " << func_name << ": " <<  msg << "\n";
    exit(1);
  }
}

#define PANIC_IF(expr) _panic_if(expr, __func__, #expr)

static u64 get_os_timer_freq() {
  return 1'000'000; // os timer is in microseconds.
}

static u64 read_os_timer() {
  struct timeval value;
  gettimeofday(&value, 0);

  return get_os_timer_freq() * value.tv_sec + value.tv_usec;
}

static inline u64 read_cpu_timer() { return __rdtsc(); }

int main(int argc, char ** argv) {
  PANIC_IF(argc != 2);
  // if (argc != 2) {
  //   printf("Usage: ./timer <microseconds>\n");
  //   return 1;
  // }

  u64 wait_time = atol(argv[1]);
  u64 cpu_clocks_start = read_cpu_timer();

  u64 start = read_os_timer();
  u64 elapsed = 0;

  while (elapsed < wait_time) {
    u64 current = read_os_timer();
    elapsed = current - start;
  }

  u64 cpu_clocks_end = read_cpu_timer();

  u64 cpu_clocks = cpu_clocks_end - cpu_clocks_start;
  printf("Finsihed. %lu, %lu\n", elapsed, cpu_clocks);

  // TODO: profile parts of the haversine processor.

  u64 clock_rate = (f64)cpu_clocks * get_os_timer_freq() / elapsed;
  printf("Estimated clock rate: %lu\n", clock_rate);
}

