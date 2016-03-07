# Intern [![Build status][travis-badge]][travis-url]

Fast, immutable [string interning][string-interning] for C.

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
$ cmake -G 'Unix Makefiles' -Wno-dev [OPTIONS]
$ make install
```

Options are:

- `-DBUILD_STATIC=1`: Build a static library (`libintern.a`) rather than a shared
  library (`libintern.so` or `libintern.dylib`)
- `-DMMAP_PAGES=1`: Allocate pages with `mmap(2)` rather than `malloc(3)`
- `-DPAGE_SIZE=4096`: Set the page size
- `-DINLINE_UNSIGNED=1`: Inline unsigned integers between 0 and `INT_MAX`
- `-DDEBUG=1`: Create a debug build

## Usage

Build your project with `-lintern` and include `<intern/strings.h>`.

See [strings.h][strings.h] and [optimize.h][optimize.h] for more details.

## Extra

[Bindings for Go][go-intern]

## License

MIT


[travis-badge]: http://img.shields.io/travis/chriso/intern.svg
[travis-url]: https://travis-ci.org/chriso/intern

[string-interning]: https://en.wikipedia.org/wiki/String_interning

[strings.h]: https://github.com/chriso/intern/blob/master/strings.h
[optimize.h]: https://github.com/chriso/intern/blob/master/optimize.h

[go-intern]: https://github.com/chriso/go-intern
