cmake_minimum_required (VERSION 2.6)
cmake_policy (VERSION 3.0.0)

project (Intern)

set (INTERN_LIB_NAME intern)

set (PAGE_SIZE 4096 CACHE STRING "Page size for allocations")
option (MMAP_PAGES "Allocate pages with mmap(2)" OFF)
option (INLINE_UNSIGNED "Inline unsigned integers into the ID" OFF)
option (BUILD_STATIC "Build a static library" OFF)

configure_file (config.h.in config.h)

if (BUILD_STATIC)
  set (INTERN_LIB_TYPE STATIC)
else ()
  set (INTERN_LIB_TYPE SHARED)
endif ()

set (INTERN_SRC strings.c block.c optimize.c)
set (INTERN_HEADERS strings.h block.h optimize.h)

add_library (${INTERN_LIB_NAME} ${INTERN_LIB_TYPE} ${INTERN_SRC})

install (TARGETS ${INTERN_LIB_NAME} DESTINATION lib)
install (FILES ${INTERN_HEADERS} DESTINATION include/${INTERN_LIB_NAME})

enable_testing()
add_executable (tests tests.c)
target_link_libraries (tests ${INTERN_LIB_NAME})
add_test (tests tests)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose DEPENDS tests)

add_executable (benchmark benchmark.c)
target_link_libraries (benchmark ${INTERN_LIB_NAME})

set (CMAKE_C_FLAGS "-std=c99 -Wall -pedantic -g -D_GNU_SOURCE")
set (CMAKE_C_FLAGS_RELEASE "-O3 -march=native")
