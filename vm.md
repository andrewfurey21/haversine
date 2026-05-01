# what is malloc actually doing?

memory is a big part of programming and computers
This post will cover how virtual
memory works, how it works specifically on linux (not a linux kernel
expert), and a few memory allocation strategies and going over their
implementations and analyzing their performance with `perf`.

## virtual memory

The point of virtual memory is to efficiently manage mulitiple
processes running concurrently, while protecting their memory
from other processes. This allows a programmer to write an
application without needing to know how it's memory is managed.
That said, when it comes to performance, this tends to be
a leaky abstraction and knowing how virtual memory works can
be really helpful in diagnosing speed issues.

If you want to get an idea of what memory looks like from your
programs perspective (i.e. the address space), you can use pmap
on Linux.

<!-- remove base and bounds section. just pages. -->

### Base and Bounds

Some constraints we'll start off with is that 1. each process
has a fixed size address space, 2. each process has the same
amount of address space and 3. the sum of all these address
spaces is never greater than physical memory.

A simple implementation of virtual memory might go like this:
the CPU has a base register and a bounds register. The OS
maintains a free list of each block (fixed size remember) of physical memory
that is not being used. When a process starts up, the OS gives
it a base and bounds register from the free list and removes
that pair from the free list. Any time the process reads/writes
from memory, a translation and check occurs (done by the hardware,
specifically, the MMU).i

```cpp
int main() {
  volatile int x = 10;
  x += 10;
  return 0;
}
```
[Check out the compiled code here.](https://godbo.lt/z/KvfzWo6oP)

The steps in loading memory under this virtual memory system would be

1. calculate address of x, which might be rsp - 4
2. add the base offset to that address
3. check the address is less than bounds
4. load/store the memory

Seems like there is some overhead here. Maybe we should cache some
translations...

There are some more details you need for this approach: the hardware
needs some sort of priveleged instructions that can access the base
and bounds registers. Only the OS should be able to execute these,
and not the process. The OS also needs to be able to register some
sort of exception handler function that executes when say the
translation occurs and an out-of-bounds happens.

Problem: what if we don't need that much memory? there's going to be
a bunch of unused address space. This system isn't that efficient.

We could use "segments". Each segment is a variable sized block
of memory. One segment could reference the stack, another the heap
another the code. Each segment would need it's own base and bounds
pair, and we would have to solve problems like translation, given
that we have now more than one base and bounds pair.

Segmentation leads to fragmentation due to their variable nature,
that can waste a lot of memory. Also, each segment in physical memory
might be sparsely used, and also be wasteful.

### Paging

We can avoid fragmentation of memory by simply using fixed length blocks
of memory instead of variable length ones. These blocks are called pages.
The OS needs to keep track of a per-process structure called a page
table which includes translations as well as other meta-info about the page
like whether it's in memory or swapped on disk.

You get two problems with paging.

The first problem is every memory access now requires two memory accesses,
one to get the physical address, the other to actually access memory. You
just doubled how long every memory access takes more or less, and memory
accesses are already really slow.

The second problem is page tables can be huge unless we implement them wisely.
I'm on a system with 48-bit virtual addresses and a page size of 4096. That
means 12 bits for the offset into the page and 36 bits to identify the page.
If in memory we stored where each page goes in physical memory it would take
2^36 bytes, which is a 64GiB.

#### Solution to paging slowness

Translation lookaside buffers. cache translations. typically fully associative.
fully associative has a search problem but tlbs (especially l1) have few entries
maybe 64-128. set associative would introduce more tlb misses.

not taking care with tlb can hurt perf. basically don't constantly access random
pages all the time!

need to take into account size but also sharing processes
and shootdowns when context switching.

os has priveleged instructions to manage the tlb. what do those look like on linux?
(the trap handlers)

could maybe read the tlb doc to understand it's typical structure a bit more.

#### Solution to paging memory use

bigger pages would lead to more "internal fragmentation".

use a multi level page table. basically more levels of indirection. more memory accesses
and more complex, but much less memory on unused pages.

## questions

- on my system, it says 48 bits virtual, 46 bits physical?
- when talking about perf, come up with (maybe pathological) examples.
- are page tables in virtual memory? how does that work?
- you can loop forever allocating memory with malloc/mmap. when you try access you get problems. more tests needed. what does mmap do precisely on linux?
- write out exactly what happens (more or less) when you do a memory access on linux.
- why does malloc 1mib a lot crash but not 1gb (just returns null)?
- i'm confused about what happens from malloc -> mmap -> os? when is a physical page assigned to a virtual address
- mmap vs shmget
- anonymous memory, memory-mapped files
- linux page cache
- implement fopen/fread/fclose, with mmap
