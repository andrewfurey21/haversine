# overview of memory

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
<!-- Give godbolt link -->

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

We could use segmentation. Each segment is a variable sized block
of memory. One segment could reference the stack, another the heap
another the code. Each segment would need it's own base and bounds
pair, and we would have to solve problems like translation, given
that we have now more than one base and bounds pair.

Segmentation leads to fragmentation due to their variable nature,
that can waste a lot of memory. Also, each segment in physical memory
might be sparsely used, and also be wasteful.

### Paging

