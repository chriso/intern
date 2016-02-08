cmake_minimum_required (VERSION 2.6)

project (Intern)

set (PAGE_SIZE 4096 CACHE STRING "Page size for allocations")
option (MMAP_PAGES "Allocate pages with mmap(2)" OFF)
option (INLINE_UNSIGNED "Inline unsigned integers into the ID" OFF)
option (BUILD_STATIC "Build a static library" OFF)

configure_file (config.h.in config.h)

if (BUILD_STATIC)
  set (INTERN_LIB_TYPE STATIC)
else (BUILD_STATIC)
  set (INTERN_LIB_TYPE SHARED)
endif (BUILD_STATIC)

add_library (intern ${INTERN_LIB_TYPE} strings.c block.c optimize.c)

install (TARGETS intern DESTINATION lib)
install (FILES strings.h block.h optimize.h DESTINATION include/intern)

enable_testing()
add_executable (tests tests.c)
target_link_libraries (tests intern)
add_test (tests tests)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS tests)

add_executable (benchmark benchmark.c)
target_link_libraries (benchmark intern)

set (CMAKE_C_FLAGS "-std=c99 -Wall -pedantic -g -O3")
