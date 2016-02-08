cmake_minimum_required (VERSION 2.6)

project (Intern)

set (PAGE_SIZE 4096 CACHE STRING "Page size for allocations")
option (MMAP_PAGES "Allocate pages with mmap(2)" OFF)
option (INLINE_UNSIGNED "Inline unsigned integers into the ID" OFF)

configure_file (config.h.in config.h)

add_library (intern strings.c block.c optimize.c)

install (TARGETS intern DESTINATION lib)
install (FILES strings.h block.h optimize.h DESTINATION include/intern)
