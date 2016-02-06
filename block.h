#include <stddef.h>
#include <stdbool.h>

struct block {
    size_t page_size;
    void **pages;
    size_t *offsets;
    size_t count;
    size_t size;
};

// Create a new block allocator
struct block *block_new(size_t page_size);

// Free a block allocator
void block_free(struct block*);

// Allocate a chunk of bytes
void *block_alloc(struct block*, size_t bytes);
