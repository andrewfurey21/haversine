#ifndef PROFILER_HPP
#define PROFILER_HPP

#include "types.hpp"
#include <iostream>
#include <cassert>
#include <immintrin.h>
#include <chrono>

using namespace std::chrono;

const u64 MAX_ANCHORS = 512;

#ifdef NDEBUG
#define PANIC_IF(expr)
#else
#define PANIC_IF(expr) panic_if(expr, __func__, #expr)
#endif
inline void panic_if(bool expr, const char * func_name, const char * msg) {
  if (expr) {
    std::cerr << "Error at " << func_name << "(): " <<  msg << "\n";
    exit(1);
  }
}

inline u64 get_clocks() { return __rdtsc(); }

struct Anchor {
  u64 hit_count;
  u64 clocks;
  u64 child_clocks;
  u64 bytes_processed;
  const char * label;
};

struct Block;

class Profiler {
  friend class Block;

  static u64 num_anchors() { return GetInstance()._num_anchors; }
  static Anchor * get_anchor(u64 anchor_index) {
    return Profiler::_anchors + anchor_index;
  }
  static void new_anchor(const char * label, u64 index) {
    Profiler& profiler = GetInstance();
    PANIC_IF(profiler._num_anchors >= MAX_ANCHORS);
    PANIC_IF(index > MAX_ANCHORS);

    Profiler::_anchors[index].label = label;
    profiler._num_anchors++;
  }
  static u64 update_recent_anchor_index(u64 anchor_index) {
    u64 saved = GetInstance()._recent_anchor_index;
    GetInstance()._recent_anchor_index = anchor_index;
    return saved;
  }
  static Profiler& GetInstance() {
    static Profiler profiler;
    return profiler;
  }

  // 0 index is garbage.
  Profiler() :
  _num_anchors{1},
  _recent_anchor_index{0},
  _start_clock(get_clocks()),
  _start_time(std::chrono::steady_clock::now()) {}

  Profiler(const Profiler& other) = delete;
  Profiler(Profiler&& other) = delete;
  Profiler& operator=(const Profiler& other) = delete;
  Profiler& operator=(Profiler&& other) = delete;
  ~Profiler() {
    // TODO: don't like this. should end timer when last block destructs.
    // TODO: add with children
    u64 total_clocks = get_clocks() - _start_clock;
    milliseconds total_milli =
      duration_cast<milliseconds>(steady_clock::now() - _start_time);
    f64 clock_freq_ghz = (f64)total_clocks / (total_milli.count() / 1000.0) / 1e9;

    std::cout << "Totals : " << total_milli.count() << "ms,"
              << " (" << clock_freq_ghz << "GHz), " << total_clocks <<  " cycles\n";

    for (u64 i = 1; i < MAX_ANCHORS; i++) {
      Anchor& anchor = _anchors[i];
      if (anchor.label != 0) {
        u64 clocks = anchor.clocks - anchor.child_clocks;
        f64 perc = (f64)clocks / total_clocks * 100.0;
        std::cout << anchor.label << " (" << anchor.hit_count <<  "): " << perc << "%";
        if (anchor.bytes_processed != 0) {
          f64 megabytes = anchor.bytes_processed / (1024.0 * 1024.0);
          f64 seconds = ((f64)clocks / 1e9) / clock_freq_ghz;
          f64 gbs = (megabytes / (1024.0)) / seconds;
          std::cout << ", " << megabytes << "MiB at " << gbs << "GiB/s";
        }
        std::cout << "\n";
      }
    }
  }

  u64 _recent_anchor_index;
  u64 _num_anchors;
  u64 _start_clock;
  time_point<steady_clock> _start_time;
  // needs to be static. else, explicitly set all to zero.
  static inline Anchor _anchors[MAX_ANCHORS];
  // inline static Anchor _anchors[MAX_ANCHORS]; ?? what is inline static
};

class Block {
public:
  Block(const char * label, u64 anchor_index, u64 bytes = 0) {
    PANIC_IF(label == 0);
    this->anchor_index = anchor_index;
    this->bytes = bytes;

    Anchor * anchor = Profiler::get_anchor(anchor_index);
    if (anchor->label == 0) {
      Profiler::new_anchor(label, anchor_index);
    }

    anchor->hit_count++;
    this->parent_anchor_index = Profiler::update_recent_anchor_index(anchor_index);
    start_clock = get_clocks();
  }

  ~Block() {
    u64 end_clock = get_clocks();
    Anchor * anchor = Profiler::get_anchor(anchor_index);
    Anchor * parent = Profiler::get_anchor(parent_anchor_index);

    u64 total_clocks = (end_clock - start_clock);

    parent->child_clocks += total_clocks;
    anchor->clocks += total_clocks;
    anchor->bytes_processed += bytes;

    Profiler::update_recent_anchor_index(parent_anchor_index);
  }
private:
  u64 start_clock;
  u64 anchor_index;
  u64 parent_anchor_index;
  u64 bytes;
};

#ifdef NDEBUG
#define PROFILE_BANDWIDTH
#define PROFILE_BLOCK
#define PROFILE_FUNCTION
#else
#define _CONCAT(x, y) x##y
#define CONCAT(x, y) _CONCAT(x, y)
#define PROFILE_BANDWIDTH(name, bytes) \
  Block CONCAT(block, __LINE__) = Block(name, __COUNTER__ + 1, bytes)
#define PROFILE_BLOCK(name) \
  Block CONCAT(block, __LINE__) = Block(name, __COUNTER__ + 1)
#define PROFILE_FUNCTION PROFILE_BLOCK(__func__)
#endif

#endif
