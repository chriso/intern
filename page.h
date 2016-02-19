#ifdef MMAP_PAGES
#include <sys/mman.h>

#ifdef __APPLE__
# define INTERN_MAP_ANON MAP_ANON
#else
# define INTERN_MAP_ANON MAP_ANONYMOUS
#endif

static inline void *page_alloc(size_t page_size) {
    return mmap(NULL, page_size, PROT_READ | PROT_WRITE,
                INTERN_MAP_ANON | MAP_PRIVATE, -1, 0);
}

static inline void page_free(void *ptr, size_t page_size) {
    munmap(ptr, page_size);
}

#else
#include <stdlib.h>

static inline void *page_alloc(size_t page_size) {
    return malloc(page_size);
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
static inline void page_free(void *ptr, size_t _) {
    free(ptr);
}

#endif
