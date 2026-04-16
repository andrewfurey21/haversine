#ifndef PROFILER_HPP
#define PROFILER_HPP

#include "types.hpp"

#include <ctime>
#include <cassert>
#include <immintrin.h>

const u64 MAX_BLOCK_STACK = 65536;
const u64 MAX_ANCHORS = 512;

// total_clocks = (tsc() - start) - child_clocks;
// parent->child_clocks += total_clocks;

// anchor list
// block stack (surjective onto anchor list)

inline u64 get_clocks() { return __rdtsc(); }

struct Anchor {
  u64 hit_count;
  u64 clocks;
  u64 child_clocks;
};

struct Block {
  Block(const char * label, Anchor * anchor, Anchor * parent_anchor) :
    label(label),
    start_clocks(get_clocks()),
    anchor(anchor),
    parent_anchor(parent_anchor) {}

  ~Block() {
    u64 end = get_clocks();
  }

  const char * label;
  u64 start_clocks;
  Anchor * anchor;
  Anchor * parent_anchor;
};

class Profiler {
public:
  Block start_timer(const char * label, u64 anchor_index) {
    Anchor * parent_anchor = block_stack[block_stack_pointer].anchor;
    Anchor * anchor = anchors + anchor_index;
    // block_stack[++block_stack_pointer] = Block(label, anchors + anc)

    // return Block(label, anchors + anchor_index, parent_anchor);
  }

private:
  static Profiler& GetInstance() {
    static Profiler profiler;
    return profiler;
  }

  Profiler() : block_stack_pointer{0} {
    // struct timespec x;
    // clock_gettime(CLOCK_MONOTONIC, &x);
    // better c++ version?
    // std::clock + std::high_resolution_clock
    // high_resolution_clock can an alias to steady_clock vs system_clock
  }

  Profiler(const Profiler& other) = delete;
  Profiler(Profiler&& other) = delete;
  Profiler& operator=(const Profiler& other) = delete;
  Profiler& operator=(Profiler&& other) = delete;
  ~Profiler() { /* print stats */ }

  static Anchor anchors[MAX_ANCHORS];

  u64 block_stack_pointer;
  static Block block_stack[MAX_BLOCK_STACK];
};

// TODO: Macros for ease of use

// PROFILE_BLOCK
// PROFILE_FUNCT

#endif
