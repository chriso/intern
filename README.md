### What is this?

- A fast, immutable string interning scheme (aka. a way of assigning a unique integer id to each unique string, without collisions)
- Two-way lookup: id => string, string => id
- Each string is stored only once in memory
- Optional inlining of unsigned int strings, into the id, via tagging
- Very low fragmentation via a custom block allocator
- Minimal total overhead per string (currently 48 bytes. Could be lower at the cost of additional fragmentation)
- Fast: intern many millions of strings per second
