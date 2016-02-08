# Intern

Fast, immutable string interning for C.

## What is this?

- A way of assigning a unique integer ID to each unique string, without collisions
- Two-way lookup: ID => string, string => ID
- Each string is stored only once in memory
- Optional inlining of unsigned integer strings
- Very low fragmentation via a custom block allocator
- Minimal overhead per string: currently ~40 bytes, which could be lower at the cost of additional fragmentation
- Fast: intern many millions of strings per second
- String repository optimization based on frequency analysis (improve locality)
- Support for snapshots (restore to a previous state)

## Installation

```sh
$ cmake -Wno-dev [-DBUILD_STATIC=1] [-DPAGE_SIZE=4096] [-DINLINE_UNSIGNED=1] [-DMMAP_PAGES=1] .
$ make install
```

## Usage

Build your project with `-lintern` and include `<intern/strings.h>`.

See [strings.h][strings.h] and [optimize.h][optimize.h] for more details.

[strings.h]: https://github.com/chriso/intern/blob/master/strings.h
[optimize.h]: https://github.com/chriso/intern/blob/master/optimize.h

## License

MIT
