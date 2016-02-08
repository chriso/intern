### What is this?

- A fast, immutable string interning scheme (aka. a way of assigning a unique integer id to each unique string, without collisions)
- Two-way lookup: id => string, string => id
- Each string is stored only once in memory
- Optional inlining of unsigned integer strings
- Very low fragmentation via a custom block allocator
- Minimal overhead per string: currently ~40 bytes, which could be lower at the cost of additional fragmentation
- Fast: intern many millions of strings per second
- String repository optimization based on frequency analysis (improve locality)
- Snapshots + restore to a previous state

See [strings.h][strings.h] and [optimize.h][optimize.h] for usage.


[strings.h]: https://github.com/chriso/intern.c/blob/master/strings.h
[optimize.h]: https://github.com/chriso/intern.c/blob/master/optimize.h
