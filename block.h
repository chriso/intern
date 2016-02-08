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

struct block_snapshot {
    size_t count;
    size_t offset;
};

// Create a snapshot. The block can later be restored to this position
void block_snapshot(const struct block*, struct block_snapshot*);

// Restore the allocator to a previous position. Any allocations made after
// the snapshot are removed. This function returns true if the restore was
// successful, and false if an error occurred
bool block_restore(struct block*, const struct block_snapshot*);
