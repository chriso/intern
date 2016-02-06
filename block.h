#include <stddef.h>

struct block {
    size_t page_size;
    void **pages;
    size_t *offsets;
    size_t count;
    size_t size;
};

struct block *block_new(size_t page_size);

void block_free(struct block*);

void *block_alloc(struct block*, size_t bytes);
