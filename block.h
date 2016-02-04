#ifndef BLOCK_PAGE_SIZE
# define BLOCK_PAGE_SIZE 65536
#endif

#include <stdint.h>

struct block {
    void **pages;
    uint32_t *offsets;
    size_t count;
    size_t size;
};

struct block *block_new(void);

void block_free(struct block*);

void *block_alloc(struct block*, size_t bytes);
