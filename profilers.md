# how to write a basic instrumented cpu profiler

## getting accurate time readings: invariant tsc, clock_gettime

rdtsc, compare with linux `clock_gettime`. counts at a constant clock rate,
how does clock_gettime work? it uses rdtsc in some way? how?
how does it get nice human-friendly time?

independent of actual clock rate, that count at the nominal frequency (on this laptop, around 2.3GHz).
and *consistent across cores*. not actual cycles though. my cpu has constant_tsc extension.
you can also check with CPUID. `CPUID input=EAX=80000007H, output=EDX bit 8`

want to avoid instruction reordering
rdtscp includes the processor?
rdtsc can be reordered? rdtscp can't? should mfence be used after?

cpuid is serializing, but it clobbers registers :( lfence better :)

rdpru, rdpmc

use rdtsc over something like clock_gettime because there is much lower overhead.
in fact, from stepping through clock_gettime, it seems to use rdtsc.

`__rdtsc()` intrinsic will shift and or rdx and rax.

### time

if you want to get time, use clock_gettime over something like gettimeofday.
gettimeofday affected by NTP sync. it's good for getting the time of day,
but not timining!

if it's okay for getting time of day, how can it not be okay for timing.

https://blog.habets.se/2010/09/gettimeofday-should-never-be-used-to-measure-time.html
https://github.com/nmap/nmap/issues/180

## intel how to benchmark code execution times

https://cis.temple.edu/~qzeng/cis3207-spring18/files/ia-32-ia-64-benchmark-code-execution-paper.pdf

All power optimization, Intel Hyper-Threading technology,
frequency scaling and turbo mode functionalities were turned off

prevent cpu scheduler preemption to guarantee exclusive cpu ownership
by writing a kernel module.
need to measure overhead of the calls to RDTSC and serializing instructions.

TSC keeps incrementing, even if the program isn't currently running
due to scheduler preemption. Avoid on linux by writing a kernel module.

For this post, I won't do that, just for simplicity.

https://docs.kernel.org/kbuild/modules.html

TODO: reread, still not sure how improvements work.

Maybe i should graph stuff too. maybe shouldn't because i don't know
what i'm doing.

### memory and execution ordering on x86

Memory ordering means what memory reordering you can expect at runtime, given
the processor you are targeting and toolchain you are using.

x86 has a strongly ordered memory model, meaning each read and write has
implicit acquire and release semantics.

Q: what's the difference between #LoadStore and #StoreLoad?
A: can a load then store be reordered vs a store then load.

Q: SO says lfence is nop, but other SO says use lfence for rdtsc
A: (?) lfence is a nop memory-wise, not instruction-wise.

sfence: Serializes store operations.
all stores finish before sfence before any stores can execute after.

lfence: Serializes load operations.
all prior instructions need to execute before lfence.
not a serializing instruction, it doesn't flush the store buffer.

mfence: all loads+stores prior finish before and loads+stores post.
mfence != sfence+lfence or lfence+sfence.
sfence+lfence does not ensure old stores visible to young loads.

## instrumented profiler

while instrumented profilers have a much larger overhead, they are
much simpler to implement and if we don't overuse them they can produce
pretty accurate results

goals for an instrumented profiler:
1. accurate, fast, simple. try not to affect runtime as much
2. human readable time in seconds. not just clock cycles. also estimate
   clocks frequency. use `clock_gettime`.
3. easy to use
4. easy to turn off, use `NDEBUG`.
5. able to time functions and blocks like while, for, if etc and know how
   many times they were used.
6. correct for nested and recursive functions
7. at the end, nice statistics.

naive solution would be to have a list, each with a label.
count cycles, add up at the end. problem is that a function calling
a function will count the cycles of that function. not super useful.
if you add a timer inside a function, timing another function
or block, you don't want to include those cycles in the outer
functions timer bigger
problem with recursive functions.

### blow's prof

Zone (Anchor):
- total self ticks
- total hier ticks
- t self start
- t hier start
- total entry count: just the number of times this zone was used.

Profiling:
- zone stack (runtime)
- zone pointers by index (compile time)

### my profiler

when a child ends, it updates it's parents no ticks.
Anchor: start, end, empty/no_ticks.
total = (end - start) - emtpy;
parent->empty += child->total;
// parent->empty can never be greater than end - start

use std=c++26 for reflection, this would make the post great.

## stuff to look into later

i learned recently there are non-temporal instructions, that seem to
not mess around with the cache? maybe those instructions will make the
profiler more accurate.

intel benchmark whitepaper uses linux kernel modules to ensure scheduler
preemption doesn't happen to get more accurate timings.
