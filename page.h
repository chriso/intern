#ifdef PAGE_MMAP
#include <sys/mman.h>

static inline void *page_alloc(size_t page_size) {
    return mmap(NULL, page_size, PROT_READ | PROT_WRITE,
                MAP_ANON | MAP_PRIVATE, -1, 0);
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
