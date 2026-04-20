#ifndef PROFILER_HPP
#define PROFILER_HPP

#include "types.hpp"
#include <iostream>
#include <cassert>
#include <immintrin.h>

// TODO: test each panic_if, double check bounds checking.

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
  const char * label;
};

class Profiler {
public:
  static u64 num_anchors() { return GetInstance()._num_anchors; }
  static Anchor * get_anchor(u64 anchor_index) {
    return Profiler::_anchors + anchor_index;
  }
  static void new_anchor(const char * label) {
    Profiler& profiler = GetInstance();
    PANIC_IF(profiler._num_anchors >= MAX_ANCHORS);

    Profiler::_anchors[profiler._num_anchors].label = label;
    profiler._num_anchors++;
  }
  static u64 child_clocks(u64 anchor_index) {
    const Profiler& profiler = GetInstance();
    PANIC_IF(anchor_index == 0);
    PANIC_IF(anchor_index > MAX_ANCHORS);
    return Profiler::_anchors[anchor_index].child_clocks;
  }
  static u64 update_recent_anchor_index(u64 anchor_index) {
    u64 saved = GetInstance()._recent_anchor_index;
    GetInstance()._recent_anchor_index = anchor_index;
    return saved;
  }
private:
  static Profiler& GetInstance() {
    static Profiler profiler;
    return profiler;
  }

  // 0 index is garbage.
  Profiler() : _num_anchors{1}, _recent_anchor_index{0} {}

  Profiler(const Profiler& other) = delete;
  Profiler(Profiler&& other) = delete;
  Profiler& operator=(const Profiler& other) = delete;
  Profiler& operator=(Profiler&& other) = delete;
  ~Profiler() {
    u64 total_clocks = 0;
    for (u64 i = 1; i < _num_anchors; i++) {
      Anchor& anchor = Profiler::_anchors[i];
      total_clocks += anchor.clocks - anchor.child_clocks;
    }

    std::cout << "Total clocks: " << total_clocks << "\n";
  }

  u64 _recent_anchor_index;
  u64 _num_anchors;
  // needs to be static. else, explicitly set all to zero.
  static inline Anchor _anchors[MAX_ANCHORS];
};

class Block {
public:
  Block(const char * label, u64 anchor_index) {
    PANIC_IF(label == 0);
    this->anchor_index = anchor_index;

    Anchor * anchor = Profiler::get_anchor(anchor_index);
    if (anchor->label == 0) {
      Profiler::new_anchor(label);
    }

    anchor->hit_count++;
    this->parent_anchor_index = Profiler::update_recent_anchor_index(anchor_index);
    start_clock = get_clocks();
  }

  ~Block() {
    u64 end_clock = get_clocks();
    u64 total_clocks =
      (end_clock - start_clock) - Profiler::child_clocks(anchor_index);
    Anchor * anchor = Profiler::get_anchor(anchor_index);
    Anchor * parent = Profiler::get_anchor(parent_anchor_index);

    parent->child_clocks += total_clocks;
    anchor->clocks += total_clocks;

    Profiler::update_recent_anchor_index(parent_anchor_index);
  }
private:
  u64 start_clock;
  u64 anchor_index;
  u64 parent_anchor_index;
};

#ifdef NDEBUG
#define PROFILE_BLOCK
#define PROFILE_FUNCTION
#else
#define CONCAT(x, y) x##y
#define PROFILE_BLOCK(name) \
  Block CONCAT(block, __LINE__) = Block(name, __COUNTER__ + 1)
#define PROFILE_FUNCTION PROFILE_BLOCK(__func__)
#endif

#endif
